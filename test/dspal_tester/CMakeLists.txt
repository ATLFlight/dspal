############################################################################
#
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
# 3. Neither the name PX4 nor the names of its contributors may be
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
#
############################################################################

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(version_major 0)
set(version_minor 1)
set(version_patch 0)
set(version "${version_major}.${version_minor}.${version_patch}")
set(package-contact "charlebm@gmail.com")

include(${CMAKE_TOOLCHAIN_FILE})
if ("$ENV{HEXAGON_TOOLS_ROOT}" STREQUAL "")
	message( FATAL_ERROR "HEXAGON_TOOLS_ROOT not set")
endif()

if ("$ENV{HEXAGON_SDK_ROOT}" STREQUAL "")
	message( FATAL_ERROR "HEXAGON_SDK_ROOT not set")
endif()

set(HEXAGON_TOOLS_ROOT $ENV{HEXAGON_TOOLS_ROOT})
set(HEXAGON_SDK_ROOT $ENV{HEXAGON_SDK_ROOT})

include_directories(
	${HEXAGON_TOOLS_ROOT}/Tools/target/hexagon/include
	${HEXAGON_SDK_ROOT}/inc/stddef
	${HEXAGON_SDK_ROOT}/lib/common/remote/ship/hexagon_Debug
	${CMAKE_SOURCE_DIR}/../include
	include
	)

message(STATUS "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")

set(CMAKE_SHARED_LIB_FLAGS "-L${HEXAGON_SDK_ROOT}/lib/common/remote/ship/hexagon_Debug -l adsprpc")

add_subdirectory(src/adsp_proc)
add_subdirectory(src/apps_proc)
add_subdirectory(src/common)

# vim: set noet fenc=utf-8 ff=unix ft=cmake :
