#pragma once

#include <cstdint>

#include <QString>

namespace SysInfo {
QString currentSystem();
QString useQTForArch();
QString getSupportedJavaArchitecture();
int defaultMaxJvmMem();
int defaultMinJvmMem();
}  // namespace SysInfo
