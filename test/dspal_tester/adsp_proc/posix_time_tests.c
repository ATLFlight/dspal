/****************************************************************************
 *   Copyright (c) 2015 James Wilson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name ATLFlight nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#include <signal.h>
#include <time.h>
#include <pthread.h>
#include "test_utils.h"
#include "dspal_tester.h"
#include "HAP_farf.h"
#include "HAP_power.h"


enum DSPAL_TESTER_THREAD_EXIT_STATUS {
	ONE_SHOT_TIMER_CB_EXIT_STATUS = 1,
	PERIODIC_TIMER_CB_EXIT_STATUS,
	PERIODIC_TIMER_SIGNAL_CB_EXIT_STATUS,
	PERIODIC_TIMER_SIGWAIT_EXIT_STATUS,
};

#define MAX_WAIT_TIME_US   125 //
#define PERIODIC_TIMER_MAX_EXPIRATION_NUM  100

static pthread_t one_shot_timer_cb_thread;
static pthread_t periodic_timer_cb_thread;
static pthread_t periodic_timer_signal_cb_thread;
static pthread_t periodic_timer_sigwait_thread;


int dspal_tester_test_clockid(void)
{
#ifndef CLOCK_REALTIME
	FAIL("CLOCK_REALTIME not defined");
#endif

#ifndef CLOCK_MONOTONIC
	FAIL("CLOCK_MONOTONIC not defined");
#endif

#if _POSIX_CPUTIME
#ifndef CLOCK_PROCESS_CPUTIME_ID
	FAIL("CLOCK_PROCESS_CPUTIME_ID not defined");
#endif
#endif

#if _POSIX_THREAD_CPUTIME
#ifndef CLOCK_THREAD_CPUTIME_ID
	FAIL("CLOCK_THREAD_CPUTIME_ID not defined");
#endif
#endif

	return TEST_PASS;
}

int dspal_tester_test_sigevent(void)
{
#ifndef SIGEV_NONE
	FAIL("SIGEV_NONE not defined");
#endif

#ifndef SIGEV_SIGNAL
	FAIL("SIGEV_SIGNAL not defined");
#endif

#ifndef SIGEV_THREAD
	FAIL("SIGEV_THREAD not defined");
#endif

	return TEST_PASS;
}

int dspal_tester_test_time(void)
{
	time_t rv;
	rv = time(NULL);

	if (rv <= 0) { FAIL("time returned error"); }

	time(&rv);

	if (rv <= 0) { FAIL("time returned error"); }

	return TEST_PASS;
}

int dspal_tester_test_timer_realtime_sig_none(void)
{
	int rv = 0;

	struct sigevent sev;
	sev.sigev_notify = SIGEV_NONE;

	timer_t timer_id = 0;

	rv = timer_create(CLOCK_REALTIME, &sev, &timer_id);

	if (rv != 0) { FAIL("timer_create returned non-zero"); }

	if (timer_id == 0) { FAIL("timer_create returned invalid timer_id"); }

	rv = timer_delete(timer_id);

	if (rv != 0) { FAIL("timer_delete returned non-zero"); }

	return TEST_PASS;
}

int dspal_tester_test_timer_monotonic_sig_none(void)
{
	int rv = 0;

	struct sigevent sev;
	sev.sigev_notify = SIGEV_NONE;

	timer_t timer_id = 0;

	rv = timer_create(CLOCK_MONOTONIC, &sev, &timer_id);

	if (rv != 0) { FAIL("timer_create returned non-zero"); }

	if (timer_id == 0) { FAIL("timer_create returned invalid timer_id"); }

	rv = timer_delete(timer_id);

	if (rv != 0) { FAIL("timer_delete returned non-zero"); }

	return TEST_PASS;
}

int dspal_tester_test_timer_process_cputime_sig_none(void)
{
#ifndef _POSIX_CPUTIME
	return TEST_SKIP;
#else

	if (_POSIX_CPUTIME < 0) { return TEST_SKIP; }

	int rv = 0;

	struct sigevent sev;
	sev.sigev_notify = SIGEV_NONE;

	timer_t timer_id = 0;

	rv = timer_create(CLOCK_PROCESS_CPUTIME_ID, &sev, &timer_id);

	if (rv != 0) { FAIL("timer_create returned non-zero"); }

	if (timer_id == 0) { FAIL("timer_create returned invalid timer_id"); }

	rv = timer_delete(timer_id);

	if (rv != 0) { FAIL("timer_delete returned non-zero"); }

#endif // _POSIX_CPUTIME

	return TEST_PASS;
}

int dspal_tester_test_timer_thread_cputime_sig_none(void)
{
#ifndef _POSIX_THREAD_CPUTIME
	return TEST_SKIP;
#else

	if (_POSIX_THREAD_CPUTIME < 0) { return TEST_SKIP; }

	int rv = 0;

	struct sigevent sev;
	sev.sigev_notify = SIGEV_NONE;

	timer_t timer_id = 0;

	rv = timer_create(CLOCK_THREAD_CPUTIME_ID, &sev, &timer_id);

	if (rv != 0) { FAIL("timer_create returned non-zero"); }

	if (timer_id == 0) { FAIL("timer_create returned invalid timer_id"); }

	rv = timer_delete(timer_id);

	if (rv != 0) { FAIL("timer_delete returned non-zero"); }

#endif // _POSIX_THREAD_CPUTIME

	return TEST_PASS;
}

int dspal_tester_test_time_return_value(void)
{
	time_t t = 0;

	t = time(NULL);

	while (t >= time(NULL));

	return TEST_PASS;
}

int dspal_tester_test_time_param(void)
{
	time_t start = 0;
	time_t now   = 0;

	time(&start);
	now = start;

	while (start >= now) {
		time(&now);
	}

	return TEST_PASS;
}
uint64_t dspal_get_time_1us()
{
  // grab device time, convert to microseconds
  struct timespec time_struct;
  clock_gettime(0,&time_struct);
  uint64_t time_1us = (time_struct.tv_nsec)/1000 + ((uint64_t) time_struct.tv_sec)*1000000;
  return time_1us;
}

int dspal_tester_test_usleep(void)
{
	const int ONE_SECOND_IN_MICROSECONDS = 1000000;

	time_t start = time(NULL);
	usleep(2 * ONE_SECOND_IN_MICROSECONDS);
	time_t finish = time(NULL);

	time_t length = finish - start;

	if (length < 1) {
		LOG_ERR("usleep run length: %d", length);
		FAIL("usleep returned too fast");
	}
	return TEST_PASS;
}

static void test_dspal_get_time_1us()
{
	uint64_t start_time = dspal_get_time_1us();
	int counter = 5000;
	for (int i = 0; i < counter; i ++) {		
		uint64_t curr_time = dspal_get_time_1us();
	}
	int elapsed_time = dspal_get_time_1us() - start_time;
	LOG_ERR("total %d dspal_get_time_1us calls takes %d, each %f", counter, elapsed_time, elapsed_time*1.0/counter);
	
}
int test_usleep_worker(int flag)
{
	int fail = 0, sleep_tolerance = 100; //us
	int delay_us = 50;
	int total_delta = 0, loops = 100, max_delta = 0;
	for (int i = 0; i < loops; i ++) {
		uint64_t start_time = dspal_get_time_1us();
		delay_us = delay_us + 40;	// 150 --> 2150
		usleep(delay_us); 
		int elapsed_time = dspal_get_time_1us()-start_time;
		int delta = (elapsed_time - delay_us >= 0)?(elapsed_time - delay_us):(delay_us - elapsed_time);
		total_delta += delta;
		if (delta > max_delta) max_delta = delta;
		if((elapsed_time >= delay_us) && (elapsed_time - delay_us > sleep_tolerance)) {
			if (flag == 0)
				{ LOG_ERR("usleep() slept too long. Des: %d. Act: %d, delta %d",delay_us,elapsed_time, delta); fail = 1;}
			else
				{ LOG_ERR("usleep() slept too long. Des: %d. Act: %d, delta %d",delay_us,elapsed_time, delta); fail = 1;}
		}
			
		if((delay_us > elapsed_time) && ( delay_us - elapsed_time > sleep_tolerance)) {
			if (flag == 0)
				{ LOG_ERR("usleep() slept too short. Des: %d. Act: %d, delta %d",delay_us,elapsed_time, delta); fail = 1;}
			else
				{ LOG_ERR("in thread: usleep() slept too short. Des: %d. Act: %d, delta %d",delay_us,elapsed_time, delta); fail = 1;}			
		}
		//usleep(50000);
	}
	
	LOG_ERR("%d, average delta is %f, max is %d", total_delta, total_delta*1.0/loops, max_delta);
	if (fail == 1) {
		LOG_ERR("flag %d", flag); FAIL("usleep fails in test worker");
	}
	else
		LOG_ERR("usleep test passed!!!, flag %d", flag)
	return fail;

}
void * test_usleep_thread_helper(void * param)
{	
	int *fail = (int *)param;
	*fail = test_usleep_worker(1);
	return NULL;
}
int tester_test_usleep_async(void)
{
	int rv = 0, fail = 0;

	int test_value = 0;
	pthread_t thread;
	pthread_attr_t thread_attr;
	if (pthread_attr_init(&thread_attr) != 0)
	{
	  	FAIL("failed to init thread attribute");
	}
	thread_attr.priority = 255 - 20;
	if (pthread_attr_setstacksize(&thread_attr, 4*1024) != 0)
	{
		FAIL("failed to set thread stack size");
	}
	pthread_attr_setthreadname(&thread_attr, "sleep_tester");

	rv = pthread_create(&thread, &thread_attr, test_usleep_thread_helper, &fail);

	if (rv != 0) { FAIL("thread_create returned error"); }

	rv = pthread_join(thread, NULL);

	if (rv != 0) { FAIL("thread_join returned error"); }
	
	if (fail != 0)
		FAIL("tester_test_usleep_async fails");

	return TEST_PASS;

}
int dspal_tester_test_usleep_ext(void)
{
	int fail = 0;
	test_dspal_get_time_1us();
	fail = test_usleep_worker(0);
	if (fail != 0)
		FAIL("usleep test fails");
	return TEST_PASS;

}
int dspal_tester_test_usleep_thread(void)
{
	int fail = 0;
	fail = tester_test_usleep_async();
	if (fail != 0)
		FAIL("usleep test in a seperate thread fails");
	return TEST_PASS;
}

int dspal_tester_test_clock_getres(void)
{
	struct timespec res;

	int rv = clock_getres(CLOCK_MONOTONIC, &res);

	if (rv != 0) { FAIL("clock_getres returned error"); }

	if (res.tv_sec == 0 && res.tv_nsec == 0) { FAIL("clock res is 0"); }

	return TEST_PASS;
}

int dspal_tester_test_clock_gettime(void)
{
	struct timespec tp;

	int rv = clock_gettime(CLOCK_MONOTONIC, &tp);

	if (rv != 0) { FAIL("clock_gettime returned error"); }

	if (tp.tv_sec == 0 && tp.tv_nsec == 0) { FAIL("time is 0"); }

	return TEST_PASS;
}

int dspal_tester_test_clock_settime(void)
{
	// if this method does not crash, test passed
	clock_settime(CLOCK_MONOTONIC, NULL);

	return TEST_PASS;
}

static struct timespec one_shot_timer_start;
static void one_shot_timer_callback(union sigval val)
{
	struct timespec now;

	clock_gettime(CLOCK_REALTIME, &now);
	#if 0
	LOG_DEBUG("one shot timer expected time: %ld us", MAX_WAIT_TIME_US);
	LOG_DEBUG("one shot timer elapsed time: %ld us",
		  (now.tv_sec - one_shot_timer_start.tv_sec) * 1000000 +
		  (now.tv_nsec - one_shot_timer_start.tv_nsec) / 1000);
	#endif

	pthread_kill(one_shot_timer_cb_thread, val.sival_int);
}

void *one_shot_timer_cb_test(void *context)
{
	const int SIG = SIGRTMIN;
	struct sigevent sev;
	timer_t timer_id = 0;
	sigset_t set;
	struct itimerspec timer_spec;
	int sig;
	int rv;

	LOG_DEBUG("one_shot_timer_cb_test thread id = %d", pthread_self());

	sigemptyset(&set);
	sigaddset(&set, SIG);

	sev.sigev_notify           = SIGEV_THREAD;
	sev.sigev_notify_function  = one_shot_timer_callback;
	sev.sigev_value.sival_int  = SIG;
	sev.sigev_notify_attributes = 0;

	if (timer_create(CLOCK_REALTIME, &sev, &timer_id) != 0) {
		LOG_ERR("timer_create failed");
		goto exit;
	}

	timer_spec.it_value.tv_sec     = 0;
	timer_spec.it_value.tv_nsec    = MAX_WAIT_TIME_US * 1000;
	timer_spec.it_interval.tv_sec  = 0;
	timer_spec.it_interval.tv_nsec = 0;

	if (timer_settime(timer_id, 0, &timer_spec, NULL) != 0) {
		LOG_ERR("timer_settime failed");
		timer_delete(timer_id);
		goto exit;
	}

	clock_gettime(CLOCK_REALTIME, &one_shot_timer_start);

	rv = sigwait(&set, &sig);

	if (rv != 0 || sig != SIGRTMIN) {
		LOG_ERR("sigwait failed");
	}

	timer_delete(timer_id);

exit:
	pthread_exit((void *)ONE_SHOT_TIMER_CB_EXIT_STATUS);
	return NULL;
}

int dspal_tester_test_one_shot_timer_cb(void)
{
	int rv = 0;
	int *exit_status;
	size_t stacksize = 2 * 1024;
	pthread_attr_t attr;

	rv = pthread_attr_init(&attr);

	if (rv != 0) { FAIL("pthread_attr_init returned error"); }

	rv = pthread_attr_setstacksize(&attr, stacksize);

	if (rv != 0) { FAIL("pthread_attr_setstacksize returned error"); }

	// create test thread.
	// posix functions must be called in a thread created with pthread
	if (pthread_create(&one_shot_timer_cb_thread, &attr, one_shot_timer_cb_test,
			   NULL) != 0) {
		FAIL("pthread_create failed");
	}

	LOG_DEBUG("created one_shot_timer_cb_test thread %d", one_shot_timer_cb_thread);

	// wait on the child thread
	rv = pthread_join(one_shot_timer_cb_thread, (void **)&exit_status);

	if (rv != 0 || exit_status != (void *)ONE_SHOT_TIMER_CB_EXIT_STATUS) {
		FAIL("pthread_join failed");
	}

	return TEST_PASS;
}


/**
 * periodic timer callback function
 */
static void periodic_timer_callback(union sigval val)
{
	static struct timespec last_time;
	struct timespec now;

	if (last_time.tv_sec == 0) {
		clock_gettime(CLOCK_REALTIME, &last_time);

	} else {
		clock_gettime(CLOCK_REALTIME, &now);
		#if 0
		LOG_DEBUG("periodic timer elapsed time: %ld us",
			  (now.tv_sec - last_time.tv_sec) * 1000000 +
			  (now.tv_nsec - last_time.tv_nsec) / 1000);
		#endif
		last_time = now;
	}

	// send signal to the caller thread
	pthread_kill(periodic_timer_cb_thread, val.sival_int);
}

/**
 * periodic_timer_cb thread routine
 */
void *periodic_timer_cb_test(void *context)
{
	const int SIG = SIGRTMIN;
	struct sigevent sev;
	timer_t timer_id = 0;
	sigset_t set;
	struct itimerspec timer_spec;
	int sig;
	int rv;

	LOG_DEBUG("periodic_timer_cb_test thread id = %d", pthread_self());

	// set signal type
	sigemptyset(&set);
	sigaddset(&set, SIG);

	// set event notification function
	sev.sigev_notify           = SIGEV_THREAD;
	sev.sigev_notify_function  = periodic_timer_callback;
	sev.sigev_value.sival_int  = SIG;
	sev.sigev_notify_attributes = 0;

	if (timer_create(CLOCK_REALTIME, &sev, &timer_id) != 0) {
		LOG_ERR("timer_create failed");
		goto exit;
	}

	timer_spec.it_value.tv_sec     = 0;
	timer_spec.it_value.tv_nsec    = MAX_WAIT_TIME_US * 1000;
	timer_spec.it_interval.tv_sec  = timer_spec.it_value.tv_sec;
	timer_spec.it_interval.tv_nsec = timer_spec.it_value.tv_nsec;

	// start the timer
	if (timer_settime(timer_id, 0, &timer_spec, NULL) != 0) {
		LOG_ERR("timer_settime failed");
		timer_delete(timer_id);
		goto exit;
	}

	for (int i = 0; i < PERIODIC_TIMER_MAX_EXPIRATION_NUM; i++) {
		// wait on the signal
		rv = sigwait(&set, &sig);

		if (rv != 0 || sig != SIGRTMIN) {
			LOG_ERR("sigwait failed");
		}
	}

	// delete the timer
	timer_delete(timer_id);

exit:
	pthread_exit((void *)PERIODIC_TIMER_CB_EXIT_STATUS);
	return NULL;
}

/**
 * @brief
 * Periodic Timer Test with SIGEV_THREAD
 * In this test, a periodic timer is set up to be triggered every
 * MAX_WAIT_TIME_US us.
 * NOTE: POSIX timer functions must be called in a thread created with pthread().
 */
int dspal_tester_test_periodic_timer_cb(void)
{
	int rv = 0;
	int *exit_status;
	size_t stacksize = 2 * 1024;
	pthread_attr_t attr;

	rv = pthread_attr_init(&attr);

	if (rv != 0) { FAIL("pthread_attr_init returned error"); }

	rv = pthread_attr_setstacksize(&attr, stacksize);

	if (rv != 0) { FAIL("pthread_attr_setstacksize returned error"); }

	// create test thread.
	// posix functions must be called in a thread created with pthread
	if (pthread_create(&periodic_timer_cb_thread, &attr, periodic_timer_cb_test,
			   NULL) != 0) {
		FAIL("pthread_create failed");
	}

	LOG_DEBUG("created periodic_timer_cb_test thread %d", periodic_timer_cb_thread);

	// wait on the child thread
	rv = pthread_join(periodic_timer_cb_thread, (void **)&exit_status);

	if (rv != 0 || exit_status != (void *)PERIODIC_TIMER_CB_EXIT_STATUS) {
		FAIL("pthread_join failed");
	}

	return TEST_PASS;
}


static int timer_signal_callback_counter = 0;
/**
 * periodic timer with SIGEV_SIGNAL callback function
 */
static void periodic_timer_signal_callback(int signum)
{
	static struct timespec last_time;
	struct timespec now;

	if (last_time.tv_sec == 0) {
		clock_gettime(CLOCK_REALTIME, &last_time);

	} else {
		clock_gettime(CLOCK_REALTIME, &now);
		LOG_DEBUG("periodic timer elapsed time: %ld us",
			  (now.tv_sec - last_time.tv_sec) * 1000000 +
			  (now.tv_nsec - last_time.tv_nsec) / 1000);
		last_time = now;
	}

	timer_signal_callback_counter++;
}

/**
 * periodic_timer_signal_cb_test thread routine
 */
void *periodic_timer_signal_cb_test(void *context)
{
	struct sigevent sev;
	timer_t timer_id = 0;
	struct itimerspec timer_spec;
	struct sigaction sa;
	sigset_t set;
	int rv;

	LOG_DEBUG("periodic_timer_signal_cb_test thread id = %d", pthread_self());

	// register signal handler for SIGRTMIN
	sa.sa_handler = periodic_timer_signal_callback;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_sigaction = NULL;

	if (sigaction(SIGRTMIN, &sa, NULL) != 0) {
		LOG_ERR("sigaction failed");
		goto exit;
	}

	// set event notification function
	sev.sigev_notify           = SIGEV_SIGNAL;
	sev.sigev_signo            = SIGRTMIN;
	sev.sigev_value.sival_int  = SIGRTMIN;
	sev.sigev_notify_function  = 0;
	sev.sigev_notify_attributes = 0;

	if (timer_create(CLOCK_REALTIME, &sev, &timer_id) != 0) {
		LOG_ERR("timer_create failed");
		goto exit;
	}

	timer_spec.it_value.tv_sec     = 0;
	timer_spec.it_value.tv_nsec    = MAX_WAIT_TIME_US * 1000;
	timer_spec.it_interval.tv_sec  = timer_spec.it_value.tv_sec;
	timer_spec.it_interval.tv_nsec = MAX_WAIT_TIME_US * 1000;

	// start the timer
	if (timer_settime(timer_id, 0, &timer_spec, NULL) != 0) {
		LOG_ERR("timer_settime failed");
		timer_delete(timer_id);
		goto exit;
	}

	// set SIGRTMIN as the signal type to wait on
	sigemptyset(&set);
	sigaddset(&set, SIGRTMIN);

	// Run until the timer callback handler is invoked
	// PERIODIC_TIMER_MAX_EXPIRATION_NUM times
	while (timer_signal_callback_counter < PERIODIC_TIMER_MAX_EXPIRATION_NUM) {
		rv = sigsuspend(&set);
	}

	// delete the timer
	timer_delete(timer_id);

exit:
	pthread_exit((void *)PERIODIC_TIMER_SIGNAL_CB_EXIT_STATUS);
	return NULL;
}

/**
 * @brief
 * Periodic Timer Test with SIGEV_SIGNAL
 * In this test, a periodic timer is set up to be triggered every
 * MAX_WAIT_TIME_US us. When timer expires, the specified signal is generated,
 * and the registered signal action handler is invoked.
 * NOTE: POSIX timer functions must be called in a thread created with pthread().
 */
int dspal_tester_test_periodic_timer_signal_cb(void)
{
	// TODO: Possible issue with pthread_join().  Must be investigated.
	int rv = 0;
	// int *exit_status;
	size_t stacksize = 2 * 1024;
	pthread_attr_t attr;

	rv = pthread_attr_init(&attr);

	if (rv != 0) { FAIL("pthread_attr_init returned error"); }

	rv = pthread_attr_setstacksize(&attr, stacksize);

	if (rv != 0) { FAIL("pthread_attr_setstacksize returned error"); }

	// create test thread.
	// posix functions must be called in a thread created with pthread
	if (pthread_create(&periodic_timer_signal_cb_thread, &attr, 
			   periodic_timer_signal_cb_test,
			   NULL) != 0) {
		FAIL("pthread_create failed");
	}

	LOG_DEBUG("created periodic_timer_signal_cb_test thread %d", periodic_timer_signal_cb_thread);

	// wait on the child thread
	// TODO: Possible issue with pthread_join().  Must be investigated.
//   rv = pthread_join(periodic_timer_signal_cb_thread, (void **)&exit_status);
//   if (rv != 0 || exit_status != (void *)PERIODIC_TIMER_SIGNAL_CB_EXIT_STATUS)
//   {
//      FAIL("pthread_join failed");
//   }

	return TEST_PASS;
}


/**
 * periodic_timer_sigwait_test thread routine
 */
void *periodic_timer_sigwait_test(void *context)
{
	struct sigevent sev;
	timer_t timer_id = 0;
	struct itimerspec timer_spec;
	sigset_t set;
	int rv;
	int sig;

	LOG_DEBUG("periodic_timer_sigwait_test thread id = %d", pthread_self());

	// set event notification function
	sev.sigev_notify           = SIGEV_SIGNAL;
	sev.sigev_signo            = SIGRTMIN;
	sev.sigev_value.sival_int  = SIGRTMIN;
	sev.sigev_notify_function  = 0;
	sev.sigev_notify_attributes = 0;

	if (timer_create(CLOCK_REALTIME, &sev, &timer_id) != 0) {
		LOG_ERR("timer_create failed");
		goto exit;
	}

	timer_spec.it_value.tv_sec     = 0;
	timer_spec.it_value.tv_nsec    = MAX_WAIT_TIME_US * 1000;
	timer_spec.it_interval.tv_sec  = timer_spec.it_value.tv_sec;
	timer_spec.it_interval.tv_nsec = MAX_WAIT_TIME_US * 1000;

	// start the timer
	if (timer_settime(timer_id, 0, &timer_spec, NULL) != 0) {
		LOG_ERR("timer_settime failed");
		timer_delete(timer_id);
		goto exit;
	}

	// set SIGRTMIN as the signal type to wait on
	sigemptyset(&set);
	sigaddset(&set, SIGRTMIN);

	static struct timespec last_time;
	struct timespec now;

	// Run until the timer expires for PERIODIC_TIMER_MAX_EXPIRATION_NUM times
	for (int i = 0; i < PERIODIC_TIMER_MAX_EXPIRATION_NUM; i++) {

		// wait on the signal
		rv = sigwait(&set, &sig);

		// check the received signal
		if (rv != 0 || sig != SIGRTMIN) {
			LOG_ERR("sigwait failed rv %d sig %d", rv, sig);
			continue;
		}

		if (last_time.tv_sec == 0) {
			clock_gettime(CLOCK_REALTIME, &last_time);

		} else {
			clock_gettime(CLOCK_REALTIME, &now);
			#if 0
			LOG_DEBUG("periodic timer elapsed time: %ld us",
				  (now.tv_sec - last_time.tv_sec) * 1000000 +
				  (now.tv_nsec - last_time.tv_nsec) / 1000);
			#endif
			last_time = now;
		}
	}

	// delete the timer
	timer_delete(timer_id);

exit:
	pthread_exit((void *)PERIODIC_TIMER_SIGWAIT_EXIT_STATUS);
	return NULL;
}

/**
 * @brief
 * Periodic Timer Test with SIGEV_SIGNAL
 * In this test, a periodic timer is set up to be triggered every
 * MAX_WAIT_TIME_US us. The caller thread blocks on sigwait() and returns
 * When timer expires. In this test, no signal action handler is registered.
 * NOTE: POSIX timer functions must be called in a thread created with pthread().
 */
int dspal_tester_test_periodic_timer_sigwait(void)
{
	int rv = 0;
	int *exit_status;
	size_t stacksize = 2 * 1024;
	pthread_attr_t attr;

	rv = pthread_attr_init(&attr);

	if (rv != 0) { FAIL("pthread_attr_init returned error"); }

	rv = pthread_attr_setstacksize(&attr, stacksize);

	if (rv != 0) { FAIL("pthread_attr_setstacksize returned error"); }

	// create test thread.
	// posix functions must be called in a thread created with pthread
	if (pthread_create(&periodic_timer_sigwait_thread, &attr, 
			   periodic_timer_sigwait_test,
			   NULL) != 0) {
		FAIL("pthread_create failed");
	}

	LOG_DEBUG("created periodic_timer_sigwait_test thread %d", periodic_timer_sigwait_thread);

	// wait on the child thread
	rv = pthread_join(periodic_timer_sigwait_thread, (void **)&exit_status);

	if (rv != 0 || exit_status != (void *)PERIODIC_TIMER_SIGWAIT_EXIT_STATUS) {
		FAIL("pthread_join failed");
	}

	return TEST_PASS;
}
