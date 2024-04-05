/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MUSE_AUTOBOT_ABTYPES_H
#define MUSE_AUTOBOT_ABTYPES_H

#include <string>
#include <vector>
#include <QJSValue>

#include "io/path.h"
#include "types/ret.h"
#include "autobotutils.h"

namespace muse::autobot {
struct File {
    io::path_t path;
    Ret completeRet; // if undefined - means not tested
};

using Files = std::vector<File>;

enum class ScriptType {
    Undefined = 0,
    TestCase,
    Custom
};

struct Script
{
    io::path_t path;
    ScriptType type = ScriptType::Undefined;
    QString title;
    QString description;
};

using Scripts = std::vector<Script>;

// --- TestCase ---
//! NOTE Scripts with test cases must have a global variable with the name specified in the constant
constexpr const char* TESTCASE_JS_GLOBALNAME = "testCase";

struct Step
{
    Step(const QJSValue& jsval = QJSValue())
        : val(jsval) {}

    QString name() const { return val.property("name").toString(); }
    bool skip() const { return val.property("skip").toBool(); }
    Ret exec()
    {
        QJSValue jsret = val.property("func").call();
        return jsValueToRet(jsret);
    }

private:
    QJSValue val;
};

struct Steps
{
    Steps(const QJSValue& jsval = QJSValue())
        : val(jsval) {}

    int count() const { return val.property("length").toInt(); }
    Step step(int i) const { return Step(val.property(i)); }

private:
    QJSValue val;
};

struct TestCase
{
    TestCase(const QJSValue& jsval = QJSValue())
        : val(jsval) {}

    bool isValid() const { return !val.isUndefined() && val.hasProperty("name") && val.hasProperty("steps"); }

    QString name() const { return val.property("name").toString(); }
    QString description() const { return val.property("description").toString(); }
    Steps steps() const { return Steps(val.property("steps")); }

private:
    QJSValue val;
};

enum class StepStatus {
    Undefined = 0,
    Started,
    Paused,
    Finished,
    Skipped,
    Aborted,
    Error
};

struct StepInfo
{
    QString name;
    StepStatus status;
    int durationMsec = 0;

    StepInfo() = default;
    StepInfo(const QString& n, StepStatus s)
        : name(n), status(s) {}
    StepInfo(const QString& n, StepStatus s, int dur)
        : name(n), status(s), durationMsec(dur) {}
};

enum class SpeedMode {
    Undefined = 0,
    Default,
    Fast,
    Normal,
    Slow
};

inline QString speedModeToString(SpeedMode mode)
{
    switch (mode) {
    case SpeedMode::Undefined: return "";
    case SpeedMode::Default: return "Default";
    case SpeedMode::Fast: return "Fast";
    case SpeedMode::Normal: return "Normal";
    case SpeedMode::Slow: return "Slow";
    }
    return QString();
}

inline SpeedMode speedModeFromString(const QString& str)
{
    if (str == "Default") {
        return SpeedMode::Default;
    }
    if (str == "Fast") {
        return SpeedMode::Fast;
    }
    if (str == "Normal") {
        return SpeedMode::Normal;
    }
    if (str == "Slow") {
        return SpeedMode::Slow;
    }
    return SpeedMode::Undefined;
}
}

#endif // MUSE_AUTOBOT_AUTOBOTTYPES_H
