// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "Sensors.h"

#include <aidl/android/hardware/thermal/BnThermal.h>
#include <condition_variable>
#include <optional>
#include <thread>

namespace aidl::android::hardware::thermal::libra {

class Thermal : public BnThermal {
  public:
    Thermal();
    ~Thermal() override;
    ndk::ScopedAStatus getCoolingDevices(std::vector<CoolingDevice>* out) override;
    ndk::ScopedAStatus getCoolingDevicesWithType(CoolingType type,
                                                std::vector<CoolingDevice>* out) override;
    ndk::ScopedAStatus getTemperatures(std::vector<Temperature>* out) override;
    ndk::ScopedAStatus getTemperaturesWithType(TemperatureType type,
                                              std::vector<Temperature>* out) override;
    ndk::ScopedAStatus getTemperatureThresholds(std::vector<TemperatureThreshold>* out) override;
    ndk::ScopedAStatus getTemperatureThresholdsWithType(
            TemperatureType type, std::vector<TemperatureThreshold>* out) override;
    ndk::ScopedAStatus registerThermalChangedCallback(
            const std::shared_ptr<IThermalChangedCallback>& callback) override;
    ndk::ScopedAStatus registerThermalChangedCallbackWithType(
            const std::shared_ptr<IThermalChangedCallback>& callback, TemperatureType type) override;
    ndk::ScopedAStatus unregisterThermalChangedCallback(
            const std::shared_ptr<IThermalChangedCallback>& callback) override;

  private:
    struct Callback {
        std::shared_ptr<IThermalChangedCallback> target;
        std::optional<TemperatureType> type;
    };
    ndk::ScopedAStatus registerCallback(const std::shared_ptr<IThermalChangedCallback>& callback,
                                       std::optional<TemperatureType> type);
    void monitor();
    ::libra::Sensors mSensors;
    std::mutex mMutex;
    std::condition_variable mWake;
    bool mStop = false;
    std::vector<Callback> mCallbacks;
    std::thread mMonitor;
};
}  // namespace aidl::android::hardware::thermal::libra
