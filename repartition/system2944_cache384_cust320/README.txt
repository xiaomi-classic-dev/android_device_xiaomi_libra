Xiaomi Mi 4c (libra) recovery repartition package.

WARNING
=======

This package permanently wipes system, cache, cust, userdata, and all files in
internal storage. Back up everything before flashing it. Keep the phone on a
charger and do not interrupt the operation.

The package only accepts a libra whose current partition table has all of the
following properties:

- system is mmcblk0p41 and exactly 1920 MiB
- cache is mmcblk0p42 and exactly 384 MiB
- cust is mmcblk0p43 and exactly 320 MiB
- userdata is mmcblk0p44 and reaches the final usable GPT sector
- system, cache, cust, and userdata are contiguous

Target layout:

- system:   2944 MiB (gains 1024 MiB)
- cache:     384 MiB (same size, moved by 1024 MiB)
- cust:      320 MiB (same size, moved by 1024 MiB)
- userdata: remainder (loses 1024 MiB)

The script deletes and recreates partitions 41 through 44. It does not move or
preserve their filesystems. Partitions 1 through 40, including persist and
oops, are not modified.

Before flashing, save a GPT backup to a computer if your recovery provides
sgdisk. For example:

  adb shell sgdisk --backup=/tmp/libra-gpt-before.bin /dev/block/mmcblk0
  adb pull /tmp/libra-gpt-before.bin

Install from recovery with:

  adb sideload libra_repartition_system2944_cache384_cust320-v3.zip

The v3 updater is self-contained: its partitioning logic and static parted
payload are embedded in update-binary, so it also works with recoveries whose
streamed /sideload/package.zip cannot be reopened by the updater.

The package formats system, cache, cust, and userdata as ext4 and then reboots
back into recovery. A ROM may use the enlarged partition with:

  BOARD_SYSTEMIMAGE_PARTITION_SIZE := 3087007744

The bundled static ARM parted binary comes from xiaomi-classic-dev's aries
LineageOS 19.1 repartition package. Its SHA-256 is:

  e7c06fd959c3c259f5bce9829406207bf6990b486a79b7f2ee935314099cc126
