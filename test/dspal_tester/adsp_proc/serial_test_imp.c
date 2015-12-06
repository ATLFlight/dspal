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
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <dev_fs_lib.h>
#include <dev_fs_lib_serial.h>

/* Enable medium level debugging. */
#define FARF_MEDIUM 1  /* 0 turns me off */
#include <HAP_farf.h>

#include "dspal_errno.h"

#define SERIAL_DEVICE_PATH_1 "/dev/tty-1"
#define SERIAL_DEVICE_PATH_2 "/dev/tty-2"
#define SERIAL_DEVICE_PATH_3 "/dev/tty-3"
#define SERIAL_DEVICE_PATH_4 "/dev/tty-4"
#define SERIAL_CYCLES 100
#define SERIAL_SIZE_OF_DATA_BUFFER 128
#define SERIAL_WRITE_TEST_DATA "0123456789"
#define SERIAL_WRITE_DELAY_IN_USECS (8000 * 10)
#define SERIAL_READ_DELAY_IN_USECS (8000 * 10)



/**
* @brief Test opening multiple serial device at the same time (having them open).
*
* @par Test:
* 1) Open the first serial device ('/dev/tty-1')
* 2) Open the second serial device ('/dev/tty-2')
* 3) Close the first serial device
* 4) Close the second serial device
*
* @return
* SUCCESS ------ Test Passes
* Error -------- Test Failed
*/
int dspal_tester_serial_multi_port(void)
{
   int serial_fildes_1 = 0;
   int serial_fildes_2 = 0;
   int serial_fildes_3 = 0;
   int serial_fildes_4 = 0;
   int result = SUCCESS;

   FARF(MEDIUM, "testing multi-port serial open for: %s", SERIAL_DEVICE_PATH_1);
   serial_fildes_1 = open(SERIAL_DEVICE_PATH_1, 0);
   if (serial_fildes_1 < SUCCESS)
   {
      FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_1);
      result = ERROR;
   }

   FARF(MEDIUM, "testing multi-port serial open for: %s", SERIAL_DEVICE_PATH_2);
   serial_fildes_2 = open(SERIAL_DEVICE_PATH_2, 0);
   if (serial_fildes_2 < SUCCESS)
   {
      FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_2);
      result = ERROR;
   }

   FARF(MEDIUM, "testing multi-port serial open for: %s", SERIAL_DEVICE_PATH_3);
   serial_fildes_3 = open(SERIAL_DEVICE_PATH_3, 0);
   if (serial_fildes_3 < SUCCESS)
   {
      FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_3);
      result = ERROR;
   }

   // FARF(MEDIUM, "testing multi-port serial open for: %s", SERIAL_DEVICE_PATH_4);
   // serial_fildes_4 = open(SERIAL_DEVICE_PATH_4, 0);
   // if (serial_fildes_4 < SUCCESS)
   // {
   //    FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_4);
   //    result = ERROR;
   // }

   if (serial_fildes_1 > 0)
   {
      close(serial_fildes_1);
   }
   if (serial_fildes_2 > 0)
   {
      close(serial_fildes_2);
   }
   if (serial_fildes_3 > 0)
   {
      close(serial_fildes_3);
   }
   if (serial_fildes_4 > 0)
   {
      close(serial_fildes_4);
   }

   if (result == ERROR)
   {
      FARF(HIGH, "error: serial multi-port open failed: %d", result);
   }
   else
   {
      FARF(HIGH, "success: multi-port open succeeded");
   }

   return result;
}

void multi_port_read_callback(void *context, char *buffer, size_t num_bytes)
{
   if (num_bytes > 0)
   {
      FARF(MEDIUM, "first 5 bytes of %d bytes: 0x%X,0x%X,0x%X,0x%X,0x%X",
            num_bytes,
            buffer[0],
            buffer[1],
            buffer[2],
            buffer[3],
            buffer[4]);
   }
   else
   {
      FARF(HIGH, "error: read callback with no data in the buffer");
   }
}

/**
* @brief Test multiple serial device at the same time for open,write,read,close.
*
* @par Test:
* 1) Open the serial device /dev/tty-1, /dev/tty-2, /dev/tty-3, /dev/tty-4
* 2) register read callback on four serial devices
* 3) write data to each ports
* 4) close all serial devices
*
* @return
* SUCCESS ------ Test Passes
* Error -------- Test Failed
*/
int dspal_tester_serial_multi_port_write_read(void)
{
   int serial_fildes_1 = 0;
   int serial_fildes_2 = 0;
   int serial_fildes_3 = 0;
   int serial_fildes_4 = 0;
   int result = SUCCESS;

   // open all 4 uart ports
   FARF(MEDIUM, "testing multi-port serial open for: %s", SERIAL_DEVICE_PATH_1);
   serial_fildes_1 = open(SERIAL_DEVICE_PATH_1, 0);
   if (serial_fildes_1 < SUCCESS)
   {
      FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_1);
      result = ERROR;
      goto exit;
   }

   FARF(MEDIUM, "testing multi-port serial open for: %s", SERIAL_DEVICE_PATH_2);
   serial_fildes_2 = open(SERIAL_DEVICE_PATH_2, 0);
   if (serial_fildes_2 < SUCCESS)
   {
      FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_2);
      result = ERROR;
      goto exit;
   }

   FARF(MEDIUM, "testing multi-port serial open for: %s", SERIAL_DEVICE_PATH_3);
   serial_fildes_3 = open(SERIAL_DEVICE_PATH_3, 0);
   if (serial_fildes_3 < SUCCESS)
   {
      FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_3);
      result = ERROR;
      goto exit;
   }

   // FARF(MEDIUM, "testing multi-port serial open for: %s", SERIAL_DEVICE_PATH_4);
   // serial_fildes_4 = open(SERIAL_DEVICE_PATH_4, 0);
   // if (serial_fildes_4 < SUCCESS)
   // {
   //    FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_4);
   //    result = ERROR;
   //    goto exit;
   // }

   // set read callback on all uart ports
   struct dspal_serial_ioctl_receive_data_callback receive_callback;
   receive_callback.context = NULL;
   receive_callback.rx_data_callback_func_ptr = multi_port_read_callback;
   result |= ioctl(serial_fildes_1, SERIAL_IOCTL_SET_RECEIVE_DATA_CALLBACK, (void *)&receive_callback);
   result |= ioctl(serial_fildes_2, SERIAL_IOCTL_SET_RECEIVE_DATA_CALLBACK, (void *)&receive_callback);
   result |= ioctl(serial_fildes_3, SERIAL_IOCTL_SET_RECEIVE_DATA_CALLBACK, (void *)&receive_callback);
   // result |= ioctl(serial_fildes_4, SERIAL_IOCTL_SET_RECEIVE_DATA_CALLBACK, (void *)&receive_callback);
   if (result < SUCCESS)
   {
      FARF(HIGH, "error: failed to set serial read callback on multi-ports");
      goto exit;
   }

   int num_bytes_written;
   const char *serial_buffer = "0123456789\n";
   for(int i = 0; i < 100; i++)
   {
      num_bytes_written =
            write(serial_fildes_1, (const char *)serial_buffer, strlen(serial_buffer));
      num_bytes_written =
            write(serial_fildes_2, (const char *)serial_buffer, strlen(serial_buffer));
      num_bytes_written =
            write(serial_fildes_3, (const char *)serial_buffer, strlen(serial_buffer));
      // num_bytes_written =
      //       write(serial_fildes_4, (const char *)serial_buffer, strlen(serial_buffer));
      FARF(MEDIUM, "number of bytes written: %d", num_bytes_written);
      usleep(SERIAL_WRITE_DELAY_IN_USECS);
   }

exit:
   if (serial_fildes_1 > 0)
   {
      close(serial_fildes_1);
   }
   if (serial_fildes_2 > 0)
   {
      close(serial_fildes_2);
   }
   if (serial_fildes_3 > 0)
   {
      close(serial_fildes_3);
   }
   if (serial_fildes_4 > 0)
   {
      close(serial_fildes_4);
   }

   return result;
}


/**
* @brief Read callback function.
*
* @param    context[in]    context of the callback (what object)
* @param    buffer[in]     data buffer
* @param    num_bytes[in]  number of bytes received by the callback
*
* @return
* SUCCESS ------ Test Passes
* Error -------- Test Failed
*/
void read_callback(void *context, char *buffer, size_t num_bytes)
{
   if (num_bytes > 0)
   {
      FARF(MEDIUM, "first 5 bytes of %d bytes received in callback buffer: 0x%X,0x%X,0x%X,0x%X,0x%X",
            num_bytes,
            buffer[0],
            buffer[1],
            buffer[2],
            buffer[3],
            buffer[4]);
   }
   else
   {
      FARF(HIGH, "error: read callback with no data in the buffer");
   }
}

/**
* @brief This test is a test of the serial read callback functionality
*
* @par Detailed Description:
* The serial bus has its RX and TX wired together so it can do a loop-back of the data.
* Data is sent over the bus and read back using a read callback function
*
* Test:
* 1) Open serial device (/dev/tty-1)
* 2) Setup the device to use a callback for the read (ioctl)
* 3) Write data to the serial device (data is read in read callback buffer)
* 4) Loop step 4 for SERIAL_CYCLES number of loops
* 5) close the serial device
*
* @return
* SUCCESS ------ Test Passes
* Error -------- Test Failed
*/
int dspal_tester_serial_read_callback(void)
{
   int serial_fildes = 0;
   int num_bytes_written = 0;
   int result = ERROR;
   int cycle_count;
   struct dspal_serial_ioctl_receive_data_callback receive_callback;
   char serial_buffer[SERIAL_SIZE_OF_DATA_BUFFER];

   FARF(MEDIUM, "testing serial open for: %s", SERIAL_DEVICE_PATH_1);
   serial_fildes = open(SERIAL_DEVICE_PATH_1, 0);
   if (serial_fildes < SUCCESS)
   {
      FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_1);
      result = ERROR;
      goto exit;
   }

   /* Load up the serial buffer just allocated with test data. */
   strncpy(serial_buffer, SERIAL_WRITE_TEST_DATA, sizeof(SERIAL_WRITE_TEST_DATA) - 1);

   /* Indicate that received data should be sent to a callback function. */
   receive_callback.context = NULL;
   receive_callback.rx_data_callback_func_ptr = read_callback;
   result = ioctl(serial_fildes, SERIAL_IOCTL_SET_RECEIVE_DATA_CALLBACK, (void *)&receive_callback);
   if (result < SUCCESS)
   {
      FARF(HIGH, "error: failed to set serial open options: %s", SERIAL_DEVICE_PATH_1);
      goto exit;
   }

   FARF(MEDIUM, "running serial callback test...");
   for (cycle_count = 0; cycle_count < SERIAL_CYCLES; cycle_count++)
   {
      num_bytes_written =
            write(serial_fildes, (char *)serial_buffer, sizeof(SERIAL_WRITE_TEST_DATA) - 1);
      FARF(MEDIUM, "number of bytes written: %d", num_bytes_written);
      usleep(SERIAL_READ_DELAY_IN_USECS);
   }
   FARF(MEDIUM, "serial callback test completed");
   result = SUCCESS;

exit:
   if (serial_fildes >= SUCCESS)
   {
      FARF(MEDIUM, "closing serial port");
      close(serial_fildes);
   }
   if (result != SUCCESS)
   {
      FARF(HIGH, "error: serial port read callback test failed");
   }

   return result;
}


/**
* @brief Test serial read and write functionality
*
* @par Detailed Description:
* The serial bus has its RX and TX wired together so it can do a loop-back of the data.
* Data is sent over the bus and read back using a read call
*
* Test:
* 1) Open serial device (/dev/tty-1)
* 2) Read data using read call
* 3) Write data to the serial device
* 4) Loop steps 3-4 for SERIAL_CYCLES number of loops
* 5) Close the serial device
*
* @return
* SUCCESS ------ Test Passes
* Error -------- Test Failed
*/
int dspal_tester_serial_write_read(void)
{
   int serial_fildes = 0;
   int cycle_count = 0;
   int num_bytes_written = 0;
   int num_bytes_read = 0;

   int result = ERROR;
   char serial_buffer[SERIAL_SIZE_OF_DATA_BUFFER];

   memset(serial_buffer, 0, sizeof(serial_buffer));
   FARF(MEDIUM, "testing serial open for: %s", SERIAL_DEVICE_PATH_1);
   serial_fildes = open(SERIAL_DEVICE_PATH_1, 0);
   if (serial_fildes < SUCCESS)
   {
      FARF(HIGH, "error: failed to open serial device path: %s", SERIAL_DEVICE_PATH_1);
      result = ERROR;
      goto exit;
   }

   /* Load up the serial buffer just allocated with test data. */
   strncpy((char *)serial_buffer, SERIAL_WRITE_TEST_DATA, sizeof(SERIAL_WRITE_TEST_DATA) - 1);

   /* Repeatedly make write calls to the serial port.  Test for possible DSM item overflow. */
   for (cycle_count = 0; cycle_count < SERIAL_CYCLES; cycle_count++)
   {
      FARF(MEDIUM, "cycle count: %d", cycle_count);
      num_bytes_read = read(serial_fildes, serial_buffer, sizeof(serial_buffer));
      FARF(MEDIUM, "number of bytes read: %d", num_bytes_read);
      num_bytes_written =
            write(serial_fildes, (char *)serial_buffer, sizeof(SERIAL_WRITE_TEST_DATA) - 1);
      FARF(MEDIUM, "number of bytes written: %d", num_bytes_written);

      /*
       * Display the data read, being sure to check if all of the data read has been displayed.
       */
      if (num_bytes_read > 0)
      {
         FARF(MEDIUM, "first 5 bytes of %d in buffer from read buffer: 0x%X,0x%X,0x%X,0x%X,0x%X",
               num_bytes_read,
               serial_buffer[0],
               serial_buffer[1],
               serial_buffer[2],
               serial_buffer[3],
               serial_buffer[4]);
      }

      FARF(MEDIUM, "delay before the next test cycle");
      usleep(SERIAL_WRITE_DELAY_IN_USECS);
   }
   result = SUCCESS;

exit:
   if (serial_fildes >= SUCCESS)
   {
      FARF(MEDIUM, "closing serial port");
      close(serial_fildes);
   }
   if (result != SUCCESS)
   {
      FARF(HIGH, "error: serial port read test failed");
   }

   return result;
}


/**
* @brief Runs all the serial tests and returns 1 aggregated result.
*
* @return
* SUCCESS ------ All tests pass
* ERROR -------- One or more tests failed
*/
int dspal_tester_serial_test(void)
{
   int result;

   FARF(MEDIUM, "beginning serial callback test");
   if ((result = dspal_tester_serial_read_callback()) < SUCCESS)
   {
      FARF(HIGH, "error: serial port callback test failed: %d", result);
      return result;
   }

   FARF(MEDIUM, "beginning serial write/read test");
   if ((result = dspal_tester_serial_write_read()) < SUCCESS)
   {
      FARF(HIGH, "error: serial port write test failed: %d", result);
      return result;
   }

   FARF(MEDIUM, "beginning serial multi-port test");
   if ((result = dspal_tester_serial_multi_port()) < SUCCESS)
   {
      FARF(HIGH, "error: serial multi-port test failed: %d", result);
      return result;
   }

   FARF(MEDIUM, "beginning serial multi-port read/write test");
   if ((result = dspal_tester_serial_multi_port_write_read()) < SUCCESS)
   {
      FARF(HIGH, "error: serial multi-port write read test failed: %d", result);
      return result;
   }

   FARF(MEDIUM, "serial port read/write test succeeded");
   return result;
}
