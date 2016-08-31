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

#ifndef TEST_MASK_UTILS_H_
#define TEST_MASK_UTILS_H_

#define NUM_DSPAL_POSIX_TESTS (32)
#define NUM_DSPAL_IO_TESTS (21)
#define TOTAL_NUM_DSPAL_TESTS (NUM_DSPAL_POSIX_TESTS + NUM_DSPAL_IO_TESTS)

/**
 * @brief Prints help information.
 */
void test_mask_utils_print_help();

/**
 * @brief Processes command line arguments.
 *
 * @param   argc[in]         number of arguments
 * @param   argv[in]         array of parameters (each is a char array)
 * @param   test_mask[out]   output test mask from cli arguments
 * @return 0, Do not exit immediately.
 */
int test_mask_utils_process_cli_args(int argc, char* argv[], char test_mask[]);

/**
 * @brief Runs the provided test if its enabled in the test mask
 *
 * @param test_mask Mask of which tests are to be run
 * @param test_to_run Pointer to the test function to be run
 * @param test_name Human-readable test name
 * @return Return value of the test function if it is run
 */
int test_mask_utils_run_dspal_test(char** test_mask, int (*test_to_run)(void), char test_name[]);

#endif /* TEST_MASK_UTILS_H_ */
