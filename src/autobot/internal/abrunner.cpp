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
#include "abrunner.h"

#include <QTimer>

#include "log.h"

using namespace mu::autobot;

void AbRunner::runTestCase(const QJSValue& testCase)
{
}

void AbRunner::nextStep(const IAbContextPtr& ctx)
{
}

void AbRunner::doFinish(const IAbContextPtr& ctx)
{
    m_allFinished.send(ctx);
}

mu::async::Channel<IAbContextPtr> AbRunner::stepStarted() const
{
    return m_stepStarted;
}

mu::async::Channel<IAbContextPtr> AbRunner::stepFinished() const
{
    return m_stepFinished;
}

mu::async::Channel<IAbContextPtr> AbRunner::allFinished() const
{
    return m_allFinished;
}
