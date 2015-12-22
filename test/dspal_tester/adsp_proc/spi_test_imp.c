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
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdbool.h>
#include <dev_fs_lib_spi.h>
#include <dspal_errno.h>

/* Enable medium level debugging. */
#define FARF_HIGH   1    /* Use a value of 0 to disable the specified debug level. */
#define FARF_MEDIUM 1
#define FARF_LOW    1

#include <HAP_farf.h>

#define SPI_DEVICE_PATH "/dev/spi-8"
#define SPI_TEST_CYCLES 10

/**
 * NOTE: DO NOT send more than 64 bytes in loopback test. SPI bus automatically
 * switches to DMA mode to send more than 64 bytes. However, DMA mode in
 * loopback transfer results in system crash or hang. Transfering more than 64
 * bytes to/from peripheral device using DMA mode is supported.
 */
#define SPI_LOOPBACK_TEST_TRANSMIT_BUFFER_LENGTH  20

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

void init_write_buffer(uint8_t *buffer, int length)
{
   int i;
   char c = 'a';

   for (i = 0; i < length; i++)
   {
      buffer[i] = c;
      if (c == 'z')
         c = 'a';
      c++;
   }
}

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
int dspal_tester_spi_loopback_test(void)
{
   int spi_fildes = SUCCESS;
   int cycle_count;
   int result = SUCCESS;
   uint8_t write_data_buffer[SPI_LOOPBACK_TEST_TRANSMIT_BUFFER_LENGTH];
   uint8_t read_data_buffer[SPI_LOOPBACK_TEST_TRANSMIT_BUFFER_LENGTH];
   int test_data_length_in_bytes = SPI_LOOPBACK_TEST_TRANSMIT_BUFFER_LENGTH - 1;
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
   write_data_buffer[SPI_LOOPBACK_TEST_TRANSMIT_BUFFER_LENGTH-1] = 0;
   init_write_buffer(write_data_buffer, SPI_LOOPBACK_TEST_TRANSMIT_BUFFER_LENGTH-1);

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
    * Test loopback mode using combined read/write mode.
    */
   FARF(MEDIUM, "testing spi write/read for %d cycles", SPI_TEST_CYCLES);
   for (cycle_count = 0; cycle_count < SPI_TEST_CYCLES; cycle_count++)
   {
      memset(read_data_buffer, 0, sizeof(read_data_buffer));
      read_write.read_buffer = &read_data_buffer[0];
      read_write.read_buffer_length = test_data_length_in_bytes;
      read_write.write_buffer = &write_data_buffer[0];
      read_write.write_buffer_length = test_data_length_in_bytes;

      FARF(MEDIUM, "writing bytes: (%d bytes)",
           test_data_length_in_bytes);

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
   }

   result = SUCCESS;
   FARF(MEDIUM, "SPI lookback test passed");

exit:
   if (spi_fildes > SUCCESS)
   {
      close(spi_fildes);
   }

   return result;
}

int dspal_tester_spi_exceed_max_length_test(void)
{
   int spi_fildes = SUCCESS;
   int result = SUCCESS;
   uint8_t write_data_buffer[DSPAL_SPI_TRANSMIT_BUFFER_LENGTH + 1];
   uint8_t read_data_buffer[DSPAL_SPI_RECEIVE_BUFFER_LENGTH + 1];
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

   read_write.read_buffer = &read_data_buffer[0];
   read_write.read_buffer_length = sizeof(read_data_buffer);
   read_write.write_buffer = &write_data_buffer[0];
   read_write.write_buffer_length = sizeof(write_data_buffer);
   result = ioctl(spi_fildes, SPI_IOCTL_RDWR, &read_write);
   if (result == SUCCESS)
   {
      FARF(ALWAYS, "error: SPI_IOCTL_RDWR transfer overly large data should "
           "have failed but didn't. ");
      goto exit;
   }
   result = SUCCESS;
   FARF(MEDIUM, "SPI exceed max write length test passed");

exit:
   if (spi_fildes > SUCCESS)
   {
      close(spi_fildes);
   }

   return result;
}

/**
 * Main entry point for the SPI automated test.
 * @return
 * - ERROR: Indicates that the test has failed.
 * - SUCCESS: Test has passed
 */
int dspal_tester_spi_test(void)
{
   int result;

   FARF(ALWAYS, "beginning spi loopback test");
   if ((result = dspal_tester_spi_loopback_test()) < SUCCESS)
   {
      FARF(ALWAYS, "error: spi loopback test failed: %d", result);
      return result;
   }

   FARF(ALWAYS, "beginning spi exceed max write length test");
   if ((result = dspal_tester_spi_exceed_max_length_test()) < SUCCESS)
   {
      FARF(ALWAYS, "error: spi exceed max write length test failed: %d", result);
      return result;
   }

   return SUCCESS;
}
