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
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <dev_fs_lib.h>
#include <dev_fs_lib_spi.h>
#include <dspal_errno.h>

/* Enable medium level debugging. */
#define FARF_HIGH   1    /* Use a value of 0 to disable the specified debug level. */
#define FARF_MEDIUM 1
#define FARF_LOW    1

#include <HAP_farf.h>

#define SPI_DEVICE_PATH "/dev/spi-8"
#define SPI_TEST_CYCLES 10
#define SPI_WRITE_TEST_DATA "01234"
#define SPI_SIZE_OF_DATA_BUFFER (sizeof(SPI_WRITE_TEST_DATA) - 1)

/**
 * @brief Helper function  for 'dspal_tester_spi_test', checks if 2 data buffers are equal.
 *
 *
 * @param buffer1[in]  pointer to first buffer 
 * @param buffer2[in]  pointer to second buffer 
 * @param length[in]   length of each buffers 
 *
 * @return
 * true  ------ data buffers match
 * false ------ data buffers do not match
*/
bool dpsal_tester_is_memory_matching(uint8_t *buffer1, uint8_t *buffer2, int length)
{
   if (memcmp(buffer1, buffer2, length) != 0)
   {
      FARF(ALWAYS, "error: the bytes read to not match the bytes written");
      FARF(ALWAYS, "bytes read: %c, %c, %c, %c, %c", buffer1[0], buffer1[1], buffer1[2],
            buffer1[3], buffer1[4]);
      FARF(ALWAYS, "bytes written: %c, %c, %c, %c, %c", buffer2[0], buffer2[1], buffer2[2],
            buffer2[3], buffer2[4]);
      return false;
   }

   return true;
}

/**
 * Main entry point for the SPI automated test.
 * @return
 * - ERROR: Indicates that the test has failed.
 * - SUCCESS: Test has passed
 */

 /**
 * @brief Test read/write functionality of spi by using loopback
 * 
 * @par Detailed Description:
 * Tests the read and write functionality of the spi device by putting the device
 * in loopback mode.  This is tested in 2 ways: writing the data then reading it 
 * back from the read buffer or by doing the read/write at the same time using ioctl
 *
 * Test:
 * 1) Opens file for spi device ('/dev/spi-8')
 * 2) Sets up the spi device in loopback mode using ioctl
 * 3) Write to the spi bus
 * 4) Read from the spi bus buffer 
 * 5) Commented Out ---- Check if data written matches data read
 * 6) Loop though steps 4-5 for  SPI_TEST_CYCLES number of cycles
 * 7) So ioctl read/write operation and check if data written matches data read
 * 8) Close spi bus 
 *
 * @return
 * SUCCESS  ------ Test Passes
 * ERROR ------ Test Failed
*/
int dspal_tester_spi_test(void)
{
   int spi_fildes = SUCCESS;
   int cycle_count;
   int result = SUCCESS;
   uint8_t write_data_buffer[SPI_SIZE_OF_DATA_BUFFER];
   uint8_t read_data_buffer[SPI_SIZE_OF_DATA_BUFFER];
   int test_data_length_in_bytes = SPI_SIZE_OF_DATA_BUFFER;
   struct dspal_spi_ioctl_loopback loopback;
   struct dspal_spi_ioctl_read_write read_write;

   FARF(MEDIUM, "testing spi open for: %s", SPI_DEVICE_PATH);
   spi_fildes = open(SPI_DEVICE_PATH, 0);
   if (spi_fildes < SUCCESS)
   {
      FARF(HIGH, "error: failed to open spi device path: %s", SPI_DEVICE_PATH);
      result = ERROR;
      goto exit;
   }

   /*
    * Initialize the write buffers in preparation for a read/write sequence.
    */
   memcpy(write_data_buffer, SPI_WRITE_TEST_DATA, test_data_length_in_bytes);

   /*
    * Enable loopback mode to allow write/reads to be tested internally.
    */
   FARF(MEDIUM, "enabling spi loopback mode");
   loopback.state = SPI_LOOPBACK_STATE_ENABLED;
   result = ioctl(spi_fildes, SPI_IOCTL_LOOPBACK_TEST, &loopback);
   if (result < SUCCESS)
   {
      FARF(HIGH, "error: unable to activate spi loopback mode");
      goto exit;
   }

   /*
    * Test individual read/write operations.
    */
   FARF(MEDIUM, "testing spi write/read for %d cycles", SPI_TEST_CYCLES);
   for (cycle_count = 0; cycle_count < SPI_TEST_CYCLES; cycle_count++)
   {
      int num_bytes_read, num_bytes_written;

      /*
       * Write the data over SPI.
       */
      FARF(MEDIUM, "spi writing bytes: %s (%d bytes)", SPI_WRITE_TEST_DATA, test_data_length_in_bytes);
      num_bytes_written = write(spi_fildes, (char *)write_data_buffer, test_data_length_in_bytes);
      if (num_bytes_written < test_data_length_in_bytes)
      {
         FARF(ALWAYS, "error: failed to write the expected number of bytes: %d", sizeof(write_data_buffer));
         result = ERROR;
         goto exit;
      }

      /*
       * Read the spi buffer to see if the expected bytes written above were received.
       */
      memset(read_data_buffer, 0, sizeof(read_data_buffer));
      FARF(MEDIUM, "spi reading bytes");
      num_bytes_read = read(spi_fildes, (char *)read_data_buffer, test_data_length_in_bytes);
      FARF(MEDIUM, "read number of bytes: %d", num_bytes_read);

// TODO-JYW: TESTING-TESTING
//      /*
//       * Verify the that bytes read match those that were written.
//       */
//      if (!dpsal_tester_is_memory_matching(write_data_buffer, read_data_buffer, num_bytes_written))
//      {
//         FARF(ALWAYS, "error: read/write memory buffers do not match");
//         goto exit;
//      }
   }

   /*
    * Test the combined read/write operations.
    */
   memset(read_data_buffer, 0, sizeof(read_data_buffer));
   read_write.read_buffer = &read_data_buffer[0];
   read_write.read_buffer_length = sizeof(read_data_buffer);
   read_write.write_buffer = &write_data_buffer[0];
   read_write.write_buffer_length = test_data_length_in_bytes;
   result = ioctl(spi_fildes, SPI_IOCTL_RDWR, &read_write);
   if (result < SUCCESS)
   {
      FARF(ALWAYS, "error: unable to activate read/write ioctl");
      goto exit;
   }

   if (!dpsal_tester_is_memory_matching(write_data_buffer, read_data_buffer, test_data_length_in_bytes))
   {
      FARF(ALWAYS, "error: read/write memory buffers do not match");
      goto exit;
   }
   FARF(MEDIUM, "written data matches read data");
   result = SUCCESS;

exit:
   if (spi_fildes > SUCCESS)
   {
      close(spi_fildes);
   }

   return result;
}





