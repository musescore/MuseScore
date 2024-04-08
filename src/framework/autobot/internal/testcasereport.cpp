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
#include "testcasereport.h"

#include <QDateTime>

#include "io/path.h"
#include "log.h"

using namespace muse;
using namespace muse::autobot;

inline QTextStream& operator<<(QTextStream& s, const std::string& v)
{
    s << v.c_str();
    return s;
}

static QString formatVal(const ITestCaseContext::Val& val)
{
    return val.toString();
}

Ret TestCaseReport::beginReport(const TestCase& testCase)
{
    io::path_t reportsPath = configuration()->reportsPath();
    Ret ret = fileSystem()->makePath(reportsPath);
    if (!ret) {
        return ret;
    }

    if (m_file.isOpen()) {
        m_file.close();
    }

    QString tcname = testCase.name();
    QDateTime now = QDateTime::currentDateTime();
    QString reportPath = reportsPath.toQString()
                         + "/" + tcname
                         + "_" + now.toString("yyMMddhhmmss")
                         + ".txt";

    m_file.setFileName(reportPath);
    if (!m_file.open(QIODevice::WriteOnly)) {
        return make_ret(Ret::Code::UnknownError);
    }

    m_stream.setDevice(&m_file);

    m_stream << "Test: " << tcname << Qt::endl;
    m_stream << "date: " << now.toString("yyyy.MM.dd hh:mm") << Qt::endl;
    m_stream << "steps: ";

    Steps steps = testCase.steps();
    int count = steps.count();
    for (int i = 0; i < count; ++i) {
        m_stream << steps.step(i).name();
        if (i < (count - 1)) {
            m_stream << " -> ";
        }
    }
    m_stream << Qt::endl;
    m_stream << Qt::endl;

    m_opened = true;
    return make_ret(Ret::Code::Ok);
}

void TestCaseReport::endReport(bool aborted)
{
    if (!m_opened) {
        return;
    }

    if (aborted) {
        m_stream << "Test case aborted!" << Qt::endl;
    }

    m_stream.flush();
    m_file.close();
}

void TestCaseReport::onStepStatusChanged(const StepInfo& stepInfo, const ITestCaseContextPtr& ctx)
{
    if (!m_opened) {
        return;
    }

    switch (stepInfo.status) {
    case StepStatus::Undefined: break;
    case StepStatus::Started: {
        m_stream << "  started step: " << stepInfo.name << Qt::endl;
    } break;
    case StepStatus::Finished: {
        const ITestCaseContext::StepContext& step = ctx->currentStep();
        for (auto it = step.vals.cbegin(); it != step.vals.cend(); ++it) {
            m_stream << "    " << it->first << ": " << formatVal(it->second) << Qt::endl;
        }

        m_stream << "  finished step: " << stepInfo.name << " [" << stepInfo.durationMsec << " msec]" << Qt::endl;
    } break;
    case StepStatus::Skipped: {
        m_stream << "  skipped step: " << stepInfo.name << Qt::endl;
    } break;
    case StepStatus::Paused:
        return;
    case StepStatus::Aborted: {
        m_stream << "  abort step: " << stepInfo.name << Qt::endl;
    } break;
    case StepStatus::Error: {
        m_stream << "  error step: " << stepInfo.name << Qt::endl;
    } break;
    }

    m_stream.flush();
}
