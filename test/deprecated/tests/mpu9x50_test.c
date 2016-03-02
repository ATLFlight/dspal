/*==============================================================================
 Copyright (c) 2015 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Proprietary and Confidential.
 ==============================================================================*/

#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "mpu_tester.h"
#include "mpu9x50.h"
#include "mpu_test_common.h"
#include "test_utils.h"

#define abs(x)  (((x) < 0) ? (-(x)) : (x))
#define MEASUREMENT_OUTLIER_THRESHOLD  5
#define MEASUREMENT_MAX_NUM   10000
#define MEASUREMENT_THREAD_EXIT_STATUS 1
#define FIFO_POLL_THREAD_EXIT_STATUS   3

bool data_ready_isr_called;
pthread_t measurement_thread;
pthread_t mpu9x50_poll_data_thread;
pthread_t mpu9x50_fifo_poll_data_thread;
bool exit_mreasurement;
uint64_t sample_interval_us[MEASUREMENT_MAX_NUM];
uint64_t io_time_us[MEASUREMENT_MAX_NUM];
uint64_t thread_wakeup_time_us[MEASUREMENT_MAX_NUM];
uint64_t ts_int;
struct mpu9x50_data imu_samples[MEASUREMENT_MAX_NUM];


// actual sample frequency within +/- 5% is considered normal
#define SAMPLE_FREQ_ERROR_THRESHOLD   0.05

/**
 * @brief Test to check if mpu9x50 device initializes
 *
 * @par
 * Test
 * 1) Try to initialize mpu9x50 device
 * 2) close mpu9x50 device if step 1 was successful
 *
 * @return
 * TEST_PASS ------ Test Passes
 * TEST_FAIL ------ Test Failed
*/
int mpu_tester_test_mpu9x50_initialize()
{
   int ret = TEST_PASS;

   FARF(ALWAYS, "**** mpu_tester_test_mpu9x50_initialize ****");

   struct mpu9x50_config config = {
      .gyro_lpf = MPU9X50_GYRO_LPF_20HZ,
      .acc_lpf  = MPU9X50_ACC_LPF_20HZ,
      .gyro_fsr = MPU9X50_GYRO_FSR_500DPS,
      .acc_fsr  = MPU9X50_ACC_FSR_4G,
      .gyro_sample_rate = MPU9x50_SAMPLE_RATE_200HZ,
      .compass_enabled = true,
      .compass_sample_rate = MPU9x50_COMPASS_SAMPLE_RATE_100HZ,
      .fifo_enabled = false,
      .spi_dev_path = SPI_DEV_PATH,
   };

   if (mpu9x50_initialize(&config) != 0)
      ret = TEST_FAIL;

   mpu9x50_close();

   FARF(ALWAYS, "**** mpu_tester_test_mpu9x50_initialize: %s ****",
        ret == TEST_PASS ? "PASSED" : "FAILED");

   return ret;
}

/**
 * @brief
 * Test to initialize and close mpu9x50 device for 100 times. This test validates
 * if mpu9x50 driver can successfully initialize device for multiple times.
 *
 * @par
 * Test
 * 1) Try to initialize mpu9x50 device
 * 2) close mpu9x50 device if step 1 was successful.
 *    Otherwise stop the test.
 * 3) sleep for 100ms and return to step 1 for 100 runs
 *
 * @return
 * TEST_PASS ------ Test Passes
 * TEST_FAIL ------ Test Failed
*/
int mpu_tester_test_mpu9x50_initialize_multi()
{
   int i;
   int succ_runs = 0, fail_runs = 0;
   int total_runs = 100;
   int ret;

   FARF(ALWAYS, "**** mpu_tester_test_mpu9x50_initialize_multi ****");

   for (i = 0; i < total_runs; i++)
   {
      ret = mpu_tester_test_mpu9x50_initialize();

      fail_runs += (ret == TEST_FAIL);
      succ_runs += (ret == TEST_PASS);

      FARF(ALWAYS, "mpu_tester_test_mpu9x50_initialize_multi test[%d]: %s",
           i, ret == TEST_PASS ? "PASS":"FAIL");

      // sleep 100ms between each intialization test
      usleep(1000000);
   }

   FARF(ALWAYS, "mpu_tester_test_mpu9x50_initialize_multi test: "
        "total runs %d succ %d fail %d", total_runs, succ_runs, fail_runs);

   ret = (succ_runs == total_runs ? TEST_PASS : TEST_FAIL);

   FARF(ALWAYS, "**** mpu_tester_test_mpu9x50_initialize_multi: %s ****",
        ret == TEST_PASS ? "PASSED" : "FAILED");

   return ret;
}

void * data_ready_isr(void* context)
{
   struct timespec ts;

   if (exit_mreasurement)
      return NULL;

   clock_gettime(CLOCK_REALTIME, &ts);
   ts_int = ts.tv_sec * 1000000 + ts.tv_nsec/1000;

   data_ready_isr_called = true;

   // send signal to measurement thread
   pthread_kill(measurement_thread, SIGRTMIN);

   return NULL;
};

static int mpu9x50_data_ready_interrupt_test_result;

void *measurement_thread_main(void *context)
{
   sigset_t set;
   int sig = 0;
   int rv;

   enum gyro_sample_rate_e gyro_sample_rate;
   enum compass_sample_rate_e compass_sample_rate;
   int gyro_sample_rate_hz;
   int compass_sample_rate_hz;

   uint64_t total_gyro_samples = 0;
   uint64_t total_mag_samples = 0;
   uint64_t total_outlier_samples = 0;
   struct mpu9x50_data *tmp;
   struct mpu9x50_data data;
   uint64_t i;
   struct timespec ts_cycle_start, ts_cycle_end;
   struct timespec ts_io_start, ts_io_end;

   // initialize test result
   mpu9x50_data_ready_interrupt_test_result = TEST_FAIL;

   /*--------------------------------------------------------
    *                 Test Settings
    * Tune the following two parameters to set different sample
    * rate
    ---------------------------------------------------------*/
   gyro_sample_rate = MPU9x50_SAMPLE_RATE_1000HZ;
   compass_sample_rate = MPU9x50_COMPASS_SAMPLE_RATE_100HZ;
   gyro_sample_rate_hz = gyro_sample_rate_enum_to_hz(gyro_sample_rate);
   compass_sample_rate_hz = compass_sample_rate_enum_to_hz(compass_sample_rate);

   struct mpu9x50_config config = {
      .gyro_lpf = MPU9X50_GYRO_LPF_20HZ,
      .acc_lpf  = MPU9X50_ACC_LPF_20HZ,
      .gyro_fsr = MPU9X50_GYRO_FSR_500DPS,
      .acc_fsr  = MPU9X50_ACC_FSR_4G,
      .gyro_sample_rate = gyro_sample_rate,
      .compass_enabled = true,
      .compass_sample_rate = compass_sample_rate,
      .fifo_enabled = false,
      .spi_dev_path = SPI_DEV_PATH,
   };

   // initialize signal
   sigemptyset(&set);
   sigaddset(&set, SIGRTMIN);

   // open and initialize mpu9x50 device
   if (mpu9x50_initialize(&config) != 0)
   {
      FARF(ALWAYS, "mpu9x50_initialize() failed");
      goto exit;
   }

   // register interrupt handler and enable data ready interrupt mode
   if (mpu9x50_register_interrupt(SPI_INT_GPIO, &data_ready_isr, NULL) != 0)
   {
      FARF(ALWAYS, "mpu9x50_register_interrupt() failed");
      goto exit;
   }

   clock_gettime(CLOCK_REALTIME, &ts_cycle_start);

   // Run until we collected MEASUREMENT_MAX_NUM samples
   while(total_gyro_samples < MEASUREMENT_MAX_NUM)
   {
      // wait on signal
      rv = sigwait(&set, &sig);

      // check if we are waken up by the proper signal
      if (rv != 0 || sig != SIGRTMIN)
      {
         FARF(ALWAYS, "sigwait failed rv %d sig %d", rv, sig);
         continue;
      }

      clock_gettime(CLOCK_REALTIME, &ts_io_start);

      // calculate the thread wakeup delay since the INT fires
      thread_wakeup_time_us[total_gyro_samples] =
         ts_io_start.tv_sec*1000000+ts_io_start.tv_nsec/1000 - ts_int;

      // read new IMU sample
      if (mpu9x50_get_data(&data) != 0)
      {
         FARF(ALWAYS, "mpu9x50_get_data() failed");
         continue;
      }

      clock_gettime(CLOCK_REALTIME, &ts_io_end);
      io_time_us[total_gyro_samples] =
            (ts_io_end.tv_sec - ts_io_start.tv_sec) * 1000000 +
                  (ts_io_end.tv_nsec - ts_io_start.tv_nsec) / 1000;


      memcpy(&imu_samples[total_gyro_samples], &data, sizeof(struct mpu9x50_data));

      clock_gettime(CLOCK_REALTIME, &ts_cycle_end);
      sample_interval_us[total_gyro_samples] =
            (ts_cycle_end.tv_sec - ts_cycle_start.tv_sec) * 1000000 +
                  (ts_cycle_end.tv_nsec - ts_cycle_start.tv_nsec) / 1000;
      ts_cycle_start = ts_cycle_end;

      total_gyro_samples++;
      total_mag_samples += data.mag_data_ready;
   }

   exit_mreasurement = true;

   /* dump all the samples */
   for (i = 0; i < total_gyro_samples; i++)
   {
      tmp = &imu_samples[i];
      struct mpu9x50_data *prev;

      if (i > 0) {
         prev = &imu_samples[i-1];

         // We compare the adjacent samples to detect potential outlier values.
         // We observed that high SPI bus speed may result in corrupted bytes
         if ((abs((int)(tmp->temperature*1000) - (int)(prev->temperature*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->accel_raw[0]*tmp->accel_scaling*1000) - (int)(prev->accel_raw[0]*prev->accel_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->accel_raw[1]*tmp->accel_scaling*1000) - (int)(prev->accel_raw[1]*prev->accel_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->accel_raw[2]*tmp->accel_scaling*1000) - (int)(prev->accel_raw[2]*prev->accel_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->gyro_raw[0]*tmp->gyro_scaling*1000) - (int)(prev->gyro_raw[0]*prev->gyro_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->gyro_raw[1]*tmp->gyro_scaling*1000) - (int)(prev->gyro_raw[1]*prev->gyro_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->gyro_raw[2]*tmp->gyro_scaling*1000) - (int)(prev->gyro_raw[2]*prev->gyro_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000)) {
            total_outlier_samples++;
         }
         FARF(ALWAYS, "[%d] ts %llu temp %d accel %d %d %d gyro %d %d %d",
              i, tmp->timestamp, (int)(tmp->temperature*1000),
              (int)(tmp->accel_raw[0]*tmp->accel_scaling*1000),
              (int)(tmp->accel_raw[1]*tmp->accel_scaling*1000),
              (int)(tmp->accel_raw[2]*tmp->accel_scaling*1000),
              (int)(tmp->gyro_raw[0]*tmp->gyro_scaling*1000),
              (int)(tmp->gyro_raw[1]*tmp->gyro_scaling*1000),
              (int)(tmp->gyro_raw[2]*tmp->gyro_scaling*1000));
      }
   }
   FARF(ALWAYS, "total gyro samples: %llu mag samples: %llu outliers: %llu",
        total_gyro_samples, total_mag_samples, total_outlier_samples);

   /* calculate the profiling statistics */
   float avg, stdev;
   uint64_t min, max;

   calculate_stats(io_time_us, total_gyro_samples,
                   &avg, &stdev, &min, &max);
   FARF(ALWAYS, "io time(us): avg %d stdev %d min %llu max %llu",
        (int)(avg), (int)(stdev), min, max);

   calculate_stats(thread_wakeup_time_us, total_gyro_samples,
                   &avg, &stdev, &min, &max);
   FARF(ALWAYS, "thread wakeup delay(us): avg %d stdev %d min %llu max %llu",
        (int)(avg), (int)(stdev), min, max);

   calculate_stats(sample_interval_us, total_gyro_samples,
                   &avg, &stdev, &min, &max);
   FARF(ALWAYS, "sample interval time(us): avg %d stdev %d min %llu max %llu",
        (int)(avg), (int)(stdev), min, max);

   int avg_gyro_sample_freq = (int)(total_gyro_samples/(total_gyro_samples*avg)*1000000);
   int avg_compass_sample_freq = (int)(total_mag_samples/(total_gyro_samples*avg)*1000000);

   FARF(ALWAYS, "avg sample freq(Hz): gyro %d mag %d", avg_gyro_sample_freq,
        avg_compass_sample_freq);

   // consider TEST PASS if actual sample frequency is within normal error range
   // consider TEST PASS if actual sample frequency is within normal error range
   if (abs(gyro_sample_rate_hz - avg_gyro_sample_freq) <
       (int)(SAMPLE_FREQ_ERROR_THRESHOLD * gyro_sample_rate_hz) &&
       abs(compass_sample_rate_hz - avg_compass_sample_freq) <
       (int)(SAMPLE_FREQ_ERROR_THRESHOLD * compass_sample_rate_hz))
   {
      mpu9x50_data_ready_interrupt_test_result = TEST_PASS;
   }
   else
   {
      FARF(ALWAYS, "actual sample frequency exceed error threshold!");
   }

exit:
   FARF(ALWAYS, "exiting measurement_thread_main");

   mpu9x50_close();

   pthread_exit((void *) MEASUREMENT_THREAD_EXIT_STATUS);

   return NULL;
}

/**
 * @brief
 * Test getting mpu9x50 gyro, accel, temperature and compass measurement in DRI
 * mode with 1KHz sample rate
 *
 * @par
 * This test spawns a separate measurement thread to run the test in. It registers a
 * GPIO ISR to handle the IMU data ready interrupt. On interrupt, the ISR wakes
 * up the measurement thread to read all sensor measurements.
 * The performance of the reads
 * is also profiled and shown in the console.
 *
 * ISR functions sets a flag indicating it was called and kills the thread
 * created in step 1.
 *
 * NOTE: due to the SPI transfer overhead, data ready interrupt and data polling
 * can only support up to 1KHz sample rate. To achieve 8KHz sample rate, use
 * FIFO mode.
 *
 * @par
 * Test
 * 1) Spawn a separate thread and join it to this thread
 * In separate thread:
 * 2) Initialize the mpu9x50 device
 * 3) Register an ISR function with the mpu9x50 device
 * 4) Setup signals and timers
 * 5) Waits on signal
 * 6) Reads data from mpu9x50 device
 * 7) Profiles the data read
 * 8) Repeat steps 5-7 a few times
 * 9) Close the mpu9x50 device
 * 10) kill the current thread
 *
 * @return
 * TEST_PASS ------ Test Passes
 * TEST_FAIL ------ Test Failed
*/
int mpu_tester_test_mpu9x50_data_ready_interrupt()
{
   int *exit_status;
   int rv;
   int test_result = TEST_PASS;

   data_ready_isr_called = false;

   // NOTE: Default qurt posix thread stack size is 1KB. We must increase
   // the stack size to accommodate the measurement thread. Otherwise, stack
   // corruption occurs and cause adsp to crash
   pthread_attr_t attr;
   size_t stacksize = -1;
   pthread_attr_init(&attr);
   pthread_attr_getstacksize(&attr, &stacksize);
   FARF(ALWAYS, "stack size: %d", stacksize);
   stacksize = 8 * 1024;

   FARF(ALWAYS, "setting the thread stack size to[%d]", stacksize);
   pthread_attr_setstacksize(&attr, stacksize);

   // Create measurement thread
   if (pthread_create(&measurement_thread, &attr, measurement_thread_main,
                      NULL) != 0)
   {
      FARF(ALWAYS, "pthread_create failed");
      test_result = TEST_FAIL;
      goto exit;
   }

   FARF(ALWAYS, "measurement_thread created");

   // wait until measurement thread exits
   rv = pthread_join(measurement_thread, (void **)&exit_status);
   if (rv != 0 || exit_status != (void *)MEASUREMENT_THREAD_EXIT_STATUS)
   {
      FARF(ALWAYS, "pthread_join error: rv %d exit status %d", rv, exit_status);
      test_result = TEST_FAIL;
      goto exit;
   }

   // check if data ready isr is called.
   if (!data_ready_isr_called)
   {
      FARF(ALWAYS, "data ready ISR is not called");
      test_result = TEST_FAIL;
      goto exit;
   }

   test_result = mpu9x50_data_ready_interrupt_test_result;

exit:
   FARF(ALWAYS, "**** mpu_tester_test_mpu9x50_data_ready_interrupt: %s ****",
        test_result == TEST_PASS ? "PASSED" : "FAILED");

   return test_result;
}


static int mpu9x50_poll_data_test_result;
/**
 * mpu9x50_poll_data_main thread routine
 */
void *mpu9x50_poll_data_main(void *context)
{
   struct sigevent sev;
   timer_t timer_id = 0;
   struct itimerspec timer_spec;
   sigset_t set;
   int rv;
   int sig;

   enum gyro_sample_rate_e gyro_sample_rate;
   enum compass_sample_rate_e compass_sample_rate;
   uint64_t gyro_sample_cycle_us;
   int gyro_sample_rate_hz;
   int compass_sample_rate_hz;

   uint64_t total_gyro_samples = 0;
   uint64_t total_mag_samples = 0;
   uint64_t total_outlier_samples = 0;
   struct mpu9x50_data *tmp;
   struct mpu9x50_data data;
   uint64_t i;
   struct timespec ts_cycle_start, ts_cycle_end;
   struct timespec ts_io_start, ts_io_end;

   // initialize test result
   mpu9x50_poll_data_test_result = TEST_FAIL;

   FARF(ALWAYS,"mpu9x50_poll_data_main thread id = %d", pthread_self());

   /*--------------------------------------------------------
    *                 Test Settings
    * Tune the following two parameters to set different sample
    * rate
    ---------------------------------------------------------*/
   gyro_sample_rate = MPU9x50_SAMPLE_RATE_1000HZ;
   compass_sample_rate = MPU9x50_COMPASS_SAMPLE_RATE_100HZ;

   struct mpu9x50_config config = {
      .gyro_lpf = MPU9X50_GYRO_LPF_20HZ,
      .acc_lpf  = MPU9X50_ACC_LPF_20HZ,
      .gyro_fsr = MPU9X50_GYRO_FSR_500DPS,
      .acc_fsr  = MPU9X50_ACC_FSR_4G,
      .gyro_sample_rate = gyro_sample_rate,
      .compass_enabled = true,
      .compass_sample_rate = compass_sample_rate,
      .fifo_enabled = false,
      .spi_dev_path = SPI_DEV_PATH,
   };

   /* set up periodic timer */
   // set event notification function
   sev.sigev_notify           = SIGEV_SIGNAL;
   sev.sigev_signo            = SIGRTMIN;
   sev.sigev_value.sival_int  = SIGRTMIN;
   sev.sigev_notify_function  = 0;
   sev.sigev_notify_attributes = 0;

   // create timer
   if (timer_create(CLOCK_REALTIME, &sev, &timer_id) != 0)
   {
      FARF(ALWAYS,"timer_create failed");
      goto exit;
   }

   gyro_sample_rate_hz = gyro_sample_rate_enum_to_hz(gyro_sample_rate);
   gyro_sample_cycle_us = 1000000/gyro_sample_rate_hz;
   compass_sample_rate_hz = compass_sample_rate_enum_to_hz(compass_sample_rate);

   // set periodic timer
   timer_spec.it_value.tv_sec     = 0;
   timer_spec.it_value.tv_nsec    = gyro_sample_cycle_us * 1000;
   timer_spec.it_interval.tv_sec  = 0;
   timer_spec.it_interval.tv_nsec = gyro_sample_cycle_us * 1000;

   FARF(ALWAYS, "sample period %llu us", gyro_sample_cycle_us);

   // set SIGRTMIN as the signal type to wait on
   sigemptyset(&set);
   sigaddset(&set, SIGRTMIN);

   // open and initialize mpu9x50 device
   if (mpu9x50_initialize(&config) != 0)
   {
      FARF(ALWAYS, "mpu9x50_initialize() failed");
      goto exit;
   }

   // start the timer
   if (timer_settime(timer_id, 0, &timer_spec, NULL) != 0)
   {
      FARF(ALWAYS,"timer_settime failed");
      timer_delete(timer_id);
      goto exit;
   }

   clock_gettime(CLOCK_REALTIME, &ts_cycle_start);

   // Run until we collected MEASUREMENT_MAX_NUM samples
   while(total_gyro_samples < MEASUREMENT_MAX_NUM)
   {
      // wait on the signal
      rv = sigwait(&set, &sig);

      // check the received signal
      if (rv != 0 || sig != SIGRTMIN)
      {
         FARF(ALWAYS, "sigwait failed rv %d sig %d", rv, sig);
         continue;
      }

      clock_gettime(CLOCK_REALTIME, &ts_io_start);

      // read new IMU sample
      if (mpu9x50_get_data(&data) != 0)
      {
         FARF(ALWAYS, "mpu9x50_get_data() failed");
         continue;
      }

      clock_gettime(CLOCK_REALTIME, &ts_io_end);
      io_time_us[total_gyro_samples] =
            (ts_io_end.tv_sec - ts_io_start.tv_sec) * 1000000 +
                  (ts_io_end.tv_nsec - ts_io_start.tv_nsec) / 1000;

      memcpy(&imu_samples[total_gyro_samples], &data, sizeof(struct mpu9x50_data));

      clock_gettime(CLOCK_REALTIME, &ts_cycle_end);
      sample_interval_us[total_gyro_samples] =
            (ts_cycle_end.tv_sec - ts_cycle_start.tv_sec) * 1000000 +
                  (ts_cycle_end.tv_nsec - ts_cycle_start.tv_nsec) / 1000;

      ts_cycle_start = ts_cycle_end;

      total_gyro_samples++;
      total_mag_samples += data.mag_data_ready;
   }

   // delete the timer
   timer_delete(timer_id);

   /* dump all the samples */
   for (i = 0; i < total_gyro_samples; i++)
   {
      tmp = &imu_samples[i];
      struct mpu9x50_data *prev;

      if (i > 0) {
         prev = &imu_samples[i-1];

         // We compare the adjacent samples to detect potential outlier values.
         // We observed that high SPI bus speed may result in corrupted bytes
         if ((abs((int)(tmp->temperature*1000) - (int)(prev->temperature*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->accel_raw[0]*tmp->accel_scaling*1000) - (int)(prev->accel_raw[0]*prev->accel_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->accel_raw[1]*tmp->accel_scaling*1000) - (int)(prev->accel_raw[1]*prev->accel_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->accel_raw[2]*tmp->accel_scaling*1000) - (int)(prev->accel_raw[2]*prev->accel_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->gyro_raw[0]*tmp->gyro_scaling*1000) - (int)(prev->gyro_raw[0]*prev->gyro_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->gyro_raw[1]*tmp->gyro_scaling*1000) - (int)(prev->gyro_raw[1]*prev->gyro_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->gyro_raw[2]*tmp->gyro_scaling*1000) - (int)(prev->gyro_raw[2]*prev->gyro_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000)) {
            total_outlier_samples++;
         }
         FARF(ALWAYS, "[%d] ts %llu temp %d accel %d %d %d gyro %d %d %d",
              i, tmp->timestamp, (int)(tmp->temperature*1000),
              (int)(tmp->accel_raw[0]*tmp->accel_scaling*1000),
              (int)(tmp->accel_raw[1]*tmp->accel_scaling*1000),
              (int)(tmp->accel_raw[2]*tmp->accel_scaling*1000),
              (int)(tmp->gyro_raw[0]*tmp->gyro_scaling*1000),
              (int)(tmp->gyro_raw[1]*tmp->gyro_scaling*1000),
              (int)(tmp->gyro_raw[2]*tmp->gyro_scaling*1000));
      }
   }
   FARF(ALWAYS, "total gyro samples: %llu mag samples: %llu outliers: %llu",
        total_gyro_samples, total_mag_samples, total_outlier_samples);

   /* calculate the profiling statistics */
   float avg, stdev;
   uint64_t min, max;

   calculate_stats(io_time_us, total_gyro_samples,
                   &avg, &stdev, &min, &max);
   FARF(ALWAYS, "io time(us): avg %d stdev %d min %llu max %llu",
        (int)(avg), (int)(stdev), min, max);

   calculate_stats(sample_interval_us, total_gyro_samples,
                   &avg, &stdev, &min, &max);
   FARF(ALWAYS, "sample interval time(us): avg %d stdev %d min %llu max %llu",
        (int)(avg), (int)(stdev), min, max);

   int avg_gyro_sample_freq = (int)(total_gyro_samples/(total_gyro_samples*avg)*1000000);
   int avg_compass_sample_freq = (int)(total_mag_samples/(total_gyro_samples*avg)*1000000);

   FARF(ALWAYS, "avg sample freq(Hz): gyro %d mag %d", avg_gyro_sample_freq,
        avg_compass_sample_freq);

   // consider TEST PASS if actual sample frequency is within normal error range
   // consider TEST PASS if actual sample frequency is within normal error range
   if (abs(gyro_sample_rate_hz - avg_gyro_sample_freq) <
       (int)(SAMPLE_FREQ_ERROR_THRESHOLD * gyro_sample_rate_hz) &&
       abs(compass_sample_rate_hz - avg_compass_sample_freq) <
       (int)(SAMPLE_FREQ_ERROR_THRESHOLD * compass_sample_rate_hz))
   {
      mpu9x50_poll_data_test_result = TEST_PASS;
   }
   else
   {
      FARF(ALWAYS, "actual sample frequency exceed error threshold!");
   }

exit:
   FARF(ALWAYS, "exiting mpu9x50_poll_data_thread");

   mpu9x50_close();

   pthread_exit((void *)MEASUREMENT_THREAD_EXIT_STATUS);
   return NULL;
}

/**
 * @brief
 * Test getting mpu9x50 gyro, accel, temperature and compass measurement in
 * polling mode with 1KHz sample rate
 *
 * @par
 * This test spawns a separate measurement thread to run the test in. The
 * measurement thread sets up a 1KHz timer. When timer fires, we read gyro,
 * accel, temperature and compass measurement values from the registers over
 * SPI bus.
 * The performance of the reads is also profiled and shown in the console.
 *
 * @par
 * Test
 * 1) Spawn a separate thread and join it to this thread
 * In separate thread:
 * 2) Initialize the mpu9x50 device
 * 3) Setup signals and timers
 * 4) Waits on signal
 * 5) Reads data from mpu9x50 device
 * 6) Profiles the data read
 * 7) Repeat steps 4-6 a few times
 * 9) Close the mpu9x50 device
 * 9) kill the current thread
 *
 * @return
 * TEST_PASS ------ Test Passes
 * TEST_FAIL ------ Test Failed
*/
int mpu_tester_test_mpu9x50_poll_data()
{
   int *exit_status;
   int rv;
   int test_result = TEST_PASS;

   // NOTE: default qurt posix thread stack size is 1KB. We must increase
   // the stack size to accommodate the measurement thread. Otherwise, stack
   // corruption occurs and cause adsp to crash
   pthread_attr_t attr;
   size_t stacksize = -1;
   pthread_attr_init(&attr);
   pthread_attr_getstacksize(&attr, &stacksize);
   FARF(ALWAYS, "stack size: %d", stacksize);
   stacksize = 8 * 1024;

   FARF(ALWAYS, "setting the thread stack size to[%d]", stacksize);
   pthread_attr_setstacksize(&attr, stacksize);

   // create measurement thread
   if (pthread_create(&mpu9x50_poll_data_thread, &attr,
       mpu9x50_poll_data_main, NULL) != 0)
   {
      FARF(ALWAYS, "pthread_create failed");
      test_result = TEST_FAIL;
      goto exit;
   }

   FARF(ALWAYS, "mpu9x50_poll_data_main created");

   // wait until measurement thread exits
   rv = pthread_join(mpu9x50_poll_data_thread, (void **)&exit_status);
   if (rv != 0 || exit_status != (void *)MEASUREMENT_THREAD_EXIT_STATUS)
   {
      FARF(ALWAYS, "pthread_join error: rv %d exit status %d", rv, exit_status);
      test_result = TEST_FAIL;
      goto exit;
   }

   test_result = mpu9x50_poll_data_test_result;

exit:
   FARF(ALWAYS, "**** mpu_tester_test_mpu9x50_poll_data: %s ****",
        test_result == TEST_PASS ? "PASSED" : "FAILED");

   return test_result;
}

static int mpu9x50_fifo_poll_data_test_result;

/**
 * mpu9x50_fifo_poll_data_main thread routine
 */
void *mpu9x50_fifo_poll_data_main(void *context)
{
   struct sigevent sev;
   timer_t timer_id = 0;
   struct itimerspec timer_spec;
   sigset_t set;
   int rv;
   int sig;

   // initialize test result
   mpu9x50_fifo_poll_data_test_result = TEST_FAIL;

   enum gyro_sample_rate_e gyro_sample_rate;
   int gyro_sample_rate_hz;
   uint64_t gyro_sample_cycle_us;
   enum gyro_lpf_e gyro_lpf;
   uint64_t fifo_wait_cycle;
   uint8_t fifo_en_mask = 0;

   struct mpu9x50_data *data_list = NULL;
   uint64_t total_samples;
   uint64_t total_outlier_samples = 0;
   struct mpu9x50_data *tmp;
   int i;
   struct timespec ts_io_start, ts_io_end;
   struct timespec ts_cycle_start, ts_cycle_end;

   FARF(ALWAYS,"mpu9x50_fifo_poll_data_main thread id = %d", pthread_self());

   /*--------------------------------------------------------
    *                 Test Settings
    * Tune the following two parameters to set different
    * sample rate and fifo wait time
    ---------------------------------------------------------*/
   gyro_sample_rate = MPU9x50_SAMPLE_RATE_8000HZ; // gyro sample rate
   fifo_wait_cycle = 16;    // do FIFO transfer for every 16 samples (2ms)
   fifo_en_mask |= BIT_TEMP_FIFO_EN;  // enable FIFO on temperature
   fifo_en_mask |= BIT_GYRO_FIFO_EN;  // enable FIFO on gyro
   fifo_en_mask |= BIT_ACCEL_FIFO_EN; // enable FIFO on accel
   /*-------------- End of Test Settings ----------------*/


   gyro_sample_rate_hz = gyro_sample_rate_enum_to_hz(gyro_sample_rate);
   gyro_sample_cycle_us = 1000000/gyro_sample_rate_hz;
   if (gyro_sample_rate == MPU9x50_SAMPLE_RATE_8000HZ)
   {
      gyro_lpf = MPU9X50_GYRO_LPF_250HZ;
   }
   else
   {
      gyro_lpf = MPU9X50_GYRO_LPF_20HZ;
   }

   /* mpu9x50 driver configuration */
   struct mpu9x50_config config = {
      .gyro_lpf = gyro_lpf,
      .acc_lpf  = MPU9X50_ACC_LPF_20HZ,
      .gyro_fsr = MPU9X50_GYRO_FSR_500DPS,
      .acc_fsr  = MPU9X50_ACC_FSR_4G,
      .gyro_sample_rate = gyro_sample_rate,
      .compass_enabled = false,
      .compass_sample_rate = MPU9x50_COMPASS_SAMPLE_RATE_100HZ,
      .fifo_enabled = true,
      .fifo_en_mask = fifo_en_mask,
      .spi_dev_path = SPI_DEV_PATH,
   };

   /* set up periodic timer */
   // set event notification function
   sev.sigev_notify           = SIGEV_SIGNAL;
   sev.sigev_signo            = SIGRTMIN;
   sev.sigev_value.sival_int  = SIGRTMIN;
   sev.sigev_notify_function  = 0;
   sev.sigev_notify_attributes = 0;

   // create timer
   if (timer_create(CLOCK_REALTIME, &sev, &timer_id) != 0)
   {
      FARF(ALWAYS,"timer_create failed");
      goto exit;
   }

   // set timer period to collect fifo_wait_cycle samples
   timer_spec.it_value.tv_sec     = 0;
   timer_spec.it_value.tv_nsec    = gyro_sample_cycle_us*fifo_wait_cycle * 1000;
   timer_spec.it_interval.tv_sec  = 0;
   timer_spec.it_interval.tv_nsec = gyro_sample_cycle_us*fifo_wait_cycle * 1000;

   FARF(ALWAYS, "sample period %llu us FIFO wait cycle %llu",
        gyro_sample_cycle_us, fifo_wait_cycle);

   // set SIGRTMIN as the signal type to wait on
   sigemptyset(&set);
   sigaddset(&set, SIGRTMIN);

   // open and initialize mpu9x50 device
   if (mpu9x50_initialize(&config) != 0)
   {
      FARF(ALWAYS, "mpu9x50_initialize() failed");
      goto exit;
   }

   // Allocate enough space to accomandate the MPU9250 buffer size
   int max_burst_read_samples = mpu9x50_get_fifo_size() / 14;
   if ((data_list = (struct mpu9x50_data *)malloc(sizeof(struct mpu9x50_data)*max_burst_read_samples)) == NULL)
   {
      FARF(ALWAYS, "memory failure");
      goto exit;
   }

   mpu9x50_start_fifo();

   // start the timer
   if (timer_settime(timer_id, 0, &timer_spec, NULL) != 0)
   {
      FARF(ALWAYS,"timer_settime failed");
      timer_delete(timer_id);
      goto exit;
   }

   uint64_t sample_reading_cycles = 0;

   // reset the counters
   total_samples = 0;

   clock_gettime(CLOCK_REALTIME, &ts_cycle_start);

   // Run until we collected MEASUREMENT_MAX_NUM samples
   while(total_samples < MEASUREMENT_MAX_NUM)
   {
      int samples;

      // wait on the signal
      rv = sigwait(&set, &sig);

      // check the received signal
      if (rv != 0 || sig != SIGRTMIN)
      {
         FARF(ALWAYS, "sigwait failed rv %d sig %d", rv, sig);
         continue;
      }

      clock_gettime(CLOCK_REALTIME, &ts_io_start);

      samples = mpu9x50_read_fifo(data_list, max_burst_read_samples);
      if (samples < 0)
      {
         FARF(ALWAYS, "failed to read mpu9250 FIFO data");
         break;
      }

      // we only record up to MEASUREMENT_MAX_NUM samples
      if (samples + total_samples >= MEASUREMENT_MAX_NUM)
      {
         samples = MEASUREMENT_MAX_NUM - total_samples;
      }

      // save samples to a large array.
      // NOTE: FARF() has very high overhead, so be very careful to print
      // during the measurement cycle. This may delay the next FIFO reading
      // and cause some time critical operations like IMU reading to
      // malfunction!! In particular, with 8KHz sample rate, the timing is
      // very critical.
      for (i = 0; i < samples; i++)
      {
         tmp = &data_list[i];
         memcpy(&imu_samples[total_samples+i], tmp, sizeof(struct mpu9x50_data));
      }

      total_samples += samples;

      clock_gettime(CLOCK_REALTIME, &ts_io_end);

      io_time_us[sample_reading_cycles] =
            (ts_io_end.tv_sec - ts_io_start.tv_sec) * 1000000 +
                  (ts_io_end.tv_nsec - ts_io_start.tv_nsec) / 1000;

      clock_gettime(CLOCK_REALTIME, &ts_cycle_end);
      sample_interval_us[sample_reading_cycles] =
            (ts_cycle_end.tv_sec - ts_cycle_start.tv_sec) * 1000000 +
                  (ts_cycle_end.tv_nsec - ts_cycle_start.tv_nsec) / 1000;
      ts_cycle_start = ts_cycle_end;

      sample_reading_cycles++;
   }

   // stop FIFO operation
   mpu9x50_stop_fifo();

   // delete the timer
   timer_delete(timer_id);
   timer_id = 0;

   /* dump all the samples */
   for (i = 0; i < total_samples; i++)
   {
      tmp = &imu_samples[i];
      struct mpu9x50_data *prev;

      if (i > 0) {
         prev = &imu_samples[i-1];

         // We compare the adjacent samples to detect potential outlier values.
         // We observed that high SPI bus speed may result in corrupted bytes
         if ((abs((int)(tmp->temperature*1000) - (int)(prev->temperature*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->accel_raw[0]*tmp->accel_scaling*1000) - (int)(prev->accel_raw[0]*prev->accel_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->accel_raw[1]*tmp->accel_scaling*1000) - (int)(prev->accel_raw[1]*prev->accel_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->accel_raw[2]*tmp->accel_scaling*1000) - (int)(prev->accel_raw[2]*prev->accel_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->gyro_raw[0]*tmp->gyro_scaling*1000) - (int)(prev->gyro_raw[0]*prev->gyro_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->gyro_raw[1]*tmp->gyro_scaling*1000) - (int)(prev->gyro_raw[1]*prev->gyro_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000) ||
             (abs((int)(tmp->gyro_raw[2]*tmp->gyro_scaling*1000) - (int)(prev->gyro_raw[2]*prev->gyro_scaling*1000)) > MEASUREMENT_OUTLIER_THRESHOLD*1000)) {
            total_outlier_samples++;
         }
         FARF(ALWAYS, "[%d] ts %llu temp %d accel %d %d %d gyro %d %d %d",
              i, tmp->timestamp, (int)(tmp->temperature*1000),
              (int)(tmp->accel_raw[0]*tmp->accel_scaling*1000),
              (int)(tmp->accel_raw[1]*tmp->accel_scaling*1000),
              (int)(tmp->accel_raw[2]*tmp->accel_scaling*1000),
              (int)(tmp->gyro_raw[0]*tmp->gyro_scaling*1000),
              (int)(tmp->gyro_raw[1]*tmp->gyro_scaling*1000),
              (int)(tmp->gyro_raw[2]*tmp->gyro_scaling*1000));
      }
   }
   FARF(ALWAYS, "total gyro samples: %llu outliers %llu",
        total_samples, total_outlier_samples);

   /* calculate the profiling statistics */
   float avg, stdev;
   uint64_t min, max;

   FARF(ALWAYS, "total FIFO readings %llu", sample_reading_cycles);

   calculate_stats(io_time_us, sample_reading_cycles,
                   &avg, &stdev, &min, &max);
   FARF(ALWAYS, "fifo io time(us): avg %d stdev %d min %llu max %llu",
        (int)(avg), (int)(stdev), min, max);

   calculate_stats(sample_interval_us, sample_reading_cycles,
                   &avg, &stdev, &min, &max);
   FARF(ALWAYS, "reading cycle(us): avg %d stdev %d min %llu max %llu",
        (int)(avg), (int)(stdev), min, max);
   int avg_sample_freq = (int)(total_samples/(sample_reading_cycles*avg)*1000000);
   FARF(ALWAYS, "avg IMU sample freq(Hz): %d", avg_sample_freq);

   // consider TEST PASS if actual sample frequency is within normal error range
   if (abs(gyro_sample_rate_hz - avg_sample_freq) <
       (int)(SAMPLE_FREQ_ERROR_THRESHOLD * gyro_sample_rate_hz))
   {
      mpu9x50_fifo_poll_data_test_result = TEST_PASS;
   }
   else
   {
      FARF(ALWAYS, "actual sample frequency exceed error threshold!");
   }

exit:
   if (data_list)
   {
      free(data_list);
   }

   if (timer_id != 0)
   {
      // delete the timer
      timer_delete(timer_id);
   }

   FARF(ALWAYS, "exiting mpu9x50_fifo_poll_data_thread");

   mpu9x50_close();

   pthread_exit((void *)FIFO_POLL_THREAD_EXIT_STATUS);
   return NULL;
}

/**
 * @brief
 * Test reading mpu9x50 gyro, accel and temperature measurements through
 * FIFO mode.
 *
 * @par
 * This test spawns a separate thread to run the test in.  IMU is configured
 * to sample gyro at 8KHz. The accelerometer and temperature is measured at
 * lower sample rate. We use 1KHz timer to schedule reading samples from FIFO
 * buffer periodically. Each time we read around 8 samples in a burst.
 * NOTE: The choice of reading 8 samples is based on careful tuning to ensure
 * both reliability and timeliness of the IMU measurement
 * - The total SPI I/O cost to read 8 samples from FIFO buffer is about 450us
 * - The FIFO operation is conducted every 1000us.
 * - Within 1000us, we have enough time to finish FIFO I/O and leave sufficient
 *   time for other operations. Also, this ensures samples are not too old.
 * The performance of the reads is also profiled and shown in the console.
 *
 * @par
 * Test
 * 1) Spawn a separate thread and join it to this thread
 * In separate thread:
 * 2) Initialize the mpu9x50 device
 * 3) Start FIFO operation
 * 4) Setup signals and timers
 * 5) Waits on signal
 * 6) Reads a group of samples from mpu9x50 FIFO buffer
 * 7) Profiles the data read
 * 8) Repeat steps 5-7 a few times
 * 9) Close the mpu9x50 device
 * 10) kill the current thread
 *
 * @return
 * TEST_PASS ------ Test Passes
 * TEST_FAIL ------ Test Failed
*/
int mpu_tester_test_mpu9x50_fifo_poll_data()
{
   int *exit_status;
   int rv;
   int test_result = TEST_PASS;

   // NOTE: default qurt posix thread stack size is 1KB. We must increase
   // the stack size to accommodate the measurement thread. Otherwise, stack
   // corruption occurs and cause adsp to crash
   pthread_attr_t attr;
   size_t stacksize = -1;
   pthread_attr_init(&attr);
   pthread_attr_getstacksize(&attr, &stacksize);
   FARF(ALWAYS, "stack size: %d", stacksize);
   stacksize = 8 * 1024;

   FARF(ALWAYS, "setting the thread stack size to[%d]", stacksize);
   pthread_attr_setstacksize(&attr, stacksize);

   // create measurement thread
   if (pthread_create(&mpu9x50_fifo_poll_data_thread, &attr,
       mpu9x50_fifo_poll_data_main, NULL) != 0)
   {
      FARF(ALWAYS, "pthread_create failed");
      test_result = TEST_FAIL;
      goto exit;
   }

   FARF(ALWAYS, "mpu9x50_fifo_poll_data_main created");

   // wait until measurement thread exits
   rv = pthread_join(mpu9x50_fifo_poll_data_thread, (void **)&exit_status);
   if (rv != 0 || exit_status != (void *)FIFO_POLL_THREAD_EXIT_STATUS)
   {
      FARF(ALWAYS, "pthread_join error: rv %d exit status %d", rv, exit_status);
      test_result = TEST_FAIL;
      goto exit;
   }


   test_result = mpu9x50_fifo_poll_data_test_result;

exit:
   FARF(ALWAYS, "**** mpu_tester_test_mpu9x50_fifo_poll_data: %s ****",
        test_result == TEST_PASS ? "PASSED" : "FAILED");

   return test_result;
}
