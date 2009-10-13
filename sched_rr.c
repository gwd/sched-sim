#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define ASSERT assert

#include "list.h"
#include "sim.h"


#define MAX_VMS 16
#define TSLICE 2000

struct sched_vm {
    struct list_head queue;
    int vid;
    struct vm *v;
};

struct {
    struct list_head queue;
    struct sched_vm vms[MAX_VMS];
} sched_priv;


void sched_rr_init(void)
{
    printf("%s()\n", __func__);
    INIT_LIST_HEAD(&sched_priv.queue);
}

void sched_rr_vm_init(int vid)
{
    struct sched_vm *svm;

    printf("%s: vm %d\n", __func__, vid);

    if ( vid > MAX_VMS )
    {
        fprintf(stderr, "vid %d > MAX_VMS %d!\n", vid, MAX_VMS);
        exit(1);
    }

    svm = sched_priv.vms + vid;

    INIT_LIST_HEAD(&svm->queue);

    svm->vid = vid;
    svm->v = vm_from_vid(vid);
    
}

void sched_rr_wake(int time, struct vm * v)
{
    struct sched_vm *svm;

    printf("%s: time %d vid %d\n",
           __func__, time, v->vid);

    svm = sched_priv.vms + v->vid;

    ASSERT(list_empty(&svm->queue));

    list_add_tail(&svm->queue, &sched_priv.queue);
}

struct vm* sched_rr_schedule(int time, int pid)
{
    struct sched_vm *svm;
    struct vm *next, *prev;

    printf("%s: time %d pid %d\n",
           __func__, time, pid);
    prev = current(pid);

    if ( prev )
    {
        printf(" current v%d\n", prev->vid);
        svm = sched_priv.vms + prev->vid;

        if ( svm->v->runstate == RUNSTATE_RUNNING )
        {
            printf(" adding to runqueue\n");
            list_add_tail(&svm->queue, &sched_priv.queue);
        }
    }

    /* Take guy on front of runqueue, set new timer */
    if ( list_empty(&sched_priv.queue) )
    {
        printf(" No runnable entities\n");
        return NULL;
    }

    svm = list_entry(sched_priv.queue.next, struct sched_vm, queue);

    list_del_init(&svm->queue);
    next = svm->v;

    sim_sched_timer(TSLICE, pid);

    printf(" next: v%d\n", next->vid);
    
    return next;
}

struct scheduler sched_rr =
{
    .name="round-robin",
    .ops = {
        .sched_init = sched_rr_init,
        .vm_init    = sched_rr_vm_init,
        .wake       = sched_rr_wake,
        .schedule   = sched_rr_schedule
    }
};
