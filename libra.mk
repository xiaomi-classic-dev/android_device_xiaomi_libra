DEVICE_PATH := device/xiaomi/libra
BOARD_PLATFORM := msm8992

# Covers both the 2 GiB and 3 GiB Libra variants. Without a heap profile,
# AndroidRuntime defaults to a 16 MiB maximum and system_server cannot boot.
$(call inherit-product, frameworks/native/build/phone-xhdpi-2048-dalvik-heap.mk)

# Android 14 netd requires the cgroup v2 hierarchy at /sys/fs/cgroup.
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/cgroups.json:$(TARGET_COPY_OUT_VENDOR)/etc/cgroups.json

# Radio profile
# The common tree also serves CDMA-capable variants. Libra uses a 3GPP-only
# GSM/WCDMA/LTE profile; TD-SCDMA is not used by China Unicom.
MSM8994_RIL_SUBSCRIPTION_TYPES := NV,RUIM
MSM8994_DEFAULT_NETWORK := 9,9

# Get non-open-source specific aspects
$(call inherit-product, device/xiaomi/msm8994-common/msm8994-common.mk)
$(call inherit-product, vendor/xiaomi/libra/libra-vendor.mk)

# Keep Kenzo's RIL compatible with Libra's older Voice QMI service.
PRODUCT_PACKAGES += libqmi_voice_compat
PRODUCT_PACKAGES += libppd_poll_compat
PRODUCT_PACKAGES += libpm_stack_compat
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/rootdir/zz-libra-rild.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/zz-libra-rild.rc

# Run the older GNSS blob against the isolated msm8994 QMI client ABI.
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/rootdir/zz-libra-location.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/zz-libra-location.rc

# System properties
-include $(DEVICE_PATH)/system_prop.mk

DEVICE_PACKAGE_OVERLAYS += \
    $(DEVICE_PATH)/overlay \
    $(DEVICE_PATH)/overlay-lineage

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.consumerir.xml:system/vendor/etc/permissions/android.hardware.consumerir.xml

# Media
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/media/media_codecs_performance.xml:system/vendor/etc/media_codecs_performance.xml

# Audio
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/audio/audio_platform_info.xml:system/vendor/etc/audio_platform_info.xml \
    $(DEVICE_PATH)/audio/mixer_paths.xml:system/vendor/etc/mixer_paths.xml

# Fingerprint
PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint@2.1-service

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/uinput-fpc.kl:system/vendor/usr/keylayout/uinput-fpc.kl \
    $(DEVICE_PATH)/configs/uinput-fpc.idc:system/vendor/usr/idc/uinput-fpc.idc

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.fingerprint.xml:system/vendor/etc/permissions/android.hardware.fingerprint.xml

# IRQ
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/msm_irqbalance.conf:system/vendor/etc/msm_irqbalance.conf

# keylayout
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/keylayout/ft5x46.kl:system/vendor/usr/keylayout/ft5x46.kl \
    $(DEVICE_PATH)/keylayout/atmel-maxtouch.kl:system/vendor/usr/keylayout/atmel-maxtouch.kl \
    $(DEVICE_PATH)/keylayout/atmel-maxtouch-edge.kl:system/vendor/usr/keylayout/atmel-maxtouch-edge.kl \
    $(DEVICE_PATH)/keylayout/synaptics_dsx.kl:system/vendor/usr/keylayout/synaptics_dsx.kl \
    $(DEVICE_PATH)/keylayout/synaptics_dsx_edge.kl:system/vendor/usr/keylayout/synaptics_dsx_edge.kl

# EdgeGesture
PRODUCT_PACKAGES += \
    EdgeGesture

# IR
PRODUCT_PACKAGES += \
    android.hardware.ir-service.libra

# Sensors
PRODUCT_PACKAGES += \
    sensors.msm8992

# Expose the original thermal-engine policy and real sensors to Android.
PRODUCT_PACKAGES += android.hardware.thermal-service.libra

# Screen density
PRODUCT_AAPT_PREF_CONFIG := xxhdpi

# Touch
PRODUCT_PACKAGES += \
    vendor.lineage.touch@1.0-service.libra

# Boot animation
TARGET_SCREEN_HEIGHT := 1920
TARGET_SCREEN_WIDTH := 1080

# Ramdisk
PRODUCT_PACKAGES += \
    init.fpc.rc \
    init.target.rc
