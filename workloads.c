#include <stdlib.h>
#include "workload.h"

const int default_workload = 0;
struct workload builtin_workloads[] =
{
    {
        .name="r1",
        .vm_count=3,
        .vm_workloads = {
            { .phase_count = 2,
              .list = {
                    {
                    .type=PHASE_RUN,
                    .time=70
                    },
                    {
                    .type=PHASE_BLOCK,
                    .time=250
                    },
                }
            },
            { .phase_count = 2,
              .list = {
                    {
                    .type=PHASE_RUN,
                    .time=500
                    },
                    {
                    .type=PHASE_BLOCK,
                    .time=500
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
    {
        .name="N2s2a1",
        .vm_count=6,
        .vm_workloads = {
            { .phase_count = 2, .list = {
                    { .type=PHASE_BLOCK, .time=20 },
                    { .type=PHASE_RUN,   .time=5 }
                } },
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=15 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=16 },
                    { .type=PHASE_BLOCK, .time=6 },  
                    { .type=PHASE_RUN, .time=14 },
                    { .type=PHASE_BLOCK, .time=4 },  
                    { .type=PHASE_RUN, .time=87 },
                    { .type=PHASE_BLOCK, .time=30 },
                } },
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=13 },
                    { .type=PHASE_BLOCK, .time=10 },  
                    { .type=PHASE_RUN, .time=17 },
                    { .type=PHASE_BLOCK, .time=7 },  
                    { .type=PHASE_RUN, .time=15 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=30 },
                    { .type=PHASE_BLOCK, .time=2 },
                } },
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=66 },
                    { .type=PHASE_BLOCK, .time=30 },  
                    { .type=PHASE_RUN, .time=5 },
                    { .type=PHASE_BLOCK, .time=50 },  
                    { .type=PHASE_RUN, .time=70 },
                    { .type=PHASE_BLOCK, .time=42 },  
                    { .type=PHASE_RUN, .time=80 },
                    { .type=PHASE_BLOCK, .time=41 },
                    /* Run: 221 block: 163 57.5% */
                } },
            { .phase_count = 6, .list = {
                    { .type=PHASE_RUN, .time=1250 },
                    { .type=PHASE_BLOCK, .time=10 },  
                    { .type=PHASE_RUN, .time=10 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=10 },
                    { .type=PHASE_BLOCK, .time=5 },  
                } },
            { .phase_count = 6, .list = {
                    { .type=PHASE_RUN, .time=850 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=7 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=9 },
                    { .type=PHASE_BLOCK, .time=5 },  
                } },
        }
    },
    {
        .name="n2s2",
        .vm_count=5,
        .vm_workloads = {
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=50 },
                    { .type=PHASE_BLOCK, .time=50 },  
                    { .type=PHASE_RUN, .time=45 },
                    { .type=PHASE_BLOCK, .time=67 },  
                    { .type=PHASE_RUN, .time=55 },
                    { .type=PHASE_BLOCK, .time=82 },  
                    { .type=PHASE_RUN, .time=37 },
                    { .type=PHASE_BLOCK, .time=100 },
                    /* Run: 187 block: 299  38.4% */
                } },
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=45 },
                    { .type=PHASE_BLOCK, .time=67 },  
                    { .type=PHASE_RUN, .time=50 },
                    { .type=PHASE_BLOCK, .time=50 },  
                    { .type=PHASE_RUN, .time=55 },
                    { .type=PHASE_BLOCK, .time=82 },  
                    { .type=PHASE_RUN, .time=41 },
                    { .type=PHASE_BLOCK, .time=80 },
                    /* Run: 191 block: 279 40.6% */
                } },
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=66 },
                    { .type=PHASE_BLOCK, .time=30 },  
                    { .type=PHASE_RUN, .time=5 },
                    { .type=PHASE_BLOCK, .time=50 },  
                    { .type=PHASE_RUN, .time=70 },
                    { .type=PHASE_BLOCK, .time=42 },  
                    { .type=PHASE_RUN, .time=80 },
                    { .type=PHASE_BLOCK, .time=41 },
                    /* Run: 221 block: 163 57.5% */
                } },
            { .phase_count = 6, .list = {
                    { .type=PHASE_RUN, .time=1250 },
                    { .type=PHASE_BLOCK, .time=10 },  
                    { .type=PHASE_RUN, .time=10 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=10 },
                    { .type=PHASE_BLOCK, .time=5 },  
                } },
            { .phase_count = 6, .list = {
                    { .type=PHASE_RUN, .time=850 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=7 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=9 },
                    { .type=PHASE_BLOCK, .time=5 },  
                } },
        }
    },
    {
        .name="n2",
        .vm_count=3,
        .vm_workloads = {
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=50 },
                    { .type=PHASE_BLOCK, .time=50 },  
                    { .type=PHASE_RUN, .time=45 },
                    { .type=PHASE_BLOCK, .time=67 },  
                    { .type=PHASE_RUN, .time=55 },
                    { .type=PHASE_BLOCK, .time=82 },  
                    { .type=PHASE_RUN, .time=37 },
                    { .type=PHASE_BLOCK, .time=100 },
                    /* Run: 187 block: 299  38.4% */
                } },
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=45 },
                    { .type=PHASE_BLOCK, .time=67 },  
                    { .type=PHASE_RUN, .time=50 },
                    { .type=PHASE_BLOCK, .time=50 },  
                    { .type=PHASE_RUN, .time=55 },
                    { .type=PHASE_BLOCK, .time=82 },  
                    { .type=PHASE_RUN, .time=41 },
                    { .type=PHASE_BLOCK, .time=80 },
                    /* Run: 191 block: 279 40.6% */
                } },
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=66 },
                    { .type=PHASE_BLOCK, .time=30 },  
                    { .type=PHASE_RUN, .time=5 },
                    { .type=PHASE_BLOCK, .time=50 },  
                    { .type=PHASE_RUN, .time=70 },
                    { .type=PHASE_BLOCK, .time=42 },  
                    { .type=PHASE_RUN, .time=80 },
                    { .type=PHASE_BLOCK, .time=41 },
                    /* Run: 221 block: 163 57.5% */
                } },
        }
    },
    {
        .name="N2s2",
        .vm_count=5,
        .vm_workloads = {
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=15 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=16 },
                    { .type=PHASE_BLOCK, .time=6 },  
                    { .type=PHASE_RUN, .time=14 },
                    { .type=PHASE_BLOCK, .time=4 },  
                    { .type=PHASE_RUN, .time=87 },
                    { .type=PHASE_BLOCK, .time=30 },
                } },
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=13 },
                    { .type=PHASE_BLOCK, .time=10 },  
                    { .type=PHASE_RUN, .time=17 },
                    { .type=PHASE_BLOCK, .time=7 },  
                    { .type=PHASE_RUN, .time=15 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=30 },
                    { .type=PHASE_BLOCK, .time=2 },
                } },
            { .phase_count = 8, .list = {
                    { .type=PHASE_RUN, .time=66 },
                    { .type=PHASE_BLOCK, .time=30 },  
                    { .type=PHASE_RUN, .time=5 },
                    { .type=PHASE_BLOCK, .time=50 },  
                    { .type=PHASE_RUN, .time=70 },
                    { .type=PHASE_BLOCK, .time=42 },  
                    { .type=PHASE_RUN, .time=80 },
                    { .type=PHASE_BLOCK, .time=41 },
                    /* Run: 221 block: 163 57.5% */
                } },
            { .phase_count = 6, .list = {
                    { .type=PHASE_RUN, .time=1250 },
                    { .type=PHASE_BLOCK, .time=10 },  
                    { .type=PHASE_RUN, .time=10 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=10 },
                    { .type=PHASE_BLOCK, .time=5 },  
                } },
            { .phase_count = 6, .list = {
                    { .type=PHASE_RUN, .time=850 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=7 },
                    { .type=PHASE_BLOCK, .time=5 },  
                    { .type=PHASE_RUN, .time=9 },
                    { .type=PHASE_BLOCK, .time=5 },  
                } },
        }
    },
    {
        .name="s3",
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
    { .name=NULL }
        
};
