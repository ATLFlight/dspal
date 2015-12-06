/****************************************************************************
 *   Copyright (c) 2015 Mark Charlebois. All rights reserved.
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

#ifndef DEV_FS_LIB_H_
#define DEV_FS_LIB_H_

/**
 * @file
 * The declarations in this file are for the DspAL internal implementation only.
 * DspAL users shall never directly call these functions!
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * The maximum length of the device path used when naming the port or bus resources to be
 * opened.
 */
#define MAX_LEN_DEVICE_PATH_IN_BYTES 32

int sys_open(const char* name, unsigned int mode);

int sys_ioctl(int fd, int request, void *argp);

int sys_read(int fd, char* buffer, int count);

int sys_write(int fd, const char* buffer, int count);

int sys_close(int fd);

#define DSPAL_USED_STATUS_COOKIE     "    USED" /**< indicates that the associated item is in use */
#define DSPAL_NOT_USED_STATUS_COOKIE "NOT USED" /**< indicates that the associated item is not in use */

#ifdef __cplusplus
}
#endif

#endif /* DEV_FS_LIB_H_ */
