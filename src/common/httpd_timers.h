// This file is part of the CivetWeb web server.
// See https://github.com/civetweb/civetweb/
// (C) 2014-2017 by the CivetWeb authors, MIT license.

#ifndef INCLUDE_HTTDP_TIMERS___H
#define INCLUDE_HTTDP_TIMERS___H

#include "httpd.h"

#if !defined(MAX_TIMERS)
#define MAX_TIMERS MAX_WORKER_THREADS
#endif

typedef int (*taction)(void *arg);

struct ttimer
{
    double time;
    double period;
    taction action;
    void *arg;
};

struct ttimers
{
    pthread_t threadid;                 //  Timer thread ID
    pthread_mutex_t mutex;              // Protects timer lists
    struct ttimer timers[MAX_TIMERS];   // List of timers
    unsigned timer_count;               // Current size of timer list
};


/**
 * Get the current time from the timer
 *
 * @return Current timer value.
 */

double web_timer_getcurrenttime( void );


/**
 * @brief Add a new timer
 * @param ctx Web context
 * @param next_time
 * @param period
 * @param is_relative
 * @param action    Action to perform
 * @param arg   Argument to action
 *
 */

int web_timer_add( struct web_context *ctx,
                    double next_time,
                    double period,
                    int is_relative,
                    taction action,
                    void *arg );


/**
 * @brief Init the timer subsystem.
 * @param ctx A web context
 * @return
 */

int web_timers_init( struct web_context *ctx );


/**
 * @brief Terminate a timer.
 * @param ctx Web context
 */

void web_timers_exit( struct web_context *ctx );


#endif