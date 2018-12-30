# Dalvik heap
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapstartsize=16m \
    dalvik.vm.heapgrowthlimit=192m \
    dalvik.vm.heapsize=512m

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=480

# Display feature support
# bit0-ColorPrefer bit1-EyeCare bit2-AD bit3-CE bit4-CABC bit5-SRGB
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sys.display.support=23

#Enable 16 bit PCM offload by default
PRODUCT_PROPERTY_OVERRIDES += \
    audio.offload.pcm.16bit.enable=false

#Enable 24 bit PCM offload by default
PRODUCT_PROPERTY_OVERRIDES += \
    audio.offload.pcm.24bit.enable=false \
    audio.offload.disable=1

#property to set fingerpirnt vendor
PRODUCT_PROPERTY_OVERRIDES += \
    ro.hardware.fingerprint=fpc \
    sys.fpc.tu.disabled=0
