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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <dspal_time.h>
#include <dev_fs_lib.h>

#include "test_utils.h"
#include "dspal_tester.h"

#define TEST_FILE_PATH  "/dev/fs/test.txt"


/**
* @brief Test to see if a file can be opened
*
* @par Detailed Description:
* This tests opens a file from the adsp side ('/dev/fs/test.txt').  It then
* tries to open the file without the dspal file prefix ('test.txt').

* Test:
* 1) Opens file with dspal path prefix ('/dev/fs/test.txt') in read_only mode
* 2) Close that file
* 3) Open file without dispal path prefix ('test.txt') in read_only mode
* 4) Close that file
*
* @return
* TEST_PASS ------ Always
*/
int dspal_tester_test_posix_file_open(void)
{
   int fd;
   fd = open(TEST_FILE_PATH, O_RDWR);

   if (fd == -1)
   {
      FAIL("open test.txt failed. Make sure to have test.txt at $ADSP_LIBRARY_PATH");
   }

   close(fd);

   // test open file path without dspal file path prefix
   fd = open("test.txt", O_RDONLY);

   if (fd == -1)
   {
      FAIL("open test.txt failed. Make sure to have test.txt at $ADSP_LIBRARY_PATH");
   }

   close(fd);

   return TEST_PASS;
}

/**
* @brief Test to see if a file can be opened and then closed
*
* @par
* Test:
* 1) Opens file with dspal path prefix ('/dev/fs/test.txt') in read only mode
* 2) Close that file
*
* @return
* TEST_PASS ------ Always
*/
int dspal_tester_test_posix_file_close(void)
{
   int fd;
   fd = open(TEST_FILE_PATH, O_RDONLY);

   if (fd == -1)
   {
      FAIL("open test.txt failed. Make sure to have test.txt at $ADSP_LIBRARY_PATH");
   }

   if (close(fd) != 0)
   {
      FAIL("close test.txt failed.");
   }

   return TEST_PASS;
}


/**
* @brief Test to see if a file can be opened, read fully and then closed
*
* @par
* Test:
* 1) Opens file with dspal path prefix ('/dev/fs/test.txt') in read_only mode
* 2) Read all the bytes in the file (printing to console)
* 3) Close the file
*
* @return
* TEST_PASS ------ Always
*/
int dspal_tester_test_posix_file_read(void)
{
   int fd;
   fd = open(TEST_FILE_PATH, O_RDONLY);
   uint8_t c;
   int bytes_read;
   int cnt = 0;

   if (fd == -1)
   {
      FAIL("open test.txt failed. Make sure to have test.txt at $ADSP_LIBRARY_PATH");
   }

   MSG("reading test.txt:");
   while(1) {
      bytes_read = read(fd, &c, 1);
      if (bytes_read == 0) {
         break;
      } else if (bytes_read == 1) {
         MSG("byte %d: %c", cnt++, c);
      } else {
         FAIL("read test.txt failed.");
      }
   }

   close(fd);

   return TEST_PASS;
}

/**
* @brief Test to see if a file can be opened, written to and then closed
*
* @par
* Test:
* 1) Opens file with dspal path prefix ('/dev/fs/test.txt') in read/write mode with append
* 2) Write some data (time stamp) to the file
* 3) Close the file
*
* @return
* TEST_PASS ------ Always
*/
int dspal_tester_test_posix_file_write(void)
{
   int fd;
   fd = open(TEST_FILE_PATH, O_RDWR | O_APPEND);
   char buffer[50] = {0};
   uint64_t timestamp = time(NULL);

   if (fd == -1)
   {
      FAIL("open test.txt failed. Make sure to have test.txt at $ADSP_LIBRARY_PATH");
   }

   sprintf(buffer, "test - timestamp: %llu\n", timestamp);
   MSG("writing to test.txt: %s (len: %d)", buffer, strlen(buffer) + 1);

   if (write(fd, buffer, strlen(buffer)+1) != (ssize_t)(strlen(buffer) + 1))
   {
      FAIL("write test.txt failed.");
   }

   close(fd);

   return TEST_PASS;
}


/**
* @brief Test to see if ioctl fails (ioctl is not supported for files)
*
* @par
* Test:
* 1) Opens file with dspal path prefix ('/dev/fs/test.txt') in read/write mode with append
* 2) Try ioctl call and make sure it fails
* 3) Close the file
*
* @return
* TEST_PASS ------ Always
*/
int dspal_tester_test_posix_file_ioctl(void)
{
   int fd;
   fd = open(TEST_FILE_PATH, O_RDWR | O_APPEND);

   if (fd == -1)
   {
      FAIL("open test.txt failed. Make sure to have test.txt at $ADSP_LIBRARY_PATH");
   }

   if (ioctl(fd, 0, NULL) != -1)
   {
      FAIL("ioctl() is not supported and should have returned -1.");
   }

   close(fd);

   return TEST_PASS;
}

/**
* @brief Test to remove the specified file
*
* @par
* Test:
* 1) Opens file with dspal path prefix ('/dev/fs/test.txt') and create it if
*    it does not exist
* 2) close the file
* 3) remove the file
*
* @return
* TEST_PASS ------ if remove returns 0
* TEST FAIL ------ on error
*/
int dspal_tester_test_posix_file_remove(void)
{
   int fd;

   // First create the file if it does not exist yet
   fd = open(TEST_FILE_PATH, O_RDWR);

   if (fd == -1)
   {
      FAIL("open test.txt failed. Make sure to have test.txt at $ADSP_LIBRARY_PATH");
   }

   close(fd);

   if (remove(TEST_FILE_PATH) != 0)
   {
      FARF(ALWAYS, "failed to remove %s", TEST_FILE_PATH);
      return TEST_FAIL;
   }

   // test removing a file with invalid dspal path
   if (remove("test.txt") == 0)
   {
      FARF(ALWAYS, "removed %s. This shouldn't happen", TEST_FILE_PATH);
      return TEST_FAIL;
   }

   return TEST_PASS;
}

/**
* @brief Test to a file can be opened in all supported modes and closed.
*
* @par
* Test:
* 1) Opens file with dspal path prefix ('/dev/fs/test.txt') in one of the
*    fopen supported modes
* 2) Close the file
*
* @return
* TEST_FAIL ------ if fopen failed on certain mode
* TEST_PASS ------ if fopen succeeds on all modes
*/
int dspal_tester_test_fopen_fclose(void)
{
   FILE *fd;
   const char *modes[] = {
      "w", "w+", "r", "r+", "a", "a+"
   };
   int num_modes = sizeof(modes) / sizeof(const char *);

   for (int i = 0; i < num_modes; i++)
   {
      fd = fopen(TEST_FILE_PATH, modes[i]);
      if (fd == NULL)
      {
         FARF(ALWAYS, "fopen() mode %s returned NULL", modes[i]);
         return TEST_FAIL;
      }
      fclose(fd);
      FARF(ALWAYS, "fopen()/fclose mode %s succ", modes[i]);
   }

   FARF(ALWAYS, "fopen_fclose test passed");

   return TEST_PASS;
}

/**
* @brief Test to a file can be written and read using fwrite() and frea()
*
* @par
* Test:
* 1) Opens file with dspal path prefix ('/dev/fs/test.txt') in write mode
* 2) write timestamp to file
* 3) fflush and fclose the file
* 4) open the file in r mode
* 5) fread() the file content and compare the bytes read and bytes written
* 6) Close the file
*
* @return
* TEST_PASS ------ if fwrite and fread succeed, and the bytes read and bytes
*                  written are identical.
* TEST_FAIL ------ otherwise
*/
int dspal_tester_test_fwrite_fread(void)
{
   FILE *fd;
   char buffer[50] = {0};
   uint64_t timestamp = time(NULL);
   size_t bytes_written;
   size_t bytes_read;
   size_t buffer_len;

   fd = fopen(TEST_FILE_PATH, "w");
   if (fd == NULL)
   {
      FARF(ALWAYS, "fopen() mode w returned NULL");
      return TEST_FAIL;
   }

   sprintf(buffer, "test - timestamp: %llu\n", timestamp);
   buffer_len = strlen(buffer) + 1;

   MSG("writing to test.txt: %s (len: %d)", buffer, buffer_len);

   bytes_written = fwrite(buffer, 1, buffer_len, fd);
   if (bytes_written != buffer_len)
   {
      FARF(ALWAYS, "fwrite() %d bytes returned less than expected %d",
           buffer_len, bytes_written);
      return TEST_FAIL;
   }

   fflush(fd);
   fclose(fd);

   fd = fopen(TEST_FILE_PATH, "r");
   if (fd == NULL)
   {
      FARF(ALWAYS, "fopen() mode r returned NULL");
      return TEST_FAIL;
   }

   memset(buffer, 0, 50);
   bytes_read = fread(buffer, 1, buffer_len, fd);
   if (bytes_read != buffer_len)
   {
      FARF(ALWAYS, "fread() %d bytes returned less than expected %d",
           buffer_len, bytes_read);
      return TEST_FAIL;
   }

   FARF(ALWAYS, "fread() %d bytes: %s", bytes_read, buffer);

   fclose(fd);

   FARF(ALWAYS, "fwrite_fread test passed");

   return TEST_PASS;
}
