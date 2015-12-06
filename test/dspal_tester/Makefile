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

all: build_adsp/libdspal_tester_skel.so build_apps/dspal_tester

stubs: dspal_tester_skel.c dspal_tester_stub.c

dspal_tester_skel.c dspal_tester_stub.c: dspal_tester.idl
	${HEXAGON_SDK_ROOT}/tools/qaic/Ubuntu14/qaic -m dll -I ${HEXAGON_SDK_ROOT}/inc/stddef $<

build_adsp/libdspal_tester_skel.so: stubs
	mkdir -p build_dsp && cd build_dsp && cmake -Wno-dev ../adsp_proc -DCMAKE_TOOLCHAIN_FILE=Toolchain-qurt.cmake
	cd build_adsp && make
	
build_apps/dspal_tester: stubs
	mkdir -p build_apps && cd build_apps && cmake -Wno-dev ../apps_proc -DCMAKE_TOOLCHAIN_FILE=Toolchain-arm-linux-gnueabihf.cmake
	cd build_apps && make

load:
	adb shell rm -f /usr/share/data/adsp/libdspal_tester_skel.so
	adb shell rm -f /home/linaro/libdspal_tester.so
	adb shell rm -f /home/linaro/dspal_tester
	adb shell rm -f /home/linaro/setup.sh
	adb push build_adsp/libdspal_tester_skel.so /usr/share/data/adsp
	adb push build_apps/libdspal_tester.so /home/linaro/libdspal_tester.so
	adb push build_apps/dspal_tester /home/linaro/dspal_tester
	adb push setup.sh /home/linaro/
	adb shell chmod 777 /home/linaro/dspal_tester
	adb shell sync
