// SPDX-License-Identifier: Apache-2.0
#include "Thermal.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

namespace aidl::android::hardware::thermal::libra {
namespace {
TemperatureType typeOf(::libra::SensorKind kind) {
    switch (kind) {
        case ::libra::SensorKind::SKIN: return TemperatureType::SKIN;
        case ::libra::SensorKind::BATTERY: return TemperatureType::BATTERY;
        case ::libra::SensorKind::PMIC: return TemperatureType::UNKNOWN;
    }
    return TemperatureType::UNKNOWN;
}
bool equal(const std::shared_ptr<IThermalChangedCallback>& a,
           const std::shared_ptr<IThermalChangedCallback>& b) {
    return a && b && a->asBinder() == b->asBinder();
}
ndk::ScopedAStatus invalid(const char* message) {
    return ndk::ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_ARGUMENT, message);
}
}  // namespace

Thermal::Thermal() : mMonitor(&Thermal::monitor, this) {}
Thermal::~Thermal() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mStop = true;
    }
    mWake.notify_all();
    mMonitor.join();
}

ndk::ScopedAStatus Thermal::getTemperatures(std::vector<Temperature>* out) {
    out->clear();
    for (const auto& reading : mSensors.temperatures()) {
        Temperature value;
        value.name = reading.name;
        value.type = typeOf(reading.kind);
        value.value = reading.value;
        value.throttlingStatus = static_cast<ThrottlingSeverity>(reading.severity);
        out->push_back(value);
    }
    if (out->empty()) {
        return ndk::ScopedAStatus::fromExceptionCodeWithMessage(EX_ILLEGAL_STATE,
                                                               "No readable Libra thermal sensors");
    }
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getTemperaturesWithType(TemperatureType type,
                                                  std::vector<Temperature>* out) {
    auto status = getTemperatures(out);
    if (!status.isOk()) return status;
    out->erase(std::remove_if(out->begin(), out->end(),
                              [type](const auto& t) { return t.type != type; }), out->end());
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getTemperatureThresholds(std::vector<TemperatureThreshold>* out) {
    out->clear();
    for (const auto& reading : mSensors.temperatures()) {
        TemperatureThreshold value;
        value.name = reading.name;
        value.type = typeOf(reading.kind);
        value.hotThrottlingThresholds.assign(reading.hot.begin(), reading.hot.end());
        value.coldThrottlingThresholds.assign(7, std::numeric_limits<float>::quiet_NaN());
        out->push_back(value);
    }
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getTemperatureThresholdsWithType(
        TemperatureType type, std::vector<TemperatureThreshold>* out) {
    auto status = getTemperatureThresholds(out);
    if (!status.isOk()) return status;
    out->erase(std::remove_if(out->begin(), out->end(),
                              [type](const auto& t) { return t.type != type; }), out->end());
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getCoolingDevices(std::vector<CoolingDevice>* out) {
    out->clear();
    for (const auto& reading : mSensors.coolingDevices()) {
        CoolingDevice value;
        value.name = reading.name;
        value.type = reading.gpu ? CoolingType::GPU : CoolingType::CPU;
        value.value = reading.value;
        out->push_back(value);
    }
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::getCoolingDevicesWithType(CoolingType type,
                                                    std::vector<CoolingDevice>* out) {
    auto status = getCoolingDevices(out);
    if (!status.isOk()) return status;
    out->erase(std::remove_if(out->begin(), out->end(),
                              [type](const auto& t) { return t.type != type; }), out->end());
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::registerCallback(
        const std::shared_ptr<IThermalChangedCallback>& callback,
        std::optional<TemperatureType> type) {
    if (!callback) return invalid("Null thermal callback");
    std::lock_guard<std::mutex> lock(mMutex);
    for (const auto& registered : mCallbacks) {
        if (equal(registered.target, callback)) return invalid("Thermal callback already registered");
    }
    mCallbacks.push_back({callback, type});
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Thermal::registerThermalChangedCallback(
        const std::shared_ptr<IThermalChangedCallback>& callback) {
    return registerCallback(callback, std::nullopt);
}
ndk::ScopedAStatus Thermal::registerThermalChangedCallbackWithType(
        const std::shared_ptr<IThermalChangedCallback>& callback, TemperatureType type) {
    return registerCallback(callback, type);
}
ndk::ScopedAStatus Thermal::unregisterThermalChangedCallback(
        const std::shared_ptr<IThermalChangedCallback>& callback) {
    if (!callback) return invalid("Null thermal callback");
    std::lock_guard<std::mutex> lock(mMutex);
    auto match = std::find_if(mCallbacks.begin(), mCallbacks.end(),
                             [&](const auto& c) { return equal(c.target, callback); });
    if (match == mCallbacks.end()) return invalid("Thermal callback was not registered");
    mCallbacks.erase(match);
    return ndk::ScopedAStatus::ok();
}

void Thermal::monitor() {
    std::map<std::string, Temperature> previous;
    for (;;) {
        std::vector<Temperature> current;
        if (getTemperatures(&current).isOk()) {
            std::vector<Callback> callbacks;
            {
                std::lock_guard<std::mutex> lock(mMutex);
                callbacks = mCallbacks;
            }
            for (const auto& value : current) {
                auto old = previous.find(value.name);
                if (old != previous.end() && old->second.throttlingStatus == value.throttlingStatus &&
                    std::fabs(old->second.value - value.value) < 0.5f) continue;
                previous[value.name] = value;
                for (const auto& callback : callbacks) {
                    if (callback.type && *callback.type != value.type) continue;
                    if (callback.target->notifyThrottling(value).getStatus() == STATUS_DEAD_OBJECT) {
                        unregisterThermalChangedCallback(callback.target);
                    }
                }
            }
        }
        std::unique_lock<std::mutex> lock(mMutex);
        if (mWake.wait_for(lock, std::chrono::seconds(2), [this] { return mStop; })) break;
    }
}
}  // namespace aidl::android::hardware::thermal::libra
