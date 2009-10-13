#ifndef __WORKLOAD_H
#define __WORKLOAD_H
struct sim_phase {
    enum { PHASE_RUN, PHASE_BLOCK } type;
    int time;
};

#define MAX_VMS 16
#define MAX_PHASES 16
struct vm_workload {
    int phase_count;
    const struct sim_phase list[MAX_PHASES];
};

struct workload {
    const char * name;
    int vm_count;
    const struct vm_workload vm_workloads[MAX_VMS];
};

extern const int default_workload;
extern struct workload builtin_workloads[];
#endif
