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
#include "autobot.h"

#include <QTimer>

#include "log.h"

#include "abcontext.h"

#include "scriptengine.h"

using namespace mu::autobot;

Autobot::Autobot()
{
}

void Autobot::init()
{
    m_runner.allFinished().onReceive(this, [this](const IAbContextPtr& ctx) {
        // onFileFinished(ctx);
    });

    m_runner.stepStarted().onReceive(this, [this](const IAbContextPtr& ctx) {
        m_report.beginStep(ctx);
    });

    m_runner.stepFinished().onReceive(this, [this](const IAbContextPtr& ctx) {
        m_report.endStep(ctx);
    });
}

mu::Ret Autobot::loadScript(const Script& script)
{
    LOGD() << script.path;

    m_engine.setScriptPath(script.path);
    Ret ret = m_engine.call("main");
    if (!ret) {
        LOGE() << ret.toString();
    }
    return ret;
}

void Autobot::runTestCase(const QJSValue& testCase)
{
    m_runner.runTestCase(testCase);
}

void Autobot::stop()
{
    //m_status.set(Status::Stoped);
}
