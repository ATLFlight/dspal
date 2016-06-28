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

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <dev_fs_lib_pwm.h>
#include <test_status.h>

/**
* @brief
* Test to define PWM signals at a specified period and vary the pulse width.
*
* @par
* Test:
* 1) Open the PWM device (/dev/pwm-1)
* 2) Configure the
*     GPIO 5 (pin 3, J9 on the Eagle Board)
*
* 3) Close the PWM device
*
* @return
* SUCCESS ------ Test Passes
* ERROR ------ Test Failed
*/

#define PWM_TEST_PULSE_WIDTH_INCREMENTS 150

int dspal_tester_pwm_test(void)
{
	int ret = SUCCESS;
	int pulse_width;
	/*
	 * Open PWM device
	 */
	int fd = -1;
	fd = open("/dev/pwm-1", 0);

	if (fd > 0) {
		/*
		 * Configure PWM device
		 */
		struct dspal_pwm pwm_gpio[2];
		struct dspal_pwm_ioctl_signal_definition signal_definition;
		struct dspal_pwm_ioctl_update_buffer *update_buffer;
		struct dspal_pwm *pwm;

		pwm_gpio[0].gpio_id = 5;
		pwm_gpio[0].pulse_width_in_usecs = 100;
		pwm_gpio[1].gpio_id = 6;
		pwm_gpio[1].pulse_width_in_usecs = 100;

		signal_definition.num_gpios = 2;
		signal_definition.period_in_usecs = 1000;
		signal_definition.pwm_signal = &pwm_gpio[0];

		if (ioctl(fd, PWM_IOCTL_SIGNAL_DEFINITION, &signal_definition) != 0) {
			ret = ERROR;
		}

		if (ioctl(fd, PWM_IOCTL_GET_UPDATE_BUFFER, &update_buffer) != 0)
		{
			ret = ERROR;
		}
		pwm = &update_buffer->pwm_signal[0];

		while (TRUE)
		{
			for (pulse_width = PWM_TEST_PULSE_WIDTH_INCREMENTS; pulse_width < (int)signal_definition.period_in_usecs; pulse_width += PWM_TEST_PULSE_WIDTH_INCREMENTS)
			{
				pwm[0].pulse_width_in_usecs = pulse_width;
				pwm[1].pulse_width_in_usecs = pulse_width;
				usleep(1000 * 1);
			}

			for (pulse_width = signal_definition.period_in_usecs - PWM_TEST_PULSE_WIDTH_INCREMENTS; pulse_width > 0; pulse_width -= PWM_TEST_PULSE_WIDTH_INCREMENTS)
			{
				pwm[0].pulse_width_in_usecs = pulse_width;
				pwm[1].pulse_width_in_usecs = pulse_width;
				usleep(1000 * 1);
			}
		}

		/*
		 * Close the device ID
		 */
		// TODO-JYW: This does not appear to be working.
		// close(fd);
	} else {
		ret = ERROR;
	}

	return ret;
}
