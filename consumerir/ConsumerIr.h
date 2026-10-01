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

#pragma once
#include <aidl/android/hardware/ir/BnConsumerIr.h>
#include <vector>

namespace aidl::android::hardware::ir::implementation {
class ConsumerIr : public BnConsumerIr {
  public:
    ndk::ScopedAStatus transmit(int32_t carrierFreq, const std::vector<int32_t>& pattern) override;
    ndk::ScopedAStatus getCarrierFreqs(std::vector<ConsumerIrFreqRange>* ranges) override;
};
}  // namespace aidl::android::hardware::ir::implementation
