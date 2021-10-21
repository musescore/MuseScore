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

Autobot::~Autobot()
{
    delete m_engine;
}

void Autobot::init()
{
    m_engine = new ScriptEngine();
}

mu::Ret Autobot::loadScript(const Script& script)
{
    LOGD() << script.path;

    m_engine->setScriptPath(script.path);
    Ret ret = m_engine->call("main");
    if (!ret) {
        LOGE() << ret.toString();
    }
    return ret;
}

void Autobot::setStepsInterval(int msec)
{
    m_runner.setStepsInterval(msec);
}

void Autobot::runTestCase(const QJSValue& testCase)
{
    m_runner.runTestCase(testCase);
}

void Autobot::abortTestCase()
{
    m_runner.abortTestCase();
}

bool Autobot::pauseTestCase()
{
    using namespace mu::framework;
    IInteractive::Result res = interactive()->question("Pause", "Continue?",
                                                       { IInteractive::Button::Continue, IInteractive::Button::Abort });

    if (res.standardButton() == IInteractive::Button::Abort) {
        abortTestCase();
        return false;
    }

    return true;
}
