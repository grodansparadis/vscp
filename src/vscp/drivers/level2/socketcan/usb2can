#/bin/sh
modprobe can
modprobe can_raw
modprobe can_dev
modprobe can_bcm
modprobe vcan
insmod usb_8dev.ko
ip link add dev vcan0 type vcan
ip link set up vcan0
ip link show vcan0
ip link set can0 up type can bitrate 125000 sample-point 0.875
