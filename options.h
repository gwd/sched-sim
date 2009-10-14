#ifndef _OPTIONS_H
#define _OPTIONS_H
struct options {
    int time_limit;
    int pcpu_count;
    const struct workload * workload;
    const struct scheduler * scheduler;
} opt;

extern struct options opt;
void parse_options(int argc, char * argv[]);

#endif
