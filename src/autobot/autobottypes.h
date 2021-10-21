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
#ifndef MU_AUTOBOT_ABTYPES_H
#define MU_AUTOBOT_ABTYPES_H

#include <string>
#include <vector>
#include <QJSValue>

#include "io/path.h"
#include "ret.h"

namespace mu::autobot {
struct File {
    io::path path;
    Ret completeRet; // if undefined - means not tested
};

using Files = std::vector<File>;

struct Script
{
    io::path path;
    QString title;
    QString description;
};

using Scripts = std::vector<Script>;

// --- TestCase ---
struct Step
{
    Step(const QJSValue& jsval = QJSValue())
        : val(jsval) {}

    QString name() const { return val.property("name").toString(); }
    Ret exec()
    {
        val.property("func").call();

        if (val.isError()) {
            QString fileName = val.property("fileName").toString();
            int line = val.property("lineNumber").toInt();
            return make_ret(Ret::Code::UnknownError,
                            QString("File: %1, Exception at line: %2, %3").arg(fileName).arg(line).arg(val.toString()));
        }

        return Ret(Ret::Code::Ok);
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

    QString name() const { return val.property("name").toString(); }
    Steps steps() const { return Steps(val.property("steps")); }

private:
    QJSValue val;
};
}

#endif // MU_AUTOBOT_AUTOBOTTYPES_H
