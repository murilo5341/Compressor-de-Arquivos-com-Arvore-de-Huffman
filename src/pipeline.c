/*
 * pipeline.c - Pipeline concorrente de compressao (Modulo 13).
 *
 * Arquitetura (ver pipeline.h):
 *   leitora -> raw_queue -> N codificadoras -> done_queue -> escritora
 *
 * As filas (Modulo 12) sao LIMITADAS: dao contrapressao (backpressure) para que
 * uma etapa rapida nao consuma memoria sem limite. O fim de cada etapa e
 * sinalizado fechando a fila (queue_close):
 *   - a leitora fecha a raw_queue ao terminar de ler;
 *   - a ULTIMA codificadora a sair fecha a done_queue (contador protegido por
 *     mutex), para a escritora saber que nao chegarao mais blocos.
 *
 * SINCRONIZACAO: o unico estado mutavel COMPARTILHADO entre threads sao o
 * contador de codificadoras ativas e os flags de erro, todos protegidos por
 * ctx->lock. As filas tem sincronizacao propria. O FILE* de saida e tocado por
 * UMA thread de cada vez com ordem garantida por pthread_create/join:
 *   main escreve o cabecalho global -> (cria threads) -> escritora grava os
 *   blocos -> (join) -> main reescreve o block_count. Sem acesso concorrente ao
 *   arquivo, portanto sem mutex global de I/O (RULES REGRA 9). Validar com TSan.
 *
 * REORDENADOR (escritora): os blocos comprimidos chegam fora de ordem. A
 * escritora mantem 'next' (proximo indice a gravar) e um vetor 'pending'
 * indexado pelo indice do bloco; ao chegar o bloco 'next', grava-o e os
 * seguintes ja presentes, liberando a memoria de cada um logo apos a gravacao.
 */
#include "pipeline.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "crc32.h"
#include "format.h"
#include "queue.h"
#include "tree_serialization.h"

/* Capacidade base das filas: da folga para as threads sem segurar tudo em RAM. */
#define QUEUE_MIN_CAPACITY 8

/* Bloco cru lido do arquivo (a leitora e dona de 'data' ate a codificadora). */
typedef struct {
    uint64_t index;
    uint8_t *data;  /* malloc; liberado pela codificadora apos comprimir */
    size_t   size;
} RawBlock;

/*
 * Bloco ja comprimido, pronto para gravar. A codificadora e dona de tree_bytes
 * e payload ate a escritora grava-los e libera-los.
 */
typedef struct {
    uint64_t index;
    uint32_t original_size;
    uint32_t compressed_size;
    uint32_t tree_size;
    uint32_t crc;
    uint8_t *tree_bytes;  /* arvore serializada (Modulo 7) */
    uint8_t *payload;     /* bits comprimidos (Modulo 6) */
} CompressedBlock;

/* Estado compartilhado do pipeline. */
typedef struct {
    FILE          *in;
    FILE          *out;
    uint32_t       block_size;
    BlockingQueue *raw_q;
    BlockingQueue *done_q;

    pthread_mutex_t lock;            /* protege os campos abaixo */
    int             encoders_active; /* codificadoras que ainda nao sairam */
    bool            read_error;
    bool            encode_error;

    /* Escrito apenas pela escritora; lido pela main APOS o join (sem corrida). */
    bool            write_error;
    uint64_t        block_count;     /* quantos blocos a escritora gravou */
} Pipeline;

/* ------------------------------------------------------------------ */
/* Helpers de flag (sob o mutex)                                       */
/* ------------------------------------------------------------------ */

static void set_read_error(Pipeline *p)
{
    pthread_mutex_lock(&p->lock);
    p->read_error = true;
    pthread_mutex_unlock(&p->lock);
}

static void set_encode_error(Pipeline *p)
{
    pthread_mutex_lock(&p->lock);
    p->encode_error = true;
    pthread_mutex_unlock(&p->lock);
}

static void free_compressed(CompressedBlock *cb)
{
    if (cb == NULL)
        return;
    free(cb->tree_bytes);
    free(cb->payload);
    free(cb);
}

/* ------------------------------------------------------------------ */
/* Thread leitora                                                      */
/* ------------------------------------------------------------------ */

static void *reader_thread(void *arg)
{
    Pipeline *p = (Pipeline *)arg;
    uint64_t index = 0;

    for (;;) {
        uint8_t *data = malloc(p->block_size);
        if (data == NULL) {
            set_read_error(p);
            break;
        }
        size_t n = fread(data, 1, p->block_size, p->in);
        if (n == 0) {  /* EOF (ou erro, verificado abaixo) */
            free(data);
            break;
        }

        RawBlock *rb = malloc(sizeof(*rb));
        if (rb == NULL) {
            free(data);
            set_read_error(p);
            break;
        }
        rb->index = index++;
        rb->data  = data;
        rb->size  = n;

        if (!queue_push(p->raw_q, rb)) {  /* fila fechada inesperadamente */
            free(rb->data);
            free(rb);
            break;
        }
    }

    if (ferror(p->in))
        set_read_error(p);

    queue_close(p->raw_q);  /* nao havera mais blocos crus */
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Threads codificadoras                                               */
/* ------------------------------------------------------------------ */

/* Comprime um RawBlock num CompressedBlock. Retorna NULL em erro (ja libera o
 * que alocou internamente; nao libera 'rb'). */
static CompressedBlock *encode_block(const RawBlock *rb)
{
    CompressedBlock *cb = malloc(sizeof(*cb));
    if (cb == NULL)
        return NULL;

    uint32_t crc = crc32_buffer(rb->data, rb->size);

    BlockCompressed bc;
    if (!block_compress(rb->data, rb->size, &bc)) {
        free(cb);
        return NULL;
    }

    uint8_t *tree_bytes = NULL;
    size_t   tree_size  = 0;
    if (!tree_serialize(bc.tree, &tree_bytes, &tree_size)) {
        block_compressed_free(&bc);
        free(cb);
        return NULL;
    }

    cb->index           = rb->index;
    cb->original_size   = (uint32_t)bc.original_size;
    cb->compressed_size = (uint32_t)bc.compressed_size;
    cb->tree_size       = (uint32_t)tree_size;
    cb->crc             = crc;
    cb->tree_bytes      = tree_bytes;
    cb->payload         = bc.data;  /* doacao: evita copiar o payload */
    bc.data             = NULL;     /* para block_compressed_free nao liberar */

    block_compressed_free(&bc);     /* libera so a arvore (data ja foi doado) */
    return cb;
}

static void *encoder_thread(void *arg)
{
    Pipeline *p = (Pipeline *)arg;

    for (;;) {
        RawBlock *rb = NULL;
        if (!queue_pop(p->raw_q, (void **)&rb))
            break;  /* raw_q vazia e fechada: fim */

        CompressedBlock *cb = encode_block(rb);
        free(rb->data);
        free(rb);

        if (cb == NULL) {
            set_encode_error(p);
            continue;  /* segue drenando para nao travar a leitora */
        }

        if (!queue_push(p->done_q, cb)) {  /* fila fechada inesperadamente */
            free_compressed(cb);
            break;
        }
    }

    /* A ULTIMA codificadora a terminar fecha a done_q. */
    pthread_mutex_lock(&p->lock);
    bool last = (--p->encoders_active == 0);
    pthread_mutex_unlock(&p->lock);
    if (last)
        queue_close(p->done_q);

    return NULL;
}

/* ------------------------------------------------------------------ */
/* Thread escritora (reordenador)                                      */
/* ------------------------------------------------------------------ */

/* Grava cabecalho + arvore + payload de um bloco no .cz (layout do Modulo 9). */
static bool write_block(FILE *out, const CompressedBlock *cb)
{
    CzBlockHeader bh;
    bh.block_index     = cb->index;
    bh.original_size   = cb->original_size;
    bh.compressed_size = cb->compressed_size;
    bh.tree_size       = cb->tree_size;
    bh.original_crc32  = cb->crc;

    if (!cz_write_block_header(out, &bh))
        return false;
    if (cb->tree_size > 0 &&
        fwrite(cb->tree_bytes, 1, cb->tree_size, out) != cb->tree_size)
        return false;
    if (cb->compressed_size > 0 &&
        fwrite(cb->payload, 1, cb->compressed_size, out) != cb->compressed_size)
        return false;
    return true;
}

static void *writer_thread(void *arg)
{
    Pipeline *p = (Pipeline *)arg;

    CompressedBlock **pending = NULL;  /* vetor indexado pelo indice do bloco */
    size_t   pending_cap = 0;
    uint64_t next        = 0;          /* proximo indice a gravar (em ordem) */
    uint64_t count       = 0;          /* blocos efetivamente gravados */
    bool     failed      = false;

    for (;;) {
        CompressedBlock *cb = NULL;
        if (!queue_pop(p->done_q, (void **)&cb))
            break;  /* done_q vazia e fechada: fim */

        if (failed) {
            /* Ja falhou: continua drenando e liberando para nao travar. */
            free_compressed(cb);
            continue;
        }

        /* Garante espaco em 'pending' para o indice deste bloco. */
        if (cb->index >= pending_cap) {
            size_t new_cap = (pending_cap == 0) ? QUEUE_MIN_CAPACITY : pending_cap;
            while (cb->index >= new_cap)
                new_cap *= 2;
            CompressedBlock **grown =
                realloc(pending, new_cap * sizeof(*pending));
            if (grown == NULL) {
                failed = true;
                free_compressed(cb);
                continue;
            }
            for (size_t i = pending_cap; i < new_cap; i++)
                grown[i] = NULL;
            pending     = grown;
            pending_cap = new_cap;
        }

        pending[cb->index] = cb;

        /* Grava todos os blocos consecutivos ja disponiveis a partir de 'next'. */
        while (next < pending_cap && pending[next] != NULL) {
            CompressedBlock *w = pending[next];
            if (!write_block(p->out, w))
                failed = true;
            free_compressed(w);
            pending[next] = NULL;
            next++;
            count++;
        }
    }

    /* Caminho de erro: libera blocos adiantados que ficaram sem ser gravados
     * (ex.: um indice no meio nunca chegou porque uma codificadora falhou). */
    for (size_t i = 0; i < pending_cap; i++)
        free_compressed(pending[i]);
    free(pending);

    p->write_error = failed;
    p->block_count = count;
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Orquestracao                                                        */
/* ------------------------------------------------------------------ */

int pipeline_compress_file(const char *input_path, const char *output_path,
                           uint32_t block_size, int num_threads)
{
    if (num_threads < 1)
        num_threads = 1;

    FILE *in = fopen(input_path, "rb");
    if (in == NULL) {
        fprintf(stderr, "czip: nao foi possivel abrir a entrada '%s'.\n",
                input_path);
        return 1;
    }
    FILE *out = fopen(output_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "czip: nao foi possivel criar a saida '%s'.\n",
                output_path);
        fclose(in);
        return 1;
    }

    /* Cabecalho global com block_count provisorio = 0 (reescrito no fim). */
    CzGlobalHeader gh;
    cz_global_header_init(&gh, block_size, 0);
    if (!cz_write_global_header(out, &gh)) {
        fprintf(stderr, "czip: falha ao gravar o cabecalho global.\n");
        fclose(in);
        fclose(out);
        return 1;
    }

    /* Capacidade das filas proporcional ao numero de threads, com um minimo. */
    size_t cap = (size_t)num_threads * 2;
    if (cap < QUEUE_MIN_CAPACITY)
        cap = QUEUE_MIN_CAPACITY;

    Pipeline p;
    p.in              = in;
    p.out             = out;
    p.block_size      = block_size;
    p.raw_q           = queue_create(cap);
    p.done_q          = queue_create(cap);
    p.encoders_active = num_threads;
    p.read_error      = false;
    p.encode_error    = false;
    p.write_error     = false;
    p.block_count     = 0;

    if (p.raw_q == NULL || p.done_q == NULL ||
        pthread_mutex_init(&p.lock, NULL) != 0) {
        fprintf(stderr, "czip: falha ao inicializar o pipeline.\n");
        queue_destroy(p.raw_q);
        queue_destroy(p.done_q);
        fclose(in);
        fclose(out);
        return 1;
    }

    pthread_t  reader;
    pthread_t  writer;
    pthread_t *encoders = malloc((size_t)num_threads * sizeof(*encoders));
    if (encoders == NULL) {
        fprintf(stderr, "czip: sem memoria para %d threads.\n", num_threads);
        pthread_mutex_destroy(&p.lock);
        queue_destroy(p.raw_q);
        queue_destroy(p.done_q);
        fclose(in);
        fclose(out);
        return 1;
    }

    /* Cria as threads. (Se a criacao falhasse, o ambiente Linux ja estaria em
     * estado critico; tratamos o retorno por completude.) */
    int rc = 0;
    if (pthread_create(&reader, NULL, reader_thread, &p) != 0)
        rc = 1;
    int created = 0;
    for (int i = 0; rc == 0 && i < num_threads; i++) {
        if (pthread_create(&encoders[i], NULL, encoder_thread, &p) != 0)
            rc = 1;
        else
            created++;
    }
    if (rc == 0 && pthread_create(&writer, NULL, writer_thread, &p) != 0)
        rc = 1;

    if (rc != 0) {
        /* Falha rara de criacao de thread: fecha filas para destravar quem ja
         * roda, junta o que foi criado e aborta. */
        fprintf(stderr, "czip: falha ao criar threads do pipeline.\n");
        queue_close(p.raw_q);
        queue_close(p.done_q);
        pthread_join(reader, NULL);
        for (int i = 0; i < created; i++)
            pthread_join(encoders[i], NULL);
        free(encoders);
        pthread_mutex_destroy(&p.lock);
        queue_destroy(p.raw_q);
        queue_destroy(p.done_q);
        fclose(in);
        fclose(out);
        return 1;
    }

    /* Espera todo o pipeline terminar (barreira: estabelece happens-before). */
    pthread_join(reader, NULL);
    for (int i = 0; i < num_threads; i++)
        pthread_join(encoders[i], NULL);
    pthread_join(writer, NULL);

    free(encoders);

    bool ok = !(p.read_error || p.encode_error || p.write_error);

    /* Reescreve o cabecalho global com a quantidade real de blocos. */
    if (ok) {
        if (fseek(out, 0L, SEEK_SET) != 0) {
            fprintf(stderr, "czip: falha ao reposicionar para o cabecalho global.\n");
            ok = false;
        } else {
            cz_global_header_init(&gh, block_size, p.block_count);
            if (!cz_write_global_header(out, &gh)) {
                fprintf(stderr, "czip: falha ao reescrever o cabecalho global.\n");
                ok = false;
            }
        }
    } else {
        if (p.read_error)
            fprintf(stderr, "czip: erro de leitura na entrada '%s'.\n", input_path);
        if (p.encode_error)
            fprintf(stderr, "czip: falha ao comprimir um ou mais blocos.\n");
        if (p.write_error)
            fprintf(stderr, "czip: falha ao gravar no arquivo de saida.\n");
    }

    pthread_mutex_destroy(&p.lock);
    queue_destroy(p.raw_q);
    queue_destroy(p.done_q);
    fclose(in);
    if (fclose(out) != 0 && ok) {
        fprintf(stderr, "czip: falha ao finalizar o arquivo de saida.\n");
        ok = false;
    }

    if (ok) {
        printf("czip: '%s' -> '%s' (%llu bloco(s), block-size=%u, threads=%d).\n",
               input_path, output_path,
               (unsigned long long)p.block_count, block_size, num_threads);
    }
    return ok ? 0 : 1;
}
