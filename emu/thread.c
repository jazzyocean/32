#include <pthread.h>
#include <stdio.h>
#include "types.h"
#include "processor.h"
#include "debug.h"

void *cpu_thr(void *arg) {
    Arch *arch = (Arch*)arg;
    pthread_barrier_wait(&arch->barrier);
    mainloop(arch);
    return NULL;
}

void *biosc_thr(void *arg) {
    Arch *arch = (Arch*)arg;
    pthread_barrier_wait(&arch->barrier);
    return NULL;
}

void *biosp_thr(void *arg) {
    Arch *arch = (Arch*)arg;
    pthread_barrier_wait(&arch->barrier);
    return NULL;
}

int init_cpu_thr(Arch *arch) {
    pthread_t thread_id;
    int res = pthread_create(&thread_id, NULL, cpu_thr, (void*)arch);
    if (res) return res;
    arch->processor.cpu_thr_id = thread_id;
    #ifdef DEBUG
        printf(DBGTAG_THREAD"Initialized CPU_THR with thread id %I64d\n", CALC_TICKSARR, thread_id);
    #endif
    return 0;
}

int init_biosc_thr(Arch *arch) {
    pthread_t thread_id;
    int res = pthread_create(&thread_id, NULL, biosc_thr, (void*)arch);
    if (res) return res;
    arch->bios.biosc_thr_id = thread_id;
    #ifdef DEBUG
        printf(DBGTAG_THREAD"Initialized BIOSC_THR with thread id %I64d\n", CALC_TICKSARR, thread_id);
    #endif
    return 0;
}

int init_biosp_thr(Arch *arch) {
    pthread_t thread_id;
    int res = pthread_create(&thread_id, NULL, biosp_thr, (void*)arch);
    if (res) return res;
    arch->bios.biosp_thr_id = thread_id;
    #ifdef DEBUG
        printf(DBGTAG_THREAD"Initialized BIOSP_THR with thread id %I64d\n", CALC_TICKSARR, thread_id);
    #endif
    return 0;
}

int init_thr(Arch *arch) {
    pthread_barrier_t barrier;
    int res = pthread_barrier_init(&barrier, NULL, 3);
    if (res) {
        #ifdef DEBUG
            printf(DBGTAG_THREAD"Failed to initialize thread barrier with error %d\n", CALC_TICKSARR, res);
        #endif
        return res;
    }
    arch->barrier = barrier;

    res = init_cpu_thr(arch);
    if (res) {
        #ifdef DEBUG
            printf(DBGTAG_THREAD"Failed to initialize CPU_THR with error %d\n", CALC_TICKSARR, res);
        #endif
        return res;
    }
    res = init_biosc_thr(arch);
    if (res) {
        #ifdef DEBUG
            printf(DBGTAG_THREAD"Failed to initialize BIOSC_THR with error %d\n", CALC_TICKSARR, res);
        #endif
        arch->processor.on = 0;
        return res;
    }
    res = init_biosp_thr(arch);
    if (res) {
        #ifdef DEBUG
            printf(DBGTAG_THREAD"Failed to initialize BIOSP_THR with error %d\n", CALC_TICKSARR, res);
        #endif
        arch->processor.on = 0;
        return res;
    }
    return 0;
}

int thr_join(Arch *arch) {
    pthread_join(arch->processor.cpu_thr_id, NULL);
    pthread_join(arch->bios.biosc_thr_id, NULL);
    pthread_join(arch->bios.biosp_thr_id, NULL);
    
    return 0;
}