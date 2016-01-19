# DSPAL

The DSP Abstraction Layer (DSPAL) provides a standard interface for porting
code to the Hexagon processor.

## Setup Development Environment

See [GettingStarted](https://github.com/ATLFlight/ATLFlightDocs/GettingStarted.md).

After installing the development tools, make sure you have your environment variables set up:
```
export HEXAGON_SDK_ROOT=${HOME}/Qualcomm/Hexagon_SDK/2.0
export HEXAGON_TOOLS_ROOT=${HOME}/Qualcomm/HEXAGON_Tools/7.2.10/Tools
export HEXAGON_ARM_SYSROOT=${HOME}/Qualcomm/Hexagon_SDK/2.0/sysroot
export PATH=${HEXAGON_SDK_ROOT}/gcc-linaro-arm-linux-gnueabihf-4.8-2013.08_linux/bin:$PATH
```

## Testing DSPAL

```
git clone https://github.com/ATLFlight/dspal
cd dspal/test/dspal_tester
make
cd build
```

Connect the device via ADB and make sure it can be found.
```
adb devices
```
You should see something like:
```
$ adb devices
List of devices attached 
997e5d3a	device
```
Now load the dspal_tester app on the device.
```
cd build
make dspal_tester-load
```

This will push dspal_tester_app to /home/linaro/ on the device, and it will push libdspal_tester.so and libdspal_tester_skel.so to /usr/share/data/adsp/ on the device.

## Running the program
To see the program output from the code running on the DSP, you will need to run mini-dm in another terminal.
```
${HEXAGON_SDK_ROOT}/tools/mini-dm/Linux_Debug/mini-dm
```

To run the program:
```
$ adb shell
# cd /home/linaro
# ./dspal_tester_app
```

You should see output on the ADB terminal similar to:
```
Starting DspAL tests
testing time.h
/local/mnt/workspace/lnxbuild/project/trees_in_use/free_tree_le_manifest_LNX.LER.1.0_eagle8074_commander_17199839/checkout/oe-core/build/tmp-eglibc/work/cortexa8hf-vfp-neon-linux-gnueabi/adsprpc/1.0-r0/adsprpc-1.0/src/fastrpc_apps_user.c:136:failed to create tls key/local/mnt/workspace/lnxbuild/project/trees_in_use/free_tree_le_manifest_LNX.LER.1.0_eagle8074_commander_17199839/checkout/oe-core/build/tmp-eglibc/work/cortexa8hf-vfp-neon-linux-gnueabi/adsprpc/1.0-r0/adsprpc-1.0/src/listener_android.c:112:listener using ion heap: -1
[  PASS] clockid values exist
[  PASS] sigevent values exist
[  PASS] time returns good value
[  PASS] timer realtime
[  PASS] timer monotonic
[- SKIP] timer process cputime
[- SKIP] timer thread cputime
[  PASS] time return value
[  PASS] time parameter
[  PASS] usleep for two seconds
[  PASS] clock_getres
[  PASS] clock_gettime
[  PASS] clock_settime
[  PASS] one shot timer cb
[  PASS] periodic timer cb
[  PASS] periodic timer signal cb
[  PASS] periodic timer sigwait
testing pthread.h
[  PASS] pthread attr init
[  PASS] pthread create
[- SKIP] pthread cancel
[  PASS] pthread self
[  PASS] pthread exit
[- SKIP] pthread kill
[  PASS] pthread condition timed wait
[  PASS] pthread mutex lock
[  PASS] thread mutex lock thread
[  PASS] thread large allocation on stack
[  PASS] thread large allocation on heap
[  PASS] usleep for two seconds
testing semaphore.h
[  PASS] semaphore wait
testing C++
[  PASS] test C++ heap
[  PASS] test C++ static initialization
tests complete
testing device path access
[  PASS] spi loopback test
...
DspAL some tests skipped.
DspAL tests succeeded.
```
