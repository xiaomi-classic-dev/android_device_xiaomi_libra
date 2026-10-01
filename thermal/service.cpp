// SPDX-License-Identifier: Apache-2.0
#define LOG_TAG "thermal-libra"
#include "Thermal.h"
#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <cstdio>
#include <cstring>

using aidl::android::hardware::thermal::libra::Thermal;

int main(int argc, char** argv) {
    if (argc == 2 && std::strcmp(argv[1], "--dump-sensors") == 0) {
        libra::Sensors sensors;
        const auto readings = sensors.temperatures();
        for (const auto& value : readings) {
            std::printf("%s %.3f C severity=%d\n", value.name.c_str(), value.value, value.severity);
        }
        for (const auto& value : sensors.coolingDevices()) {
            std::printf("%s cooling_state=%ld\n", value.name.c_str(), value.value);
        }
        return readings.empty() ? 1 : 0;
    }
    ABinderProcess_setThreadPoolMaxThreadCount(4);
    const auto thermal = ndk::SharedRefBase::make<Thermal>();
    const std::string name = std::string(Thermal::descriptor) + "/default";
    CHECK_EQ(AServiceManager_addService(thermal->asBinder().get(), name.c_str()), STATUS_OK);
    LOG(INFO) << "Registered Libra thermal HAL using the original read-only sensor interfaces";
    ABinderProcess_joinThreadPool();
    return 1;
}
