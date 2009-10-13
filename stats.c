#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "stats.h"

#define DEFAULT_SAMPLE_SIZE 10240

static struct {
    int sample_size;
} opt = {
    .sample_size = DEFAULT_SAMPLE_SIZE,
};

void set_sampel_size(int n)
{
    opt.sample_size = n;
}

extern FILE *warn;

/* With compliments to "Numerical Recipes in C", which provided the algorithm
 * and basic template for this function. */
#if 0
static long long percentile(long long * A, int N, int ple) {
    int I, J, L, R, K;

    long long X, W;

    /* No samples! */
    if ( N == 0 )
        return 0;

    /* Find K, the element # we want */
    K=N*ple/100;

    /* Set the left and right boundaries of the current search space */
    L=0; R=N-1;

    while(L < R) {
        /* X: The value to order everything higher / lower than */
        X=A[K];

        /* Starting at the left and the right... */
        I=L;
        J=R;

        do {
            /* Find the first element on the left that is out-of-order w/ X */
            while(A[I]<X)
                I++;
            /* Find the first element on the right that is out-of-order w/ X */
            while(X<A[J])
                J--;

            /* If we found something out-of-order */
            if(I<=J) {
                /* Switch the values */
                W=A[I];
                A[I]=A[J];
                A[J]=W;

                /* And move on */
                I++; J--;
            }
        } while (I <= J); /* Keep going until our pointers meet or pass */
    
        /* Re-adjust L and R, based on which element we're looking for */
        if(J<K)
            L=I;
        if(K<I)
            R=J;
    }

    return A[K];
}

static float weighted_percentile(float * A, /* values */
                                       unsigned long long * w, /* weights */
                                       int N,                  /* total */
                                       int ple)                /* percentile */
{
    int L, R, I, J, K;
    unsigned long long L_weight, R_weight, I_weight, J_weight,
        K_weight, N_weight;

    float X, t1;
    unsigned long long t2;

    int progress;

    /* Calculate total weight */
    N_weight=0;

    for(I=0; I<N; I++) {
        assert(w[I]!=0);
        N_weight += w[I];
    }

    /* Find K_weight, the target weight we want */
    K_weight = N_weight * ple / 100;

    /* Set the left and right boundaries of the current search space */
    L=0;
    L_weight = 0;
    R=N-1;
    R_weight = N_weight - w[R];

    /* Search between L and R, narrowing down until we're done */
    while(L < R) {
        /* Chose an ordering value from right in the middle */
        K = (L + R) >> 1;
        /* X: The value to order everything higher / lower than */
        X=A[K];

        /* Starting at the left and the right... */
        I=L; I_weight = L_weight;
        J=R; J_weight = R_weight;

        do {
            /* Find the first element on the left that is out-of-order w/ X */
            while(A[I]<X) {
                I_weight += w[I];
                I++;
            }
            /* Find the first element on the right that is out-of-order w/ X */
            while(X<A[J]) {
                J_weight -= w[J];
                J--;
            }

            /* If we actually found something... */
            if(I<=J) {
                /* Switch the values */
                t1=A[I];
                A[I]=A[J];
                A[J]=t1;

                t2=w[I];
                w[I]=w[J];
                w[J]=t2;

                /* And move in */
                I_weight += w[I];
                I++;

                J_weight -= w[J];
                J--;
            }
        } while (I <= J); /* Keep going until our pointers meet or pass */

        progress = 0;
    
        /* Re-adjust L and R, based on which element we're looking for */
        if(J_weight<K_weight) {
            progress = 1;
            L=I; L_weight = I_weight;
        }
        if(K_weight<I_weight) {
            progress = 1;
            R=J; R_weight = J_weight;
        }
    }

    return A[L];
}
#endif

static long long self_weighted_percentile(long long * A,
                                   int N,            /* total */
                                   int ple)          /* percentile */
{
    int L, R, I, J, K;
    long long L_weight, R_weight, I_weight, J_weight,
        K_weight, N_weight;

    long long X, t1;

    int progress;

    /* Calculate total weight */
    N_weight=0;

    for(I=0; I<N; I++) {
        if(A[I] < 0)
            fprintf(warn, "%s: Value %lld less than zero!\n",
                    __func__, A[I]);
        assert(A[I]!=0);
        N_weight += A[I];
    }

    /* Find K_weight, the target weight we want */
    K_weight = N_weight * ple / 100;

    /* Set the left and right boundaries of the current search space */
    L=0;
    L_weight = 0;
    R=N-1;
    R_weight = N_weight - A[R];

    /* Search between L and R, narrowing down until we're done */
    while(L < R) {
        /* Chose an ordering value from right in the middle */
        K = (L + R) >> 1;
        /* X: The value to order everything higher / lower than */
        X=A[K];

        /* Starting at the left and the right... */
        I=L; I_weight = L_weight;
        J=R; J_weight = R_weight;

        do {
            /* Find the first element on the left that is out-of-order w/ X */
            while(A[I]<X) {
                I_weight += A[I];
                I++;
            }
            /* Find the first element on the right that is out-of-order w/ X */
            while(X<A[J]) {
                J_weight -= A[J];
                J--;
            }

            /* If we actually found something... */
            if(I<=J) {
                /* Switch the values */
                t1=A[I];
                A[I]=A[J];
                A[J]=t1;

                /* And move in */
                I_weight += A[I];
                I++;

                J_weight -= A[J];
                J--;
            }
        } while (I <= J); /* Keep going until our pointers meet or pass */

        progress = 0;
    
        /* Re-adjust L and R, based on which element we're looking for */
        if(J_weight<K_weight) {
            progress = 1;
            L=I; L_weight = I_weight;
        }
        if(K_weight<I_weight) {
            progress = 1;
            R=J; R_weight = J_weight;
        }
    }

    return A[L];
}

void update_cycles(struct cycle_summary *s, long long c) {
/* We don't know ahead of time how many samples there are, and working
 * with dynamic stuff is a pain, and unnecessary.  This algorithm will
 * generate a sample set that approximates an even sample.  We can
 * then take the percentiles on this, and get an approximate value. */
    int lap, index;

    if ( c == 0 )
        return;

    if ( opt.sample_size ) {
        lap = (s->count/opt.sample_size)+1;
        index =s->count % opt.sample_size;

        if((index - (lap/3))%lap == 0) {
            if(!s->sample) {
                s->sample = malloc(sizeof(*s->sample) * opt.sample_size);
                if(!s->sample) {
                    fprintf(stderr, "%s: malloc failed!\n", __func__);
                    exit(1);
                }
            }
            s->sample[index] = c;
        }
    }

    if(c > 0) {
        s->cycles += c;
    } else {
        s->cycles += -c;
    }
    s->count++;
}

void print_cycle_summary(struct cycle_summary *s,
                         tsc_t total, char *p) {
    if(s->count) {
        long long avg;
        double percent;

        avg = s->cycles / s->count;

        if ( total )
            percent = ((double)(s->cycles * 100)) / total;

        if ( opt.sample_size ) {
            long long p5, p50, p95;
            int data_size = s->count;

            if(data_size > opt.sample_size)
                data_size = opt.sample_size;

            p50 = self_weighted_percentile(s->sample, data_size, 50);
            p5 = self_weighted_percentile(s->sample, data_size, 5);
            p95 = self_weighted_percentile(s->sample, data_size, 95);

            if ( total )
                printf("%s: %7d %llu %5.2lf%% %6lld {%6lld|%6lld|%6lld}\n",
                       p, s->count,
                       s->cycles,
                       percent,
                       avg, p5, p50, p95);
            else
                printf("%s: %7d %llu %6lld {%6lld|%6lld|%6lld}\n",
                       p, s->count,
                       s->cycles,
                       avg, p5, p50, p95);
                
        } else {
            if ( total )
                printf("%s: %7d %llu %5.2lf%% %6lld\n",
                       p, s->count, 
                       s->cycles,
                       percent,
                       avg);
            else
                printf("%s: %7d %llu %6lld\n",
                       p, s->count, 
                       s->cycles,
                       avg);
        }
    }
}

