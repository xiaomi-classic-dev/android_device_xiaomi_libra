// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <mutex>
#include <string>
#include <vector>

namespace libra {

enum class SensorKind { SKIN, BATTERY, PMIC };
struct SensorReading {
    std::string name;
    SensorKind kind;
    float value;
    int severity;
    std::array<float, 7> hot;
};
struct CoolingReading {
    std::string name;
    bool gpu;
    long value;
};

// Read-only adapter. The original thermal-engine and kernel own mitigation.
class Sensors {
  public:
    explicit Sensors(std::string root = "/sys");
    std::vector<SensorReading> temperatures();
    std::vector<CoolingReading> coolingDevices();

  private:
    struct Sensor {
        std::string name;
        SensorKind kind;
        std::string path;
        float divisor;
        std::array<float, 7> hot;
        std::array<float, 7> clear;
        int severity = 0;
    };
    std::string mRoot;
    std::vector<Sensor> mSensors;
    std::mutex mMutex;
};

}  // namespace libra
