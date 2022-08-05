//
// Created by Alex Brodsky on 2021-06-27.
//

#ifndef MINER_EVENT_Q_H
#define MINER_EVENT_Q_H

#include "transactions.h"

enum {
    E_END = 0,
    E_TRX,
    E_BLK,
    E_MINE,
    E_EPOCH,
    E_LAST
};

typedef struct event {
    struct event *next;
    int type;
    unsigned int id;
    unsigned int prev_id;
    unsigned int prev_sig;
    unsigned int num_t;
    unsigned int nonce;
    unsigned int sig;
    unsigned int num_threads;
    unsigned int num_trx;
    trx_t transactions[0];
} event_t;

extern void event_append(event_t *e);
extern event_t * event_remove();
extern void event_delete(event_t *e);
extern event_t * event_alloc(int num_trx);

#endif //MINER_EVENT_Q_H
