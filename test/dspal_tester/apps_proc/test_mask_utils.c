/****************************************************************************
 *   Copyright (c) 2016 James Wilson. All rights reserved.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "test_utils.h"
#include "test_mask_utils.h"

void test_mask_utils_print_help()
{
  printf("dspal_tester [-h|--help] [test_mask]\n\n");
  printf("test_mask: string containing 0 in position of disabled tests, 1 in position of enabled tests\n\n");
  printf("List of tests:\n");
  printf(" 1) test_clockid\n");
  printf(" 2) test_sigevent\n");
  printf(" 3) test_time\n");
  printf(" 4) test_timer_realtime_sig_none\n");
  printf(" 5) test_timer_monotonic_sig_none\n");
  printf(" 6) test_timer_process_cputime_sig_none\n");
  printf(" 7) test_timer_thread_cputime_sig_none\n");
  printf(" 8) test_time_return_value\n");
  printf(" 9) test_time_param\n");
  printf("10) test_usleep\n");
  printf("11) test_clock_getres\n");
  printf("12) test_clock_gettime\n");
  printf("13) test_clock_settime\n");
  printf("14) test_one_shot_timer_cb\n");
  printf("15) test_periodic_timer_cb\n");
  printf("16) test_periodic_timer_signal_cb\n");
  printf("17) test_periodic_timer_sigwait\n");
  printf("18) test_pthread_attr_init\n");
  printf("19) test_pthread_create\n");
  printf("20) test_pthread_cancel\n");
  printf("21) test_pthread_self\n");
  printf("22) test_pthread_exit\n");
  printf("23) test_pthread_kill\n");
  printf("24) test_pthread_cond_timed_wait\n");
  printf("25) test_pthread_mutex_lock\n");
  printf("26) test_pthread_mutex_lock_thread\n");
  printf("27) test_pthread_stack\n");
  printf("28) test_pthread_heap\n");
  printf("29) test_usleep\n");
  printf("30) test_semaphore_wait\n");
  printf("31) test_cxx_heap\n");
  printf("32) test_cxx_static\n");
  printf("33) spi_test\n");
  printf("34) serial_test\n");
  printf("35) termios_test\n");
  printf("36) i2c_test\n");
  printf("37) pwm_test\n");
  printf("38) test_farf_log_info\n");
  printf("39) test_farf_log_err\n");
  printf("40) test_farf_log_debug\n");
  printf("41) test_gpio_open_close\n");
  printf("42) test_gpio_ioctl_io\n");
  printf("43) test_gpio_read_write\n");
  printf("44) test_gpio_int\n");
  printf("45) test_posix_file_open_close\n");
  printf("46) test_posix_file_read_write\n");
  printf("47) test_posix_file_open_trunc\n");
  printf("48) test_posix_file_open_append\n");
  printf("49) test_posix_file_ioctl\n");
  printf("50) test_posix_file_fsync\n");
  printf("51) test_posix_file_remove\n");
  printf("52) test_fopen_fclose\n");
  printf("53) test_fwrite_fread\n");
}

int test_mask_utils_process_cli_args(int argc, char* argv[], char test_mask[])
{
  if (argc < 2)
  {
    int i;
    for (i = 0; i < TOTAL_NUM_DSPAL_TESTS; i++)
    {
      test_mask[i] = '1';
    }
    test_mask[i] = '\0';
  }
  else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
  {
    test_mask_utils_print_help();
    return -1;
  }
  else
  {
    int i;
    for (i = 0; argv[1][i] != '\0'; i++);
    if (i != TOTAL_NUM_DSPAL_TESTS)
    {
      fprintf(stderr, "Argument error: Test mask is not %d characters long\n\n", TOTAL_NUM_DSPAL_TESTS);
      test_mask_utils_print_help();
      return -1;
    }
    else
    {
      memcpy(test_mask, argv[1], i + 1);
    }
  }
  return 0;
}

int test_mask_utils_run_dspal_test(char** test_mask, int (*test_to_run)(void), char test_name[])
{
  int test_result = TEST_PASS;
  if (*(*test_mask) == '1')
  {
    test_result = display_test_results((*test_to_run)(), test_name);
  }

  (*test_mask)++;

  return test_result;
}
