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
# 3. Neither the name ATLFLight nor the names of its contributors may be
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

all: build_apps_targets build_adsp_targets

.PHONY: ENV_VARS
ENV_VARS:
	@[ ! -z "${HEXAGON_SDK_ROOT}" ] || (echo "HEXAGON_SDK_ROOT not set" && false)
	@[ ! -z "${HEXAGON_TOOLS_ROOT}" ] || (echo "HEXAGON_TOOLS_ROOT not set" && false)

.PHONY: stubs
stubs: dspal_tester_skel.c dspal_tester_stub.c

dspal_tester_skel.c dspal_tester_stub.c: dspal_tester.idl ENV_VARS
	${HEXAGON_SDK_ROOT}/tools/qaic/Ubuntu14/qaic -m dll -I ${HEXAGON_SDK_ROOT}/inc/stddef $<

.PHONY: build_adsp_targets
build_adsp_targets: build_dsp/libdspal_tester_skel.so build_dsp/libdspal_tester.so

build_dsp/libdspal_tester_skel.so build_dsp/libdspal_tester.so: stubs ENV_VARS
	mkdir -p build_dsp && cd build_dsp && cmake -Wno-dev ../adsp_proc -DCMAKE_TOOLCHAIN_FILE=Toolchain-qurt.cmake
	cd build_dsp && make
	
.PHONY: build_apps_targets
build_apps_targets: build_apps/dspal_tester

build_apps/dspal_tester: stubs ENV_VARS
	mkdir -p build_apps && cd build_apps && cmake -Wno-dev ../apps_proc -DCMAKE_TOOLCHAIN_FILE=Toolchain-arm-linux-gnueabihf.cmake
	cd build_apps && make

load-app:
	adb shell rm -f /usr/share/data/adsp/libdspal_tester_skel.so
	adb shell rm -f /usr/share/data/adsp/libdspal_tester.so
	adb shell rm -f /home/linaro/dspal_tester
	adb push build_dsp/libdspal_tester_skel.so /usr/share/data/adsp
	adb push build_dsp/libdspal_tester.so /usr/share/data/adsp/libdspal_tester.so
	adb push build_apps/dspal_tester /home/linaro/dspal_tester
	adb shell sync

.PHONY: ADD_ON
ADD_ON:
	@[ ! -z "${HEXAGON_FC_ADDON}" ] || (echo "HEXAGON_FC_ADDON not set" && false)

load-adsp: ADD_ON
	@adb wait-for-devices
	@adb push ${HEXAGON_FC_ADDON}/images/8074-eagle/normal/adsp_proc/obj/qdsp6v5_ReleaseG/LA/system/etc/firmware /lib/firmware
	@adb shell sync
	@adb reboot

clean:
	rm -rf build_dsp build_apps dspal_tester_skel.c dspal_tester_stub.c dspal_tester.h
