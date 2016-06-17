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

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <dev_fs_lib_i2c.h>
#include <test_status.h>

#include "test_utils.h"

/**
* @brief Test to see i2c device can be opened and configured.
*
* @par
* Test:
* 1) Open the i2c device (/dev/i2c-3) 
* 2) Configure the i2c device to have (using ioctl):
*     -Slave address: 0x77
*     -Bus Frequency in khz: 400
*     -Transfer timeout in usec: 9000
* 3) RDWR Operation to get sensor ID - register 0x0D
* 4) Write operation for further calibration data reading - 
* register 0xAA 
* 5) Read 2 bytes of calibration data 
* 6) Close the i2c device 
*
* @return
* SUCCESS ------ Test Passes
* ERROR ------ Test Failed
*/
int dspal_tester_i2c_test(void)
{
	int ret = SUCCESS;
    uint8_t buf[2]; 
	/*
	 * Open i2c device
	 */
	int fd = -1;
	fd = open("/dev/i2c-2", 0);

	if (fd > 0) {
		/*
		 * Configure I2C device
		 */
		struct dspal_i2c_ioctl_slave_config slave_config;
		slave_config.slave_address = 0x76;
		slave_config.bus_frequency_in_khz = 400;
		slave_config.byte_transer_timeout_in_usecs = 9000;

		if (ioctl(fd, I2C_IOCTL_CONFIG, &slave_config) != 0) {
			ret = ERROR;
		}

        struct dspal_i2c_ioctl_combined_write_read ioctl_write_read;
        uint8_t write_buffer[1];
        /* Save the address of the register to read from in the write buffer for the combined write. */
        write_buffer[0] = 0xD0;
        ioctl_write_read.write_buf     = write_buffer;
        ioctl_write_read.write_buf_len = 1;
        ioctl_write_read.read_buf      = &buf[0];
        ioctl_write_read.read_buf_len  = 1;

        uint8_t byte_count = ioctl(fd, I2C_IOCTL_RDWR, &ioctl_write_read);
        if ( byte_count != 1) {
             ret = ERROR;
        }
        LOG_ERR("Sensor id register 0xD0 write/read 0x%x", buf[0]);

        uint8_t reg = 0xAA; 
        if (ioctl(fd, I2C_IOCTL_WRITE_REG, &reg) != 0) {
                ret = ERROR;
        }

        struct dspal_i2c_ioctl_read read_params; 
        read_params.read_buf = &buf[0]; 
        read_params.read_buf_len = 2; 
        if (ioctl(fd, I2C_IOCTL_READ, &read_params) != 0) {
                 ret = ERROR;
        }
       
        LOG_ERR("Calibration data register 0xAA read 2 bytes 0x%x 0x%x", buf[0], buf[1]);

		/*
		 * Close the device ID
		 */
		close(fd);

	} else {
		ret = ERROR;
	}

	return ret;
}
