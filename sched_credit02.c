#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define ASSERT assert

#include "list.h"
#include "sim.h"


#define MAX_VMS 16
#define CREDIT_INIT  500
#define CREDIT_RESET 0
#define MAX_TIMER 200
#define MIN_TIMER 50

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

    int global_weight;
    int scale_factor; /* ? */

    int next_check;
} sched_priv;

/*
 * - Everyone starts at fixed value
 * - Burn credit at a constant rate
 * - Insert in runq based on credit
 * - Reset 
 *  - Triggered when someone reaches zero
 *  - Sets everyone to init
 * - Timeslice
 *  - Start with basic timeslice
 *  - Don't run for more credit than you have
 *  + Only run until your credit would equal next VM in runqueue
 *  - Never less than MIN_TIMER
 */

static void reset_credit(int time)
{
    int i;
    for ( i=0; i<=sched_priv.max_vm; i++)
    {
        sched_priv.vms[i].credit = CREDIT_INIT;
        sched_priv.vms[i].start_time = time;
    }
    /* No need to resort runq, as everyone's credit is now zero */
}

static void dump_credit(int time, struct sched_vm *svm)
{
    printf("credit v%d %d %d\n", svm->vid, time, svm->credit);
}

static void burn_credit(struct sched_vm *svm, int time)
{
    ASSERT(time >= svm->start_time);

    svm->credit -= (time - svm->start_time);
    svm->start_time = time;

    dump_credit(time, svm);
}

static int calc_timer(struct sched_vm *svm)
{
    int time;

    /* Start with basic timeslice */
    time = MAX_TIMER;

    /* If we have less credit than that, cut it down to our credits */
    if ( time > svm->credit )
        time = svm->credit;

    /* If there are other VMs on the runqueue, calculate
     * how much time until our credit will equal their credit.
     * If this is less than our timeslice, cut it down again. */
    if ( !list_empty(&sched_priv.runq) )
    {
        struct sched_vm *sq = list_entry(sched_priv.runq.next, struct sched_vm, runq_elem);

        ASSERT(svm->credit >= sq->credit);

        if ( time > (svm->credit - sq->credit) )
            time = (svm->credit - sq->credit);
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
    svm->weight = 1;
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
        int i, ipid=-1, lowest = svm->credit;
        

        for ( i=0; i<P.count; i++ )
        {
            if ( P.pcpus[i].idle )
            {
                printf(" %s: p%d idle, waking\n", __func__, i);
                ipid=i;
                break;
            }
            else if ( P.pcpus[i].current )
            {
                struct vm* ovm = P.pcpus[i].current;
                int ovid = ovm->vid;
                struct sched_vm *osvm = sched_priv.vms + ovid;

                /* Update credits of currently running VM */
                burn_credit(osvm, time);

                if ( osvm->credit < lowest )
                {
                    ipid = i;
                    lowest = osvm->credit;
                }
            }
            
        }

        if ( ipid >= 0 )
            sim_sched_timer(0, ipid);
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

struct scheduler sched_credit02 =
{
    .name="credit02",
    .desc="c01 + Preempt when your credit equals the next VM on the runqueue",
    .ops = {
        .sched_init = sched_credit_init,
        .vm_init    = sched_credit_vm_init,
        .wake       = sched_credit_wake,
        .schedule   = sched_credit_schedule
    }
};
