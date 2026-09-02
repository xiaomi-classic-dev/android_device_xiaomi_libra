Xiaomi Mi 4c (libra) repartition finishing package.

Use this package only after the system2944/cache384/cust320 GPT was written but
the old recovery kernel failed to refresh it with "recv error on uevent".

First reboot back into recovery. This package then requires exactly:

- p41 system:   sectors 1048576-7077887 (2944 MiB)
- p42 cache:    sectors 7077888-7864319 (384 MiB)
- p43 cust:     sectors 7864320-8519679 (320 MiB)
- p44 userdata: sectors 8519680-last usable GPT sector

It never writes the GPT. It formats p41, p42, p43, and p44 as ext4, permanently
wiping system, cache, cust, userdata, and internal storage. It refuses to run
if the target partition layout is not visible to the current kernel, or if all
four target filesystems are already ext4.

Install after rebooting recovery:

  adb sideload libra_repartition_finish_system2944_cache384_cust320.zip

