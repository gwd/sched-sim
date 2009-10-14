#ifndef __SIM_H
#define __SIM_H

#include "stats.h"
#include "workload.h"
#include "sched.h"

enum runstate {
    RUNSTATE_RUNNING,
    RUNSTATE_RUNNABLE,
    RUNSTATE_BLOCKED
};

enum {
    STATE_RUN,
    STATE_PREEMPT,
    STATE_WAKE,
    STATE_BLOCK,
    STATE_MAX
};

struct vm
{
    /* Public */
    int vid;

    enum runstate runstate;
    int processor;

    void *private;

    /* State: i.e., runstate.  Phase: "runnable" or "blocked".  A single "runnable" phase may go through
     * several "runnable" and "running" states. */
    int state_start_time;
    int time_this_phase;
    int was_preempted;

    struct {
        struct cycle_summary state[STATE_MAX];
    } stats;

    int phase_index;
    const struct sim_phase *e; /* Shortcut pointer to workload->list[phase_index] */
    const struct vm_workload *workload;

};

#define MAX_PCPU 16
struct global_pcpu_data {
    int count, idle;
    struct pcpu {
        int pid;
        int idle; /* Indicates may be woken up if work appears */
        struct vm* current;
    } pcpus[MAX_PCPU];
};
extern struct global_pcpu_data P;


#ifdef VM_DATA_PUBLIC
struct global_vm_data {
    int count;
    struct vm vms[MAX_VMS];
};
extern struct global_vm_data V;
#endif

struct vm* vm_from_vid(int vid);
#define current(_pid) (P.pcpus[(_pid)].current)
void sim_sched_timer(int time, int pid);
void sim_runstate_change(int now, struct vm *v, int new_runstate);

#endif /* __SIM_H */

