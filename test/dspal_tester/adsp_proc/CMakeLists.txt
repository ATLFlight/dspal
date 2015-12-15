############################################################################
# Copyright (c) 2015 Mark Charlebois. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name ATLFlight nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
############################################################################

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include(../cmake/common.cmake)

include_directories(
	${HEXAGON_SDK_ROOT}/lib/common/rpcmem
	${HEXAGON_SDK_ROOT}/inc/stddef
	${HEXAGON_SDK_ROOT}/lib/common/remote/ship/hexagon_Debug
	${HEXAGON_SDK_ROOT}/inc
	../../../include
	../include
	..
	)

set(CMAKE_SHARED_LIB_FLAGS "-L${HEXAGON_SDK_ROOT}/lib/common/remote/ship/hexagon_Debug -l adsprpc")

add_library(dspal_tester SHARED
	gpio_test_imp.c
	i2c_test_imp.c
	posix_file_tests.c
	posix_pthread_tests.c
	posix_semaphore_tests.c
	posix_time_tests.c
	serial_test_imp.c
	spi_test_imp.c
	../common/test_utils.c
	)

add_library(dspal_tester_skel SHARED
	../dspal_tester_skel.c
	)

target_link_libraries(dspal_tester_skel dspal_tester)

# vim: set noet fenc=utf-8 ff=unix ft=cmake :
