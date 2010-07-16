#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>

#include "sim.h"
#include "workload.h"
#include "sched.h"
#include "options.h"

extern int default_scheduler;

struct options opt = {
    .time_limit = 100000,
    .pcpu_count = 2,
    .workload = NULL,
    .scheduler = NULL,
};

enum {
    OPT_NULL = 0,
    OPT_WORKLOAD,
    OPT_SCHEDULER,
    OPT_PCPU_COUNT,
    OPT_TIME_LIMIT,
    OPT_LIST_WORKLOADS,
    OPT_LIST_SCHEDULERS
};

const struct argp_option cmd_opts[] =  {
    { .name = "workload",
      .key = OPT_WORKLOAD,
      .arg="workload-name",
      .doc = "Synthetic workload to run.", },
    { .name = "scheduler",
      .arg="scheduler-name",
      .key = OPT_SCHEDULER,
      .doc = "Chose scheduler to run.", },
    { .name = "pcpus",
      .arg="n",
      .key = OPT_PCPU_COUNT,
      .doc = "Number of pcpus to simulate.  Defaults to 2.", },
    { .name = "time",
      .arg="n",
      .key = OPT_TIME_LIMIT,
      .doc = "How long to simulate.  Default 100000", },
    { .name = "list-schedulers",
      .key = OPT_LIST_SCHEDULERS,
      .doc = "List available schedulers.  Simulation won't run if this option is set.", },
    { .name = "list-workloads",
      .key = OPT_LIST_WORKLOADS,
      .doc = "List available workloads.  Simulation won't run if this option is set.", },

    { 0 },
};

error_t cmd_parser(int key, char *arg, struct argp_state *state)
{
    static int dont_run = 0;

    switch (key)
    {
    case OPT_LIST_SCHEDULERS:
    {
        int i;
        printf("Schedulers:\n");
        for ( i=0; schedulers[i]; i++)
            printf(" %15s: %s\n", schedulers[i]->name, schedulers[i]->desc);
        dont_run=1;
        break;
    }   
    case OPT_LIST_WORKLOADS:
    {
        int i;
        printf("Workloads:\n");
        for ( i=0; builtin_workloads[i].name; i++)
            printf(" %s\n", builtin_workloads[i].name);
        dont_run=1;
        break;
    }   
    case OPT_WORKLOAD:
    {
        int i;

        for ( i=0; builtin_workloads[i].name; i++)
        {
            if ( !strcmp(builtin_workloads[i].name, arg) )
                break;
        }
        if ( !builtin_workloads[i].name )
        {
            fprintf(stderr, "Unknown workload: %s\n",
                    arg);
            exit(1);
        }
        opt.workload = builtin_workloads + i;
    }
        break;
    case OPT_SCHEDULER:
    {
        int i;

        for ( i=0; schedulers[i]; i++)
        {
            if ( !strcmp(schedulers[i]->name, arg) )
                break;
        }
        if ( !schedulers[i] )
        {
            fprintf(stderr, "Unknown scheduler: %s\n",
                    arg);
            exit(1);
        }
        opt.scheduler = schedulers[i];
    }
        break;
    case OPT_PCPU_COUNT:
    {
        char *inval;

        opt.pcpu_count=(int)strtol(arg, &inval, 0);
        if ( inval == arg )
            argp_usage(state);
    }
        break;
    case OPT_TIME_LIMIT:
    {
        char *inval;

        opt.time_limit=(int)strtol(arg, &inval, 0);
        if ( inval == arg )
            argp_usage(state);
    }
        break;
    case ARGP_KEY_ARG:
    {
        argp_usage(state);
    }
    break;
    case ARGP_KEY_END:
    {
        if ( dont_run )
            exit(0);

        if ( !opt.workload )
            opt.workload = builtin_workloads+default_workload;

        if ( !opt.scheduler )
            opt.scheduler = schedulers[default_scheduler];
    }
    break;

    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

const struct argp parser_def = {
    .options = cmd_opts,
    .parser = cmd_parser,
    .args_doc = "",
    .doc = "",
};

const char *argp_program_version = "Scheduler simulator";
const char *argp_program_bug_address = "George Dunlap <george.dunlap@eu.citrix.com>";

void parse_options(int argc, char * argv[]) {
    argp_parse(&parser_def, argc, argv, 0, NULL, NULL);
}
