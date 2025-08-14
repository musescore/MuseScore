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
#include "process.h"

#include "muse_framework_config.h"

#ifdef QT_QPROCESS_SUPPORTED
#include <QProcess>
#endif

#include "log.h"

using namespace muse;

#ifdef QT_QPROCESS_SUPPORTED
static QStringList toQList(const std::vector<std::string>& args)
{
    QStringList list;
    for (const std::string& a : args) {
        list << QString::fromStdString(a);
    }
    return list;
}

#endif

int Process::execute(const std::string& program, const std::vector<std::string>& args)
{
#ifdef QT_QPROCESS_SUPPORTED
    int ret = QProcess::execute(QString::fromStdString(program), toQList(args));
    return ret;
#else
    UNUSED(program);
    UNUSED(args);
    NOT_SUPPORTED;
    return -1;
#endif
}

bool Process::startDetached(const std::string& program, const std::vector<std::string>& args)
{
#ifdef QT_QPROCESS_SUPPORTED
    bool ok = QProcess::startDetached(QString::fromStdString(program), toQList(args));
    return ok;
#else
    UNUSED(program);
    UNUSED(args);
    NOT_SUPPORTED;
    return false;
#endif
}
