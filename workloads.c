#include "workload.h"

const int default_workload = 0;
struct workload builtin_workloads[] =
{
    {
        .name="Sx3",
        .vm_count=3,
        .vm_workloads = {
            { .phase_count = 2,
              .list = {
                    {
                    .type=PHASE_RUN,
                    .time=695
                    },
                    {
                    .type=PHASE_BLOCK,
                    .time=5
                    },
                }
            },
            { .phase_count = 2,
              .list = {
                    {
                    .type=PHASE_RUN,
                    .time=1095
                    },
                    {
                    .type=PHASE_BLOCK,
                    .time=5
                    },
                }
            },
            { .phase_count = 2,
              .list = {
                    {
                    .type=PHASE_RUN,
                    .time=1295
                    },
                    {
                    .type=PHASE_BLOCK,
                    .time=5
                    },
                }
            },
        }
    },
};
