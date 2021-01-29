#! /bin/sh

rmmod lscpcie
insmod ./lscpcie.ko
chmod 666 /dev/lscpcie0