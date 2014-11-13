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
#define CREDIT_INIT  500
#define CREDIT_CLIP  50
#define CREDIT_RESET 0
#define MAX_TIMER 200
#define MIN_TIMER 50

/* FIXME: hack! */
int init_weight[] = { 70, 100, 100, 100, 200, 200 };

struct sched_vm {
    struct list_head runq_elem;
    struct vm *v;

    int weight;

    int credit;
    int credit_per_min_timer; /* ? */
    int start_time;
    int vid;
};

struct {
    struct list_head runq; /* Global run queue */
    int max_vm;
    struct sched_vm vms[MAX_VMS];
    int ncpus;

    int max_weight;
    int scale_factor; /* ? */

    int next_check;
} sched_priv;

/*
 * + Implement weights (hacky at the moment)
 * - Everyone starts at fixed value
 * - Burn credit based on variable weight
 * - Insert in runq based on credit
 * - Reset 
 *  - Triggered when someone reaches zero
 *  + Clip everyone's credit and add INIT
 * - Timeslice
 *  - Start with basic timeslice
 *  - Don't run for more credit than you have
 *  - Only run until your credit would equal next VM in runqueue
 *  - Never less than MIN_TIMER
 */

static int t2c(int time, struct sched_vm *svm)
{
    return time * sched_priv.max_weight / svm->weight;
}

static int c2t(int credit, struct sched_vm *svm)
{
    return credit * svm->weight / sched_priv.max_weight;
}

static void dump_credit(int time, struct sched_vm *svm)
{
    printf("credit v%d %d %d\n", svm->vid, time, svm->credit);
}

static void reset_credit(int time)
{
    int i;
    for ( i=0; i<=sched_priv.max_vm; i++)
    {
        struct sched_vm *svm = sched_priv.vms + i;
        int tmax = CREDIT_CLIP;

        if ( svm->credit > tmax )
            svm->credit = tmax;
        svm->credit += CREDIT_INIT;
        svm->start_time = time;

        dump_credit(time, svm);
    }
    /* No need to resort runq, as no one's credit is re-ordered */
}

static void burn_credit(struct sched_vm *svm, int time)
{
    ASSERT(time >= svm->start_time);

    svm->credit -= t2c(time - svm->start_time, svm);
    svm->start_time = time;

    dump_credit(time, svm);
}

static int calc_timer(struct sched_vm *svm)
{
    int time;

    /* Start with basic timeslice */
    time = MAX_TIMER;

    /* If we have less credit than that, cut it down to our credits */
    if ( time > c2t(svm->credit, svm) )
        time = c2t(svm->credit, svm);

    /* If there are other VMs on the runqueue, calculate
     * how much time until our credit will equal their credit.
     * If this is less than our timeslice, cut it down again. */
    if ( !list_empty(&sched_priv.runq) )
    {
        struct sched_vm *sq = list_entry(sched_priv.runq.next, struct sched_vm, runq_elem);

        ASSERT(svm->credit >= sq->credit);

        /* Time will be used for svm, so use it to scale c2t */
        if ( time > c2t(svm->credit - sq->credit, svm) )
            time = c2t(svm->credit - sq->credit, svm);
    }

    /* No matter what, always run for at least MIN_TIMER */
    if ( time < MIN_TIMER )
        time = MIN_TIMER;

    return time;
}

static void runq_insert(struct sched_vm *svm)
{
    struct list_head *iter;
    int pos = 0;

    list_for_each( iter, &sched_priv.runq )
    {
        struct sched_vm * iter_svm;

        iter_svm = list_entry(iter, struct sched_vm, runq_elem);

        if ( svm->credit > iter_svm->credit )
        {
            printf(" p%d v%d\n",
                   pos,
                   iter_svm->vid);
            break;
        }
        pos++;
    }

    list_add_tail(&svm->runq_elem, iter);
}

static void sched_credit_init(void)
{
    printf("%s()\n", __func__);
    INIT_LIST_HEAD(&sched_priv.runq);
    sched_priv.max_vm=0;
    sched_priv.max_weight = 0;
}

static void sched_credit_vm_init(int vid)
{
    struct sched_vm *svm;

    printf("%s: vm %d\n", __func__, vid);

    if ( vid > MAX_VMS )
    {
        fprintf(stderr, "vid %d > MAX_VMS %d!\n", vid, MAX_VMS);
        exit(1);
    }

    svm = sched_priv.vms + vid;

    INIT_LIST_HEAD(&svm->runq_elem);

    svm->vid = vid;
    svm->v = vm_from_vid(vid);

    svm->credit = CREDIT_INIT;
    /* FIXME */
    svm->weight = init_weight[vid];
    if ( sched_priv.max_weight < svm->weight )
        sched_priv.max_weight = svm->weight;
    svm->start_time = 0;
    
    if ( vid > sched_priv.max_vm )
        sched_priv.max_vm = vid;
}

static void sched_credit_wake(int time, int vid)
{
    struct vm *v;
    struct sched_vm *svm;

    v = vm_from_vid(vid);

    printf("%s: time %d vid %d\n",
           __func__, time, v->vid);

    svm = sched_priv.vms + v->vid;

    ASSERT(list_empty(&svm->runq_elem));

    runq_insert(svm);

    /* Scan for either:
     * + an idle cpu to wake up, or
     * + if there are cpus with lower credits, the lowest one
     */
    {
        int i, ipid=-1, lowest=-1, cdiff;

        for ( i=0; i<P.count; i++ )
        {
            if ( P.pcpus[i].idle )
            {
                printf(" %s: p%d idle, waking\n", __func__, i);
                ipid=i;
                lowest=0;
                break;
            }
            else if ( P.pcpus[i].current )
            {
                struct vm* ovm = P.pcpus[i].current;
                int ovid = ovm->vid;
                struct sched_vm *osvm = sched_priv.vms + ovid;

                /* Update credits of currently running VM */
                burn_credit(osvm, time);

                if ( lowest == -1 || osvm->credit < lowest )
                {
                    ipid = i;
                    lowest = osvm->credit;
                }
            }
            
        }

        cdiff = lowest - svm->credit;

        /* FIXME: c2t should be based on weight of running vm, not waiting vm! */
        if ( ipid >= 0 )
            sim_sched_timer((cdiff>0)?c2t(cdiff, svm):0, ipid);
        else
            dump_credit(time, svm);
    }
}

static struct vm* sched_credit_schedule(int time, int pid)
{
    struct sched_vm *svm;
    struct vm *next, *prev;
    int timer;

    printf("%s: time %d pid %d\n",
           __func__, time, pid);
    prev = current(pid);

    if ( prev )
    {
        printf(" current v%d\n", prev->vid);
        svm = sched_priv.vms + prev->vid;

        burn_credit(svm, time);

        if ( svm->v->runstate == RUNSTATE_RUNNING )
        {
            printf(" adding to runqueue\n");
            runq_insert(svm);
        }
    }

    /* Take guy on front of runqueue, set new timer */
    if ( list_empty(&sched_priv.runq) )
    {
        printf(" No runnable entities\n");
        return NULL;
    }

    svm = list_entry(sched_priv.runq.next, struct sched_vm, runq_elem);

    list_del_init(&svm->runq_elem);

    next = svm->v;

    if ( svm->credit <= CREDIT_RESET )
    {
        printf(" vid %d credit %c, resetting credit at time %d\n",
               svm->vid,
               svm->credit,
               time);
        reset_credit(time);
    }

    dump_credit(time, svm);
    svm->start_time = time;

    timer = calc_timer(svm);

    sim_sched_timer(timer, pid);

    printf(" next: v%d\n", next->vid);
    
    return next;
}

struct scheduler sched_credit03 =
{
    .name="credit03",
    .desc="c02 + implement weight + clip-and-add on reset",
    .ops = {
        .sched_init = sched_credit_init,
        .vm_init    = sched_credit_vm_init,
        .wake       = sched_credit_wake,
        .schedule   = sched_credit_schedule
    }
};
