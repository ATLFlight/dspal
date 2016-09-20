@echo on
adb wait-for-device

adb remount
adb root
echo start QDSPs
rem call start_dsp.cmd 3
rem sleep 5
rem


set adsp_lib_dir=/system/vendor/lib/rfsa/adsp
set user_lib_dir=/system/vendor/lib
rem set adsp_lib_dir=/usr/share/data/adsp
rem adb shell rm %adsp_lib_dir_bad%/*
adb shell rm -rf  %adsp_lib_dir_bad%

adb shell mkdir -p  %adsp_lib_dir%

adb push build\dspal_tester\libdspal_tester.so   %user_lib_dir%/
adb push build\dspal_tester\libdspal_tester.so   %adsp_lib_dir%/
adb push build\dspal_tester\libdspal_tester_skel.so  %adsp_lib_dir%/
adb push build\dspal_tester\dspal_tester /home/root/.
adb push build\dspal_tester\libdspal_tester.so   /home/root/.
adb shell chmod 755 /home/root/dspal_tester

rem adb push \\crate\adsp-platform\contexthub\scripts\testsig-0x0.so  %adsp_lib_dir%/
adb push C:\Qualcomm\Hexagon_SDK\2.0\tools\elfsigner\testsigs\testsig-0xf1ee91f3.so %adsp_lib_dir%/
adb push C:\Qualcomm\Hexagon_SDK\2.0\tools\elfsigner\testsigs\testsig-0xe998edb0.so %adsp_lib_dir%/
adb push C:\Qualcomm\Hexagon_SDK\2.0\tools\elfsigner\testsigs\testsig-0xe9490f8e.so %adsp_lib_dir%/.


adb push \\armory\atlanticus\builds\OE_0510\fastrpc_shell_0  %adsp_lib_dir%/
adb shell chmod 755   %adsp_lib_dir%/fastrpc_shell_0

adb shell ldconfig
adb shell export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:%user_lib_dir%:/usr/lib:/lib:/home/root
adb shell export ADSP_LIBRARY_PATH=%adsp_lib_dir%

adb shell "echo 73 > /sys/class/gpio/export"
adb shell "echo out > /sys/class/gpio/gpio73/direction"
adb shell "echo 1 > /sys/class/gpio/gpio73/value"
adb shell "cat /sys/class/gpio/gpio73/direction"
adb shell "cat /sys/class/gpio/gpio73/value  "

adb shell "echo 110 > /sys/class/gpio/export"
adb shell "echo out > /sys/class/gpio/gpio110/direction"
adb shell "echo 1 > /sys/class/gpio/gpio110/value"
adb shell "cat /sys/class/gpio/gpio110/direction"
adb shell "cat /sys/class/gpio/gpio110/value  "


adb shell /home/root/dspal_tester

