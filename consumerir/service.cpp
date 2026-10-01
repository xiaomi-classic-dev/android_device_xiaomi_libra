// SPDX-License-Identifier: Apache-2.0
#include "ConsumerIr.h"
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <log/log.h>
#include <string>

int main() {
    using aidl::android::hardware::ir::IConsumerIr;
    using aidl::android::hardware::ir::implementation::ConsumerIr;
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    auto service = ndk::SharedRefBase::make<ConsumerIr>();
    const std::string instance = std::string(IConsumerIr::descriptor) + "/default";
    if (AServiceManager_addService(service->asBinder().get(), instance.c_str()) != STATUS_OK) {
        ALOGE("Cannot register ConsumerIr HAL service");
        return 1;
    }
    ABinderProcess_joinThreadPool();
    return 1;
}
