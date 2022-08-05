//
// Created by Alex Brodsky on 2021-06-27.
//

#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include "event_q.h"

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
static event_t * head;
static event_t * tail;

extern void event_append(event_t *e) {
    int fail = pthread_mutex_lock(&lock);
    assert(!fail);
    if (head == NULL) {
        head = e;
        fail = pthread_cond_signal(&empty);
        assert(!fail);
    } else {
        tail->next = e;
    }
    tail = e;
    fail = pthread_mutex_unlock(&lock);
    assert(!fail);
}


extern event_t * event_remove() {
    int fail = pthread_mutex_lock(&lock);
    assert(!fail);
    if (head == NULL) {
        fail = pthread_cond_wait(&empty, &lock);
        assert(!fail);
    }
    assert(head);
    event_t *e = head;
    head = head->next;
    fail = pthread_mutex_unlock(&lock);
    assert(!fail);
    return e;
}


extern void event_delete(event_t *e) {
    assert(e);
    free(e);
}


extern event_t * event_alloc(int num_trx) {
    event_t *e = calloc(1, sizeof(event_t) + num_trx * sizeof(trx_t));
    assert(e);
    return e;
}
