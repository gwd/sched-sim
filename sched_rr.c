/*
    Copyright (C) 2010 Citrix Systems UK Ltd
    Author: George Dunlap

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define ASSERT assert

#include "list.h"
#include "sim.h"


#define MAX_VMS 16
#define TSLICE 1000

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

void sched_rr_wake(int time, int vid)
{
    struct vm *v;
    struct sched_vm *svm;

    v = vm_from_vid(vid);

    printf("%s: time %d vid %d\n",
           __func__, time, v->vid);

    svm = sched_priv.vms + v->vid;

    ASSERT(list_empty(&svm->queue));

    list_add_tail(&svm->queue, &sched_priv.queue);

    /* Never preempt on wake; only kick idle processors */
    if ( P.idle > 0 )
    {
        int i;

        for ( i=0; i<P.count; i++ )
            if ( P.pcpus[i].idle )
                break;

        printf(" %s: waking p%d\n", __func__, i);
        sim_sched_timer(0, i);
    }
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
    .desc="Basic round-robin scheduler.",
    .ops = {
        .sched_init = sched_rr_init,
        .vm_init    = sched_rr_vm_init,
        .wake       = sched_rr_wake,
        .schedule   = sched_rr_schedule
    }
};
