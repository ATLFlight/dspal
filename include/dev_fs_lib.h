/****************************************************************************
 * Copyright (c) 2015 Mark Charlebois. All rights reserved.
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
 * 3. Neither the name PX4 nor the names of its contributors may be
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

#pragma once

/**
 * @mainpage
 * This document describes the aDSP Abstraction Layer or DspAL (pronounced "dispal").  The
 * DspAL functions presented in this document are POSIX compliant and are implemented in the aDSP static
 * image.  DspAL functions may be called from code running in the static image or as an OpenDSP
 * dynamic library.
 *
 * - @subpage reading_writing_device_data
 * - @subpage sample_uart_code
 * - @subpage sample_i2c_code
 * - @subpage sample_spi_code
 *
 * @page reading_writing_device_data Reading and Writing Device Data
 * Reading and writing data to a device bus or port is accomplished using standard POSIX read/write
 * functions.  The format of the buffer containing the data to be read or written is an array of
 * bytes.
 *
 * @page sample_uart_code Sample UART Code
 * @par
 * Sample source code for read/write data to a serial port is included below:
 * @include serial_test_imp.c
 *
 * @page sample_i2c_code Sample I2C Code
 * @par
 * Sample source code for reading and writing to an I2C bus slave device is included below:
 * @include i2c_test_imp.c
 *
 * @page sample_spi_code Sample SPI Code
 * @par
 * Sample source code for reading and writing to a SPI bus slave device is included below:
 * @include spi_test_imp.c
 */

/**
 * @file
 * The declarations in this file are released to DspAL users and are used to
 * make file I/O call's with a device path.
 */

/**
 * Opens a device path in preparation to read/write raw data from I2C, SPI, and UART devices.
 * @par
 * Device path's are passed in the name parameter of the open (sys_open) function. The
 * following format is used to specify the bus type/number or port number.
 * @par
 * The string format defined for use in the device path is describe below:
 * /dev/{string}-{port/bus/device number}
 * @par
 * In the case of bus devices, the same bus device path may be opened to access different
 * peripheral slave devices.
 *
 * @param name
 * The device path of the bus or port to be opened, formatted as described above.
 *
 * @param mode
 * Current not used.
 *
 * @return
 * - {file descriptor}: The device was successfully opened and is ready for read/write access.
 * Use this return value in all other function calls requiring an "fd" parameter.
 * TODO: List error codes for all bus/port types.
 */
#define open sys_open
int sys_open(const char* name, unsigned int mode);

/**
 * Sends an IOCTL for a previous open bus/port device.
 * @param fd
 * File descriptor returned from the open function.
 * @param request
 * The numeric value of the IOCTL defined for the particular bus/port openened.  See
 * the def_fs_lib_{bus/port type}.h header file for a list of define IOCTL's.
 * @param argp
 * Parameter buffer defined for the particular IOCTL.
 * @return
 * - SUCCESS: The IOCTL was successfully processed.
 * TODO: List error codes for all bus/port types.
 */
#define ioctl sys_ioctl
int sys_ioctl(int fd, int request, void *argp);

/**
 * Requests that data be read from the bus/port device associated with the fd
 * parameter.  In the case of an I2C or SPI bus peripheral the ID of register
 * to be read must have been previously written.  See the sys_write function
 * below for additional information.
 * @par
 * An IOCTL function performing a combined read/write operation for I2C and
 * SPI bus peripherals is defined in the def_fs_lib_{bus/port type}.h header file.
 *
 * @param fd
 * File descriptor returned from the open function.
 * @param buffer
 * Pointer to a character array that will be used to store the accumulated data read
 * from the bus/port device.
 * @param count
 * The length in bytes of the buffer referenced in the buffer parameter.
 * @return
 * - The number of bytes copied to the buffer parameter.
 * - TODO: List error codes for all bus/port types.
 */
#define read sys_read
int sys_read(int fd, char* buffer, int count);

/**
 * Requests that data be written to the bus/port device associated with the
 * fd parameter.
 * @par Buffer Format for SPI and I2C Bus Devices:
 * - SPI and I2C Devices:
 * The first byte (or two bytes when addressing 16-bit peripheral devices) of
 * this buffer is the register ID on the peripheral device to be written.
 * @param fd
 * File descriptor returned from the open function.
 * @param buffer
 * Pointer to a character array containing the data to be written to the specified
 * bus/port device.  For I2C and SPI devices the device ID of the peripheral to
 * be written to is specified using the an IOCTL code.  See the dev_fs_lib_{bus-type}.h
 * files for more information.  Also, see the note above about how to specify the
 * ID of the register to be written.
 * @param count
 * @return
 */
#define write sys_write
int sys_write(int fd, char* buffer, int count);

/**
 * Close access to the particular port device or bus/peripheral device.
 * @param fd
 * File descriptor returned from the open function.
 * @return
 * - SUCCESS: The bus/port device was successfully closed.
 * - TODO: List error codes for all bus/port types.
 */
#define close sys_close
int sys_close(int fd);

#define DSPAL_USED_STATUS_COOKIE     "    USED" /**< indicates that the associated item is in use */
#define DSPAL_NOT_USED_STATUS_COOKIE "NOT USED" /**< indicates that the associated item is not in use */
