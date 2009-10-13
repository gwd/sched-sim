#ifndef _STATS_H
#define _STATS_H

typedef unsigned long long tsc_t;

struct cycle_summary {
    int count;
    unsigned long long cycles;
    long long *sample;
};

void set_sample_size(int n);
void update_cycles(struct cycle_summary *s, long long c);
void print_cycle_summary(struct cycle_summary *s,
                         tsc_t total, char *p);
#endif
