/* FLAC - Free Lossless Audio Codec
 * Copyright (C) 2015-2016  Xiph.Org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include "util.h"

#if defined _WIN32

#include <windows.h>

static double
counter_diff (const LARGE_INTEGER * start, const LARGE_INTEGER * end)
{
	LARGE_INTEGER diff, freq;

	QueryPerformanceFrequency(&freq);
	diff.QuadPart = end->QuadPart - start->QuadPart;

	return (double)diff.QuadPart/(double)freq.QuadPart;
}

double
benchmark_function (void (*testfunc) (void), unsigned count)
{
	LARGE_INTEGER start, end;
	unsigned k;

	QueryPerformanceCounter (&start) ;

	for (k = 0 ; k < count ; k++)
		testfunc();

	QueryPerformanceCounter (&end) ;

	return counter_diff (&start, &end) / count ;
} /* benchmark_function */

#elif defined FLAC__SYS_DARWIN

#include <mach/mach_time.h>

static double
counter_diff (const uint64_t * start, const uint64_t * end)
{
	mach_timebase_info_data_t t_info;
	mach_timebase_info(&t_info);
	uint64_t duration = *end - *start;

	return duration * ((double)t_info.numer/(double)t_info.denom);
}

double
benchmark_function (void (*testfunc) (void), unsigned count)
{
	uint64_t start, end;
	unsigned k;

	start = mach_absolute_time();

	for (k = 0 ; k < count ; k++)
		testfunc();

	end = mach_absolute_time();

	return counter_diff (&start, &end) / count ;
} /* benchmark_function */

#elif defined HAVE_CLOCK_GETTIME

#include <time.h>
#include <sys/time.h>

static double
timespec_diff (const struct timespec * start, const struct timespec * end)
{	struct timespec diff;

	if (end->tv_nsec - start->tv_nsec < 0)
	{	diff.tv_sec = end->tv_sec - start->tv_sec - 1 ;
		diff.tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec ;
		}
	else
	{	diff.tv_sec = end->tv_sec - start->tv_sec ;
		diff.tv_nsec = end->tv_nsec-start->tv_nsec ;
		} ;

	return diff.tv_sec + 1e-9 * diff.tv_nsec ;
}

double
benchmark_function (void (*testfunc) (void), unsigned count)
{	struct timespec start, end;
	unsigned k ;

	clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &start) ;

	for (k = 0 ; k < count ; k++)
		testfunc () ;

	clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &end) ;

	return timespec_diff (&start, &end) / count ;
} /* benchmark_function */

#else

#include <time.h>
#include <sys/time.h>

static double
timeval_diff (const struct timeval * start, const struct timeval * end)
{       struct timeval diff;

        if (end->tv_usec - start->tv_usec < 0)
        {       diff.tv_sec = end->tv_sec - start->tv_sec - 1 ;
                diff.tv_usec = 1000000 + end->tv_usec - start->tv_usec ;
                }
        else
        {       diff.tv_sec = end->tv_sec - start->tv_sec ;
                diff.tv_usec = end->tv_usec-start->tv_usec ;
                } ;

        return diff.tv_sec + 1e-6 * diff.tv_usec ;
}

double
benchmark_function (void (*testfunc) (void), unsigned count)
{	struct timeval start, end;
	unsigned k ;

	gettimeofday(&start, NULL);

	for (k = 0 ; k < count ; k++)
		testfunc () ;

	gettimeofday(&end, NULL);

	return timeval_diff (&start, &end) / count ;
} /* benchmark_function */

#endif

static int
double_cmp (const void * a, const void * b)
{	const double * pa = (double *) a ;
	const double * pb = (double *) b ;
	return pa [0] < pb [0] ;
} /* double_cmp */

void
benchmark_stats (bench_stats * stats)
{	double sum, times [stats->run_count] ;
	unsigned k ;

	for (k = 0 ; k < stats->run_count ; k++)
		times [k] = benchmark_function (stats->testfunc, stats->loop_count) ;

	qsort (times, stats->run_count, sizeof (times [0]), double_cmp) ;

	sum = 0.0 ;
	stats->min_time = stats->max_time = times [0] ;
	for (k = 0 ; k < stats->run_count ; k++)
	{	stats->min_time = stats->min_time < times [k] ? stats->min_time : times [k] ;
		stats->max_time = stats->max_time > times [k] ? stats->max_time : times [k] ;
		sum += times [k] ;
		}
	stats->mean_time = sum / stats->run_count ;
	if (stats->run_count & 1)
		stats->median_time = times [(stats->run_count + 1) / 2] ;
	else
		stats->median_time = 0.5 * (times [stats->run_count / 2] + times [(stats->run_count / 2) + 1]) ;

	return ;
} /* benchmark_stats */
