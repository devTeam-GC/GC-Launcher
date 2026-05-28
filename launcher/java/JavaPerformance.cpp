// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2026 Octol1ttle <l1ttleofficial@outlook.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "JavaPerformance.h"

#include <QDebug>
#include <QObject>

QStringList JavaPerformance::getBaseOptimizationArgs(const JavaVersion& version, const GarbageCollectorPreset preset)
{
    QStringList args;

    args << "-XX:+UnlockExperimentalVMOptions";

    // Mojang default
    // JEPs: https://openjdk.org/jeps/450, https://openjdk.org/jeps/519
    if (version.major() >= 24) {
        args << "-XX:+UseCompactObjectHeaders";
    }

    // Mojang default
    // Since Java 8 for G1GC
    // Since Java 18 for ShenandoahGC
    //if ((version.major() >= 8 && preset == GarbageCollectorPreset::G1GC) || version.major() >= 18) {
    //    args << "-XX:+UseStringDeduplication";
    //}

    // Mojang default
    // Since Java 6 (minimum supported by us is Java 7)
    //args << "-XX:+AlwaysPreTouch";

    // Prevents GC from doing writing performance data to filesystem
    // https://www.evanjones.ca/jvm-mmap-pause.html
    // Since Java 6
    args << "-XX:+PerfDisableSharedMem";

    return args;
}

QStringList JavaPerformance::getGarbageCollectorArgs(const JavaVersion& version, const GarbageCollectorPreset preset)
{
    switch (preset) {
        case GarbageCollectorPreset::None:
            return {};
        case GarbageCollectorPreset::G1GC: {
            // Mojang defaults
            QStringList args{ "-XX:+UseG1GC",
                              "-XX:G1NewSizePercent=20", "-XX:G1ReservePercent=20",
                              "-XX:MaxGCPauseMillis=50", "-XX:G1HeapRegionSize=16M",
                              // Aikar's flags
                              "-XX:SurvivorRatio=32", "-XX:MaxTenuringThreshold=1" };
            return args;
        }
        case GarbageCollectorPreset::ShenandoahGC: {
            QStringList args{ "-XX:+UseShenandoahGC", "-XX:ShenandoahGCMode=generational" };

            return args;
        }
    }

    Q_ASSERT_X(false, "JavaPerformance::getGarbageCollectorArgs", "No arguments specified for current garbage collector preset");
    return {};
}

QStringList JavaPerformance::getCompletePerformanceArgs(const JavaVersion& version,
                                                        const bool useOptimized,
                                                        GarbageCollectorPreset preset,
                                                        QString* warning)
{
    if (preset == GarbageCollectorPreset::ShenandoahGC && version.major() < 24) {
        preset = GarbageCollectorPreset::G1GC;
        if (warning) {
            *warning = QObject::tr("ShenandoahGC requires Java 24 or higher, using G1GC");
        }
    }

    if (!useOptimized) {
        return getGarbageCollectorArgs(version, preset);
    }
    return getBaseOptimizationArgs(version, preset) + getGarbageCollectorArgs(version, preset);
}
