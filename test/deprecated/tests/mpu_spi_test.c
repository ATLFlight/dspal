/*==============================================================================
 Copyright (c) 2015 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Proprietary and Confidential.
 ==============================================================================*/

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "mpu_tester.h"
#include "mpu_spi.h"
#include "mpu_test_common.h"
#include "test_utils.h"
#include "HAP_power.h"

#define MEASUREMENT_MAX_NUM   100000

static uint64_t sample_interval_us[MEASUREMENT_MAX_NUM];
static uint64_t sample_cnt;

int mpu_tester_enable_power_boost()
{
   /*---------------------------------------------------------------------
    * The following codes enable the power boost mode.
    * Power boost mode is required to do 8KHz IMU data polling. With power
    * boost mode, we can limit SPI transfer time to around 80-90us. Without
    * power boost mode, you must use FIFO mode to achieve 8KHz IMU sampling.
    *
    * The following sets clock speed to 70% of maximum and bus speed to 100%.
    -----------------------------------------------------------------------*/
   int clock, bus, latency;
   clock = 70;
   bus = 100;
   latency = 0xFFFFFFFF;
   HAP_power_request(clock, bus, latency);
   FARF(ALWAYS, "clock = %d bus = %d latency = %d", clock, bus, latency);

   return 0;
}

/**
 * @brief Test to check if mpu spi device can be opened
 *
 * @par
 * Test
 * 1) Open mpu spi device path ('/dev/spi-1')
 * 2) Close mpu spi device path
 * 3) Try to open invalid mpu spi device path and make sure it fails
 * 4) If step 3 did not fail, close the device that did open
 *
 * @return
 * TEST_PASS ------ Test Passes
 * TEST_FAIL ------ Test Failed
*/
int mpu_tester_test_mpu_spi_open()
{
   int test_result = TEST_PASS;

   FARF(ALWAYS, "**** mpu_tester_test_mpu_spi_open ****");

   if (mpu_spi_open(SPI_DEV_PATH) != 0)
   {
      FARF(ALWAYS, "error opening %s", SPI_DEV_PATH);
      test_result = TEST_FAIL;
   }
   else
   {
      mpu_spi_close();
   }

   if (mpu_spi_open("invalid_path") == 0)
   {
      FARF(ALWAYS, "invalid path is opened!");
      test_result = TEST_FAIL;
      mpu_spi_close();
   }

   FARF(ALWAYS, "**** mpu_tester_test_mpu_spi_open: %s ****",
        test_result == TEST_PASS ? "PASSED" : "FAILED");

   return test_result;
}

/**
 * @brief Test to check if mpu spi device can be opened and close correctly
 *
 * @par
 * Test
 * 1) Open mpu spi device path ('/dev/spi-1')
 * 2) Close mpu spi device path
 *
 * @return
 * TEST_PASS ------ Test Passes
 * TEST_FAIL ------ Test Failed
*/
int mpu_tester_test_mpu_spi_close()
{
   int test_result = TEST_PASS;
   int result;

   FARF(ALWAYS, "**** mpu_tester_test_mpu_spi_close ****");

   if (mpu_spi_open(SPI_DEV_PATH) != 0)
   {
      FARF(ALWAYS, "error opening %s", SPI_DEV_PATH);
      test_result = TEST_FAIL;
   }
   else if ((result = mpu_spi_close()) != 0)
   {
      FARF(ALWAYS, "error mpu_spi_close(): result %d", result);
      test_result = TEST_FAIL;
   }

   FARF(ALWAYS, "**** mpu_tester_test_mpu_spi_close: %s ****",
        test_result == TEST_PASS ? "PASSED" : "FAILED");

   return test_result;

}

/**
 * @brief In this test, we do SPI bulk data transfer in a loop to profile the
 * bulk data transfer performance.
 *
 * @par
 * Test
 * 1) Open mpu spi device path ('/dev/spi-1')
 * 2) Get timestamp
 * 3) Do mpu spi bulk read
 * 4) Get timestamp
 * 5) Calculate delta time
 * 6) Repeat steps 2-5 a couple of times
 * 7) Close the mpu spi device.
 *
 * @return
 * TEST_PASS ------ Test Passes
 * TEST_FAIL ------ Test Failed
*/
int mpu_tester_test_bulk_transfer(void)
{
   int test_result = TEST_PASS;
   int result;
   struct timespec ts_start, ts_end;
   uint64_t elapsed_time_us;

   FARF(ALWAYS, "**** mpu_tester_test_bulk_transfer ****");

   if (mpu_spi_open(SPI_DEV_PATH) != 0)
   {
      FARF(ALWAYS, "error opening %s", SPI_DEV_PATH);
      test_result = TEST_FAIL;
   }

   sample_cnt = 0;

   for (int i = 0; i < MEASUREMENT_MAX_NUM; i++)
   {
      clock_gettime(CLOCK_REALTIME, &ts_start);

      uint8_t buffer[24];
      if (mpu_spi_bulk_read(0x3a, sizeof(buffer), buffer) != 0)
      {
         FARF(ALWAYS, "failed to mpu_spi_bulk_read");
      }

      clock_gettime(CLOCK_REALTIME, &ts_end);

      elapsed_time_us = (ts_end.tv_sec - ts_start.tv_sec) * 1000000 +
            (ts_end.tv_nsec - ts_start.tv_nsec) / 1000;

      sample_interval_us[sample_cnt] = elapsed_time_us;
      sample_cnt++;
   }

   if ((result = mpu_spi_close()) != 0)
   {
      FARF(ALWAYS, "error mpu_spi_close(): result %d", result);
      test_result = TEST_FAIL;
   }

   float avg_sample_interval_us, stdev_sample_interval_us;
   uint64_t min_sample_interval_us, max_sample_interval_us;
   calculate_stats(sample_interval_us, sample_cnt, &avg_sample_interval_us,
                   &stdev_sample_interval_us, &min_sample_interval_us,
                   &max_sample_interval_us);
   FARF(ALWAYS, "mpu_spi_bulk_read 24 bytes at 10MHz time cost(us): ");
   FARF(ALWAYS, "samples %llu avg %d stdev %d min %llu max %llu",
        sample_cnt,
        (int)(avg_sample_interval_us*1000.0),
        (int)(stdev_sample_interval_us*1000.0),
        min_sample_interval_us, max_sample_interval_us);

   FARF(ALWAYS, "**** mpu_tester_test_bulk_transfer: %s ****",
        test_result == TEST_PASS ? "PASSED" : "FAILED");

   return test_result;
}

#define MPU_SPI_TIMED_BULK_TRANSFER_EXIT_STATUS 1
#define MAX_WAIT_TIME_US  125  // 8KHz
pthread_t mpu_spi_timed_bulk_transfer_task_thread;

/**
 * periodic_timer_signal_cb_test thread routine
 */
void *mpu_spi_timed_bulk_transfer_task(void *context)
{
   struct sigevent sev;
   timer_t timer_id = 0;
   struct itimerspec timer_spec;
   sigset_t set;
   int rv;
   int sig;
   uint8_t buffer[24];

   FARF(ALWAYS,"mpu_spi_timed_bulk_transfer_task thread id = %d", pthread_self());

   // open SPI device
   if (mpu_spi_open(SPI_DEV_PATH) != 0)
   {
      FARF(ALWAYS, "error opening %s", SPI_DEV_PATH);
      goto exit;
   }

   // set event notification function
   sev.sigev_notify           = SIGEV_SIGNAL;
   sev.sigev_signo            = SIGRTMIN;
   sev.sigev_value.sival_int  = SIGRTMIN;
   sev.sigev_notify_function  = 0;
   sev.sigev_notify_attributes = 0;

   if (timer_create(CLOCK_REALTIME, &sev, &timer_id) != 0)
   {
      FARF(ALWAYS,"timer_create failed");
      goto exit;
   }

   timer_spec.it_value.tv_sec     = 0;
   timer_spec.it_value.tv_nsec    = MAX_WAIT_TIME_US * 1000;
   timer_spec.it_interval.tv_sec  = timer_spec.it_value.tv_sec;
   timer_spec.it_interval.tv_nsec = MAX_WAIT_TIME_US * 1000;

   // start the timer
   if (timer_settime(timer_id, 0, &timer_spec, NULL) != 0)
   {
      FARF(ALWAYS,"timer_settime failed");
      timer_delete(timer_id);
      goto exit;
   }

   // set SIGRTMIN as the signal type to wait on
   sigemptyset(&set);
   sigaddset(&set, SIGRTMIN);

   uint64_t elapsed_time_us;
   struct timespec ts_last, ts_now;

   sample_cnt = 0;
   // Run until the timer expires for PERIODIC_TIMER_MAX_EXPIRATION_NUM times
   for (int i = 0; i < MEASUREMENT_MAX_NUM; i++)
   {

      // wait on the signal
      rv = sigwait(&set, &sig);

      // check the received signal
      if (rv != 0 || sig != SIGRTMIN)
      {
         FARF(ALWAYS, "sigwait failed rv %d sig %d", rv, sig);
         continue;
      }

      if (mpu_spi_bulk_read(0x3a, sizeof(buffer), buffer) != 0)
      {
         FARF(ALWAYS,"failed to mpu_spi_bulk_read");
         goto exit;
      }

      if (i == 0)
      {
         clock_gettime(CLOCK_REALTIME, &ts_last);
      }
      else
      {
         clock_gettime(CLOCK_REALTIME, &ts_now);

         elapsed_time_us = (ts_now.tv_sec - ts_last.tv_sec) * 1000000 +
               (ts_now.tv_nsec - ts_last.tv_nsec) / 1000;

         sample_interval_us[sample_cnt] = elapsed_time_us;
         sample_cnt++;

         ts_last = ts_now;
      }

   }

   // delete the timer
   timer_delete(timer_id);

   float avg_sample_interval_us, stdev_sample_interval_us;
   uint64_t min_sample_interval_us, max_sample_interval_us;
   calculate_stats(sample_interval_us, sample_cnt, &avg_sample_interval_us,
                   &stdev_sample_interval_us, &min_sample_interval_us,
                   &max_sample_interval_us);
   FARF(ALWAYS, "mpu_spi_bulk_read 24bytes at 10MHz. Elapsed time between interrupts(us): ");
   FARF(ALWAYS, "samples %llu avg %d stdev %d min %llu max %llu",
        sample_cnt,
        (int)(avg_sample_interval_us*1000.0),
        (int)(stdev_sample_interval_us*1000.0),
        min_sample_interval_us, max_sample_interval_us);

exit:
   FARF(ALWAYS, "exiting bulk transfer task");

   mpu_spi_close();

   pthread_exit((void *)MPU_SPI_TIMED_BULK_TRANSFER_EXIT_STATUS);
   return NULL;
}

/**
 * @brief In this test, we do SPI bulk data transfer in a loop to profile the
 * bulk data transfer performance.
 *
 * @par
 * This test is done by creating a separate thread to run the test.
 *
 * @par
 * Test
 * 1) Open mpu spi device path ('/dev/spi-1')
 * 2) Create a timer and set it
 * 3) Setup signals to wait on
 * 4) Wait on a signal
 * 5) Do a bulk mpu spi read
 * 6) Calculate elapsed time
 * 7) Loop steps 4-6 for PERIODIC_TIMER_MAX_EXPIRATION_NUM
 * 8) Delete the timer
 * 9) Close the mpu spi device
 *
 * @return
 * TEST_FAIL ---- When test fails (cannot create a thread)
 * TEST_PASS ---- Test passed
 */
int mpu_tester_test_timed_bulk_transfer(void)
{
   int rv = 0;
   int *exit_status;
   int test_result = TEST_PASS;

   FARF(ALWAYS, "**** mpu_tester_test_timed_bulk_transfer ****");

   // NOTE: Default qurt posix thread stack size is 1KB. We must increase
   // the stack size to accommodate the mpu_spi_timed_bulk_transfer_task_thread.
   // Otherwise, stack corruption occurs and cause adsp to crash
   pthread_attr_t attr;
   size_t stacksize = -1;
   pthread_attr_init(&attr);
   pthread_attr_getstacksize(&attr, &stacksize);
   FARF(ALWAYS, "stack size: %d", stacksize);
   stacksize = 8 * 1024;

   FARF(ALWAYS, "setting the thread stack size to[%d]", stacksize);
   pthread_attr_setstacksize(&attr, stacksize);

   // create test thread.
   // posix functions must be called in a thread created with pthread
   if (pthread_create(&mpu_spi_timed_bulk_transfer_task_thread, &attr,
                     mpu_spi_timed_bulk_transfer_task,
                     NULL) != 0)
   {
      FARF(ALWAYS, "pthread_create failed");
      test_result = TEST_FAIL;
      goto exit;
   }
   FARF(ALWAYS,"created mpu_spi_timed_bulk_transfer_task thread %d",
       mpu_spi_timed_bulk_transfer_task_thread);

   // wait on the child thread
   rv = pthread_join(mpu_spi_timed_bulk_transfer_task_thread,
                     (void **)&exit_status);
   if (rv != 0 || exit_status != (void *)MPU_SPI_TIMED_BULK_TRANSFER_EXIT_STATUS)
   {
      FARF(ALWAYS, "pthread_join failed");
      test_result = TEST_FAIL;
   }

exit:
   FARF(ALWAYS, "**** mpu_tester_test_timed_bulk_transfer: %s ****",
        test_result == TEST_PASS ? "PASSED" : "FAILED");

   return test_result;
}