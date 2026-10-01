// SPDX-License-Identifier: Apache-2.0
#include "Sensors.h"

#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <functional>
#include <limits>
#include <utility>

namespace libra {
namespace {
constexpr float kUnknown = std::numeric_limits<float>::quiet_NaN();
const std::array<float, 7> kNoThresholds = {
        kUnknown, kUnknown, kUnknown, kUnknown, kUnknown, kUnknown, kUnknown};

bool readNumber(const std::string& path, long* result) {
    std::ifstream input(path);
    return static_cast<bool>(input >> *result);
}
std::string readType(const std::string& path) {
    std::ifstream input(path);
    std::string result;
    input >> result;
    return result;
}
int offlinedSeverity(long mask) {
    if (mask & (1L << 5)) return 3;
    if (mask & (1L << 4)) return 2;
    return 0;
}
}  // namespace

Sensors::Sensors(std::string root) : mRoot(std::move(root)) {
    const std::string base = mRoot + "/class/thermal/";
    DIR* dir = opendir(base.c_str());
    if (dir == nullptr) return;
    while (dirent* entry = readdir(dir)) {
        const std::string zone = entry->d_name;
        if (zone.compare(0, 12, "thermal_zone") != 0) continue;
        const std::string path = base + zone;
        const std::string type = readType(path + "/type");
        if (type == "xo_therm_buf") {
            // Original Libra policy, queried through thermal_client_config_query:
            // charging mitigation starts at 38 C; CPU4/5 hotplug at 42/45 C.
            // Clear points are 35/37/40 C. No shutdown threshold is invented.
            mSensors.push_back({"skin-xo", SensorKind::SKIN, path + "/temp", 1.0f,
                               {kUnknown, 38, 42, 45, kUnknown, kUnknown, kUnknown},
                               {kUnknown, 35, 37, 40, kUnknown, kUnknown, kUnknown}});
        } else if (type == "battery") {
            // The QPNP battery zone returns millidegrees; TSENS/XO return degrees.
            mSensors.push_back({"battery", SensorKind::BATTERY, path + "/temp", 1000.0f,
                               kNoThresholds, kNoThresholds});
        } else if (type == "pm8994_tz") {
            // Original PMIC alarm thresholds 107/127 C and kernel critical trip 145 C.
            mSensors.push_back({"pm8994", SensorKind::PMIC, path + "/temp", 1000.0f,
                               {kUnknown, 107, kUnknown, 127, kUnknown, kUnknown, 145},
                               {kUnknown, 103, kUnknown, 123, kUnknown, kUnknown, 145}});
        }
    }
    closedir(dir);
    std::sort(mSensors.begin(), mSensors.end(),
              [](const Sensor& a, const Sensor& b) { return a.name < b.name; });
}

std::vector<SensorReading> Sensors::temperatures() {
    std::lock_guard<std::mutex> lock(mMutex);
    std::vector<SensorReading> readings;
    for (Sensor& sensor : mSensors) {
        long raw;
        if (!readNumber(sensor.path, &raw)) continue;
        const float value = raw / sensor.divisor;
        if (!std::isfinite(value) || value < -100 || value > 250) continue;
        int severity = sensor.severity;
        // Preserve each tier until its original clear threshold is crossed.
        while (severity > 0 &&
               (!std::isfinite(sensor.clear[severity]) || value <= sensor.clear[severity])) {
            --severity;
        }
        for (int level = severity + 1; level < 7; ++level) {
            if (std::isfinite(sensor.hot[level]) && value >= sensor.hot[level]) severity = level;
        }
        if (sensor.kind == SensorKind::SKIN) {
            long offlined = 0;
            if (readNumber(mRoot + "/module/msm_thermal/core_control/cpus_offlined", &offlined)) {
                severity = std::max(severity, offlinedSeverity(offlined));
            }
        } else if (sensor.kind == SensorKind::BATTERY) {
            const std::string health = readType(mRoot + "/class/power_supply/battery/health");
            // Report actual charger protection, without guessing battery thresholds.
            severity = (health == "Overheat" || health == "Cold") ? 3 :
                       (health == "Warm" || health == "Cool") ? 1 : 0;
        }
        sensor.severity = severity;
        readings.push_back({sensor.name, sensor.kind, value, severity, sensor.hot});
    }
    return readings;
}

std::vector<CoolingReading> Sensors::coolingDevices() {
    std::vector<CoolingReading> readings;
    long mask;
    if (readNumber(mRoot + "/module/msm_thermal/core_control/cpus_offlined", &mask)) {
        const unsigned long disabled = static_cast<unsigned long>(mask) & 0x3fUL;
        readings.push_back({"cpu-thermal-hotplug", false, __builtin_popcountl(disabled)});
    }
    const std::string gpu = mRoot + "/class/kgsl/kgsl-3d0/";
    long cap;
    if (readNumber(gpu + "max_gpuclk", &cap)) {
        std::ifstream input(gpu + "gpu_available_frequencies");
        std::vector<long> frequencies;
        long frequency;
        while (input >> frequency) frequencies.push_back(frequency);
        std::sort(frequencies.begin(), frequencies.end(), std::greater<long>());
        frequencies.erase(std::unique(frequencies.begin(), frequencies.end()), frequencies.end());
        if (!frequencies.empty()) {
            auto allowed = std::find_if(frequencies.begin(), frequencies.end(),
                                       [cap](long f) { return f <= cap; });
            const long state = allowed == frequencies.end() ? frequencies.size() - 1 :
                                                             allowed - frequencies.begin();
            readings.push_back({"gpu-frequency-cap", true, state});
        }
    }
    return readings;
}
}  // namespace libra
