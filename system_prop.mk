# Display feature support
# bit0-ColorPrefer bit1-EyeCare bit2-AD bit3-CE bit4-CABC bit5-SRGB
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sys.display.support=23 \
    ro.hardware.egl=adreno

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

# Camera
PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.HAL3.enabled=1

# eBPF support
PRODUCT_PROPERTY_OVERRIDES += \
    ro.kernel.ebpf.supported=true

# NetBpfLoad supports a version override for kernels with BPF backports.
# Android 17's Tethering APEX rejects <5.10 and expects 5.10.199 on that branch.
PRODUCT_SYSTEM_PROPERTIES += \
    ro.bpf.kver_override=5.10.199

# The kernel LMK already handles memory pressure. Android also requests PSI
# monitoring after boot; zero thresholds skip the unsupported PSI triggers.
PRODUCT_SYSTEM_PROPERTIES += \
    ro.lmk.use_new_strategy=true \
    ro.lmk.psi_partial_stall_ms=0 \
    ro.lmk.psi_complete_stall_ms=0
