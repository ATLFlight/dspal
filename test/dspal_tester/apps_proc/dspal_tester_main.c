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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "adspmsgd.h"
#include "rpcmem.h"

#include "test_utils.h"
#include "dspal_tester.h"
#include "posix_test_suite.h"
#include "io_test_suite.h"
#include "test_mask_utils.h"

/**
 * @brief Runs all the tests, the io tests and the posix tests.
 *
 * @param   argc[in]    number of arguments
 * @param   argv[in]    array of parameters (each is a char array)
 *
 * @return
 * TEST_PASS ------ All tests passed
 * TEST_FAIL ------ A test has failed
*/

int main(int argc, char *argv[])
{
	int status = TEST_PASS;

	LOG_INFO("");

	char test_mask[255];
	if (test_mask_utils_process_cli_args(argc, argv, test_mask) != 0)
	{
		return 0;
	}

	LOG_INFO("Starting DSPAL tests");

#if defined(DSP_TYPE_ADSP) // TODO: to be enabled after the API is supported on SLPI
	dspal_tester_test_dspal_get_version_info();
#endif
	status = run_posix_test_suite(test_mask);
	status |= run_io_test_suite(test_mask + NUM_DSPAL_POSIX_TESTS);

	if ((status & TEST_FAIL) == TEST_FAIL) {
		LOG_INFO("DSPAL test failed.");

	} else {
		if ((status & TEST_SKIP) == TEST_SKIP) {
			LOG_INFO("DSPAL some tests skipped.");
		}

		LOG_INFO("DSPAL tests succeeded.");
	}

	LOG_INFO("");
	return status;
}

