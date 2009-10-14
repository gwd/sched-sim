#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define ASSERT assert

#include "stats.h"
#include "list.h"
#include "sim.h"
#include "workload.h"
#include "sched.h"
#include "options.h"

FILE *warn;

enum event_type {
    EVT_BLOCK,
    EVT_WAKE,
    EVT_TIMER,
    EVT_MAX
};

char *event_name[EVT_MAX] = {
    [EVT_BLOCK]="block",
    [EVT_WAKE] ="wake ",
    [EVT_TIMER]="timer"
};

struct event {
    struct list_head event_list;
    enum event_type type;
    int time;
    int param;  /* Usually VM ID */ 
};

char * state_name[STATE_MAX] = {
    [STATE_RUN]=    "run    ",
    [STATE_PREEMPT]="preempt",
    [STATE_WAKE]=   "wake   ",
    [STATE_BLOCK]=  "block  ",
};

struct {
    int now;
    struct list_head events;
    struct list_head *timer;
    const struct sched_ops *sched_ops;
} sim;


#ifndef VM_DATA_PUBLIC
struct global_vm_data {
    int count;
    struct vm vms[MAX_VMS];
};
#endif
struct global_vm_data V;

extern struct scheduler sched_rr;
int default_scheduler = 0;
struct scheduler *schedulers[] =
{
    &sched_rr,
    NULL
};

/* Options */

struct global_pcpu_data P;

/* Sim list interface */
/* NB: Caller must free if they're not going to use it! */
#define list_event(_l) (list_entry((_l), struct event, event_list))

struct event* sim_remove_event(int type, int param)
{
    struct event* ret = NULL;
    struct list_head *pos, *tmp;

    /* Look for an event that matches this one and remove it */
    list_for_each_safe(pos, tmp, &sim.events)
    {
        struct event *tevt = list_event(pos);
        if ( tevt->type == type
             && tevt->param == param )
        {
            list_del(pos);
            ret = tevt;
            break;
        }
    }

    return ret;
}

void sim_insert_event(int time, int type, int param, int reset)
{
    struct list_head *pos = NULL;
    struct event *evt=NULL;

    ASSERT(time >= sim.now);

    if ( reset )
        evt=sim_remove_event(type, param);

    if ( !evt )
        evt = (struct event *)malloc(sizeof(*evt));

    evt->time = time;
    evt->type = type;
    evt->param = param;

    printf(" [insert t%d %s param%d]\n",
           evt->time, event_name[evt->type], evt->param);

    INIT_LIST_HEAD(&evt->event_list);

    list_for_each(pos, &sim.events)
    {
        if ( list_event(pos)->time > evt->time )
            break;
    }
    list_add_tail(&evt->event_list, pos);
}

struct event sim_next_event(void)
{
    struct event *evt;
    struct list_head *next;

    ASSERT(!list_empty(&sim.events));

    next=sim.events.next;

    list_del(next);
    
    evt=list_event(next);

    printf("%d: evt %s param%d\n",
           evt->time, event_name[evt->type], evt->param);

    free(evt);

    /* XXX */
    return *evt;
}

/*
 * VM simulation
 */
void vm_next_event(struct vm *v)
{
    v->phase_index = ( v->phase_index + 1 ) % v->workload->phase_count;

    v->e = v->workload->list + v->phase_index;
}

struct vm* vm_from_vid(int vid)
{
    if ( vid >= V.count )
    {
        fprintf(stderr, "%s: v%d >= V.count %d!\n",
                __func__, vid, V.count);
        exit(1);
    }

    return V.vms + vid;
}

void vm_block(int now, struct vm *v)
{
    ASSERT(v->e->type == PHASE_RUN);
    v->time_this_phase += now - v->state_start_time;
    printf("%s: v%d time_this_phase %d\n",
           __func__, v->vid, v->time_this_phase);

    ASSERT(v->time_this_phase == v->e->time);

    vm_next_event(v);
    
    ASSERT(v->e->type == PHASE_BLOCK);

    sim_insert_event(now + v->e->time, EVT_WAKE, v->vid, 0);
    v->time_this_phase = 0;
    v->was_preempted = 0;
}

/* Called when wake event happens; increment timer and reset state */
void vm_wake(int now, struct vm *v)
{
    ASSERT(v->e->type == PHASE_BLOCK);
    ASSERT(v->time_this_phase == 0);

    v->time_this_phase = now - v->state_start_time;

    if ( now != 0 )
        ASSERT(v->time_this_phase == v->e->time);

    vm_next_event(v);

    v->time_this_phase = 0;
}

/* Called when actually starting to run; make block event and set state */
void vm_run(int now, struct vm *v)
{
    ASSERT(v->e->type == PHASE_RUN);
    ASSERT(v->time_this_phase < v->e->time);

    sim_insert_event(now + v->e->time - v->time_this_phase, EVT_BLOCK, v->vid, 0);
    v->state_start_time = now;
}

/* Preempt: Remove block event, update amount of runtime (so that when it runs again we can accurately
 * generate a new block event) */
void vm_preempt(int now, struct vm *v)
{
    struct event* evt;

    if ( ( evt = sim_remove_event(EVT_BLOCK, v->vid) ) )
        free(evt);

    v->time_this_phase += now - v->state_start_time;
    printf("%s: v%d time_this_phase %d\n",
           __func__, v->vid, v->time_this_phase);

    ASSERT(v->time_this_phase < v->e->time);

    v->was_preempted = 1;
}


/* Callbacks the scheduler may make */
void sim_sched_timer(int time, int pid)
{
    if ( pid >= P.count )
    {
        fprintf(stderr, "%s: p%d >= P.count %d\n",
                __func__, pid, P.count);
        exit(1);
    }

    if ( P.pcpus[pid].idle )
    {
        P.pcpus[pid].idle = 0;
        P.idle--;
    }
    sim_insert_event(sim.now + time, EVT_TIMER, pid, 1);
}

void sim_runstate_change(int now, struct vm *v, int new_runstate)
{
    int ostate, nstate;
    int stime = now - v->state_start_time;

    /* Valid transitions:
     * + R->A (preemption): remove block event
     * + R->B (block)     : Insert wake event
     * + A->R (run)       : Insert block event
     * + B->A (wake)      : No action necessary
     */

    switch ( v->runstate )
    {
    case RUNSTATE_RUNNING:
        ostate = STATE_RUN;
        break;
    case RUNSTATE_RUNNABLE:
        if ( v->was_preempted )
            ostate = STATE_PREEMPT;
        else
            ostate = STATE_WAKE;
        break;
    case RUNSTATE_BLOCKED:
        ostate = STATE_BLOCK;
        break;
    }

    update_cycles(&v->stats.state[ostate], stime);


    if ( v->runstate == RUNSTATE_RUNNING
         && new_runstate == RUNSTATE_RUNNABLE )
    {
        nstate = STATE_PREEMPT;
        vm_preempt(now, v);
    }
    else if ( v->runstate == RUNSTATE_RUNNING
              && new_runstate == RUNSTATE_BLOCKED )
    {
        nstate = STATE_BLOCK;
        vm_block(now, v);
    }
    else if ( v->runstate == RUNSTATE_RUNNABLE
              && new_runstate == RUNSTATE_RUNNING )
    {
        nstate = STATE_RUN;
        vm_run(now, v);
    }
    else if ( v->runstate == RUNSTATE_BLOCKED
              && new_runstate == RUNSTATE_RUNNABLE )
    {
        nstate = STATE_WAKE;
        vm_wake(now, v);
    }
    else
        goto unexpected_transition;

    printf("%d: v%d %s %d -> %s\n",
           now, v->vid, state_name[ostate], stime, state_name[nstate]);

    v->runstate = new_runstate;
    v->state_start_time = now;

    return;

unexpected_transition:
    fprintf(stderr, "Unexpected transition for vm %d: %d->%d\n",
            v->vid,
            v->runstate,
            new_runstate);
    exit(1);
}

/* 
 * Main loop
 */
void simulate(void)
{
    while ( sim.now < opt.time_limit )
    {
        /* Take next event off list */
        struct event evt;

        evt = sim_next_event();

        sim.now = evt.time;

        switch(evt.type)
        {
        case EVT_WAKE:
        {
            struct vm *v = vm_from_vid(evt.param);
            ASSERT(v->processor == -1);
            sim_runstate_change(sim.now, v, RUNSTATE_RUNNABLE);
            sim.sched_ops->wake(sim.now, v->vid);
        }
        break;
        case EVT_BLOCK:
        {
            struct vm *v = vm_from_vid(evt.param);

            ASSERT(v->processor != -1);
            ASSERT(v->processor <= P.count);

            sim_runstate_change(sim.now, v, RUNSTATE_BLOCKED);

            evt.param = v->processor; /* FIXME */
        }
        /* FALL-THRU */
        case EVT_TIMER:
        {
            struct vm *prev, *next;
            int pid = evt.param;

            ASSERT(pid < P.count);

            prev = P.pcpus[pid].current;

            next = sim.sched_ops->schedule(sim.now, pid);

            if ( prev && prev != next )
            {
                prev->processor = -1;
                if( prev->runstate != RUNSTATE_BLOCKED )
                    sim_runstate_change(sim.now, prev, RUNSTATE_RUNNABLE);
            }

            
            P.pcpus[pid].current = next;
            if ( next )
            {
                if ( next != prev )
                {
                    sim_runstate_change(sim.now, next, RUNSTATE_RUNNING);
                    next->processor = pid;
                }
            }
            else
            {
                P.pcpus[pid].idle = 1;
                P.idle++;
            }
        }
        break;
        default:
            fprintf(stderr, "Unexpected event type: %d\n", evt.type);
            exit(1);
            break;
        }
    }
}

void init(void)
{
    int vid, i;
    const struct workload *w;

    /* Initialize simulation variables */
    sim.now=0;
    sim.timer=NULL;
    INIT_LIST_HEAD(&sim.events);
    sim.sched_ops = &opt.scheduler->ops;

    /* Initialize pcpus */
    P.count = opt.pcpu_count;
    P.idle = 0;
    for ( i=0; i<P.count; i++ )
    {
        P.pcpus[i].pid = i;
        P.pcpus[i].idle = 1;
        P.idle++;
        P.pcpus[i].current = NULL;
    }

    /* Initialize scheduler */
    sim.sched_ops->sched_init();

    /* Initialize vms */
    w=opt.workload;
    V.count = 0;
    for ( vid=0; vid<w->vm_count; vid++)
    {
        struct vm *v = V.vms+vid;

        v->vid = vid;
        v->runstate = RUNSTATE_BLOCKED;
        v->processor = -1;
        v->private = NULL;

        v->state_start_time = 0;
        v->time_this_phase = 0;
        

        v->phase_index = -1;
        v->e = NULL;
        v->workload = w->vm_workloads+vid;
        
        V.count++;

        sim.sched_ops->vm_init(vid);
    }

    /* Set VM starting conditions */
    for ( vid=0; vid<V.count; vid++)
    {
        struct vm *v = V.vms+vid;

        switch(v->workload->list[0].type)
        {
        case PHASE_RUN:
            v->phase_index = v->workload->phase_count - 1;
            v->e = v->workload->list + v->phase_index;

            sim_insert_event(sim.now, EVT_WAKE, v->vid, 0);
            v->state_start_time = sim.now;
            v->time_this_phase = 0;
            break;
        case PHASE_BLOCK:
            v->phase_index = 0;
            v->e = v->workload->list;

            sim_insert_event(sim.now + v->e->time, EVT_WAKE, v->vid, 0);
            v->state_start_time = sim.now;
            v->time_this_phase = 0;
            break;
        }
    }
}

void report(void)
{
    int i, j;

    for ( i=0; i<V.count; i++ )
    {
        struct vm *v = V.vms + i;

        printf("VM %d\n", i);
        for ( j = 0; j < STATE_MAX ; j++ )
        {
            char s[128];
            snprintf(s, 128, " %s", state_name[j]);
            print_cycle_summary(&v->stats.state[j],   sim.now, s);
        }
    }
}

int main(int argc, char * argv[])
{
    warn = stdout;

    parse_options(argc, argv);

    /* Setup simulation */
    init();
    
    /* Run simulation */
    simulate();
    /* Report statistics */
    report();
}
