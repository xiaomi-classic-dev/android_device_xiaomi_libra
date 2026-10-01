/*
 * Copyright (C) 2017-2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "ConsumerIrService"

#include <fcntl.h>
#include <cerrno>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/lirc.h>

#include <android-base/logging.h>

#include "ConsumerIr.h"

namespace aidl::android::hardware::ir::implementation {

#define LIRC_DEV_PATH "/dev/lirc0"

static const int dutyCycle = 33;

static const std::vector<ConsumerIrFreqRange> rangeVec{
    {.minHz = 30000, .maxHz = 60000},
};

static int openLircDev() {
    int fd = open(LIRC_DEV_PATH, O_RDWR);

    if (fd < 0) {
        LOG(ERROR) << "failed to open " << LIRC_DEV_PATH << ", error " << fd;
    }

    return fd;
}

ndk::ScopedAStatus ConsumerIr::transmit(int32_t carrierFreq, const std::vector<int32_t>& pattern) {
    if (pattern.empty()) return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    if (carrierFreq < 30000 || carrierFreq > 60000)
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    size_t entries = pattern.size();
    int rc;
    int lircFd;

    lircFd = openLircDev();
    if (lircFd < 0) {
        return ndk::ScopedAStatus::fromServiceSpecificError(errno);
    }

    rc = ioctl(lircFd, LIRC_SET_SEND_CARRIER, &carrierFreq);
    if (rc < 0) {
        LOG(ERROR) << "failed to set carrier " << carrierFreq << ", error: " << errno;
        goto out_close;
    }

    rc = ioctl(lircFd, LIRC_SET_SEND_DUTY_CYCLE, &dutyCycle);
    if (rc < 0) {
        LOG(ERROR) << "failed to set duty cycle " << dutyCycle << ", error: " << errno;
        goto out_close;
    }

    if ((entries & 1) != 0) {
        rc = write(lircFd, pattern.data(), sizeof(int32_t) * entries);
    } else {
        rc = write(lircFd, pattern.data(), sizeof(int32_t) * (entries - 1));
        usleep(pattern[entries - 1]);
    }

    if (rc < 0) {
        LOG(ERROR) << "failed to write pattern " << pattern.size() << ", error: " << errno;
        goto out_close;
    }

    rc = 0;

out_close:
    close(lircFd);

    return rc == 0 ? ndk::ScopedAStatus::ok()
                   : ndk::ScopedAStatus::fromServiceSpecificError(EIO);
}

ndk::ScopedAStatus ConsumerIr::getCarrierFreqs(std::vector<ConsumerIrFreqRange>* ranges) {
    *ranges = rangeVec;
    return ndk::ScopedAStatus::ok();
}

}  // namespace aidl::android::hardware::ir::implementation
