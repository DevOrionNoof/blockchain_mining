//
// Created by Alex Brodsky on 2021-06-27.
//

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "reader.h"
#include "transactions.h"
#include "event_q.h"

#define END_EVENT "END"
#define MINE_EVENT "MINE"
#define TRX_EVENT "TRX"
#define BLK_EVENT "BLK"
#define EPOCH_EVENT "EPOCH"

static pthread_t started_tid;

static void trx_read(trx_t *trx) {
    int n = scanf("%u %s %s %u %u", &trx->id, trx->payer,
                  trx->payee, &trx->amount, &trx->fee);
    assert(n == 5);
}

static void *runner(void *arg) {
    char buffer[10]; // Assume input will be correct.

    for (;;) {
        event_t * event;
        scanf("%s", buffer);  // assume input will be correct
        if (!strcmp(buffer, END_EVENT)) {
            event = event_alloc(0);
            event->type = E_END;
            printf("Received event END\n");
        } else if (!strcmp(buffer, EPOCH_EVENT)) {
            event = event_alloc(0);
            event->type = E_EPOCH;
            printf("Received event EPOCH\n");
        } else if (!strcmp(buffer, TRX_EVENT)) {
            event = event_alloc(1);
            event->type = E_TRX;
            trx_read(event->transactions);
            event->id = event->transactions->id;
            printf("Received event TRX with ID %d\n", event->id);
        } else if (!strcmp(buffer, BLK_EVENT)) {
            unsigned int block_id;
            unsigned int num_trx;
            unsigned int prev_id;
            unsigned int prev_sig;

            int n = scanf("%u %u %i %u", &block_id, &prev_id,
                          &prev_sig, &num_trx);
            assert(n == 4);

            event = event_alloc(num_trx);
            event->type = E_BLK;
            event->id = block_id;
            event->num_trx = num_trx;
            event->prev_id = prev_sig;
            event->prev_id = prev_id;

            for (int i = 0; i < num_trx; i++) {
                trx_read(&event->transactions[i]);
            }

            n = scanf("%i %i", &event->nonce, &event->sig);
            assert(n == 2);
            printf("Received event BLK with ID %d\n", event->id);
        } else if (!strcmp(buffer, MINE_EVENT)) {
            event = event_alloc(0);
            event->type = E_MINE;
            int n = scanf("%d", &event->num_threads);
            assert(n == 1);
            printf("Received event MINE\n");
        } else {
            assert(0);
        }

        event_append(event);
        if (event->type == E_END) {
            break;
        }
    }

    return NULL;
}

extern void reader_start() {
    int fail = pthread_create(&started_tid, NULL, runner, NULL);
    assert(!fail);
}

extern void reader_join() {
    int fail = pthread_join(started_tid, NULL);
    assert(!fail);
}
