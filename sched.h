#ifndef _SCHED_H
#define _SCHED_H

struct sched_ops {
    void (*sched_init)(void);
    void (*vm_init)(int vid);
    void (*wake)(int time, int vid);
    struct vm* (*schedule)(int time, int pid);
};

struct scheduler {
    char *name;
    char *desc;
    struct sched_ops ops;
};

extern struct scheduler *schedulers[];

#endif
