//
// Created by Alex Brodsky on 2021-06-04.
//

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include "siggen.h"
#include "nonce.h"


struct request {
    unsigned int num_threads;
    unsigned int thread_id;
    pthread_mutex_t lock;
    unsigned int sig;
    unsigned int nonce;
};

static void print_msg(unsigned int id, unsigned int nonce) {
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    int fail = pthread_mutex_lock(&lock);
    assert(!fail);
    printf("Thread %u checking nonce 0x%8.8x\n", id, nonce);
    fail = pthread_mutex_unlock(&lock);
    assert(!fail);
}


static void update(struct request *req, unsigned int nonce) {
    int fail = pthread_mutex_lock(&req->lock);
    assert(!fail);
    if (nonce < req->nonce) {
        req->nonce = nonce;
    }
    fail = pthread_mutex_unlock(&req->lock);
    assert(!fail);
}


static void *runner(void *arg) {
    unsigned int id;
    struct request *req = (struct request *)arg;

    int fail = pthread_mutex_lock(&req->lock);
    assert(!fail);
    id = req->thread_id;
    req->thread_id++;
    fail = pthread_mutex_unlock(&req->lock);
    assert(!fail);

    unsigned int nonce = id;
    do {
        print_msg(id, nonce);
        if ((siggen_int(req->sig, nonce) & 0xff000000) == 0) {
            update(req, nonce);
            break;
        } else if (req->nonce < nonce) {
            break;
        }
        nonce += req->num_threads;
    } while (nonce >= req->num_threads);

    return NULL;
}


extern unsigned int nonce_find(unsigned int num_threads, unsigned int sig) {
    pthread_t tid[num_threads];
    struct request req;

    req.num_threads = num_threads;
    req.nonce = 0xffffffff;
    pthread_mutex_init(&req.lock, NULL);
    req.thread_id = 0;
    req.sig = sig;

    for (int i = 0; i < num_threads; i++) {
        int fail = pthread_create(&tid[i], NULL, runner, &req);
        assert(!fail);
    }

    for (int i = 0; i < num_threads; i++) {
        int fail = pthread_join(tid[i], NULL);
        assert(!fail);
    }

    return req.nonce;
}