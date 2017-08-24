#ifndef __GT_INCLUDE_H
#define __GT_INCLUDE_H

#include "gt_signal.h"
#include "gt_spinlock.h"
#include "gt_tailq.h"
#include "gt_bitops.h"

#include "gt_uthread.h"
#include "gt_pq.h"
#include "gt_kthread.h"
#define debug 0
#define NUM_THREADS_MAX (128)

#define credit_val (10)

struct thread_mata{
    unsigned int long cpu_time;
};

struct thread_mata thread_cpu_time[NUM_THREADS_MAX];


struct thread_exe{
    unsigned int long exe_time;
};

struct thread_exe thread_exec_time[NUM_THREADS_MAX];

#endif
