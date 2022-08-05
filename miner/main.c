#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "transactions.h"
#include "mempool.h"
#include "siggen.h"
#include "nonce.h"
#include "event_q.h"
#include "reader.h"


static void trx_print(char *prefix, trx_t *trx) {
    printf("%s%d %s %s %d %d\n", prefix, trx->id, trx->payer,
           trx->payee, trx->amount, trx->fee);
}

static void trx_age(trx_t *trx, int prio) {
    printf("Aging transaction (%d):", prio);
    trx_print("", trx);
}

int main() {
    trx_list_t *selected = transaction_list();
    unsigned int prev_id = 0;
    unsigned int prev_sig = 0;

    mempool_init();
    reader_start();

    for (;;) {
        event_t *event = event_remove();
        assert(event);

        if (event->type == E_END) {
            reader_join();
            event_delete(event);
            break;
        } else if (event->type == E_EPOCH) {
            mempool_age(trx_age);
        } else if (event->type == E_TRX) {
            trx_t * trx = transaction_new();
            *trx = event->transactions[0];
            trx_print("Adding transaction: ", trx);
            mempool_add(trx);
        } else if (event->type == E_BLK) {
            for (int i = 0; i < event->num_trx; i++) {
                trx_t *t = mempool_remove(event->transactions[i].id);
                if (t != NULL) {
                    trx_print("Removing transaction: ", t);
                    transaction_delete(t);
                }
            }

            prev_id = event->id;
            prev_sig = event->sig;
        } else if (event->type == E_MINE) {
            unsigned int id = prev_id + 1;
            unsigned int num_trx = 0;
            int available = 256 - 24; // 24 bytes in block without transactions

            for (trx_t *t = mempool_select(available); t != NULL; t = mempool_select(available)) {
                available -= transaction_size(t);
                num_trx++;
                transaction_append(selected, t);
            }

            unsigned int sig = siggen_new();
            sig = siggen_int(sig, id);
            sig = siggen_int(sig, prev_id);
            sig = siggen_int(sig, prev_sig);
            sig = siggen_int(sig, num_trx);
            printf("Block mined: %d %d 0x%8.8x %d\n", id, prev_id, prev_sig, num_trx);

            for (int i = 0; i < num_trx; i++) {
                trx_t *t = transaction_pop(selected);
                sig = siggen_int(sig, t->id);
                sig = siggen_string(sig, t->payer);
                sig = siggen_string(sig, t->payee);
                sig = siggen_int(sig, t->amount);
                sig = siggen_int(sig, t->fee);
                trx_print("", t);
                transaction_delete(t);
            }

            unsigned int nonce = nonce_find(event->num_threads, sig);
            sig = siggen_int(sig, nonce);
            printf("0x%8.8x 0x%8.8x\n", nonce, sig);
            prev_id = id;
            prev_sig = sig;
        }
        event_delete(event);
    }
    return 0;
}

