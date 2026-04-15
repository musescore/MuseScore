/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "testcaserunmodel.h"

#include "internal/scriptengine.h"

#include "log.h"

using namespace muse::testflow;

TestCaseRunModel::TestCaseRunModel(QObject* parent)
    : QObject(parent), Contextable(muse::iocCtxForQmlObject(this))
{
}

void TestCaseRunModel::init()
{
    testflow()->statusChanged().onReceive(this, [this](const io::path_t&, const ITestflow::Status&) {
        emit statusChanged();
    });

    testflow()->stepStatusChanged().onReceive(this, [this](const StepInfo& stepInfo, const Ret& ret) {
        updateStep(stepInfo, ret);
    });
}

bool TestCaseRunModel::loadInfo(const QString& path)
{
    m_path = path;
    m_testCase.clear();
    m_steps.clear();

    ScriptEngine engine(iocContext());
    engine.setScriptPath(path);
    if (!engine.evaluate()) {
        LOGE() << "failed evaluate script, path: " << path;
        return false;
    }

    QJSValue tcVal = engine.globalProperty(TESTCASE_JS_GLOBALNAME);
    TestCase tc(tcVal);
    if (!tc.isValid()) {
        LOGE() << "not found `testCase` or it not valid, path: " << path;
        return false;
    }

    Steps steps = tc.steps();

    m_testCase["name"] = tc.name();
    m_testCase["description"] = tc.description();
    m_testCase["stepsCount"] = steps.count();

    for (int i = 0; i < steps.count(); ++i) {
        Step step = steps.step(i);
        StepItem item;
        item.name = step.name();
        m_steps.append(item);
    }

    emit testCaseChanged();
    emit stepsChanged();

    return true;
}

void TestCaseRunModel::perform()
{
    ITestflow::Status st = testflow()->status();
    switch (st) {
    case ITestflow::Status::Undefined:
        testflow()->execScript(m_path);
        break;
    case ITestflow::Status::Running:
        testflow()->pause();
        break;
    case ITestflow::Status::Paused:
        testflow()->unpause();
        break;
    case ITestflow::Status::Aborted:
        testflow()->execScript(m_path);
        break;
    case ITestflow::Status::Error:
        testflow()->execScript(m_path);
        break;
    case ITestflow::Status::Finished:
        testflow()->execScript(m_path);
        break;
    }
}

void TestCaseRunModel::abort()
{
    testflow()->abort();
}

QVariantMap TestCaseRunModel::testCase() const
{
    return m_testCase;
}

int TestCaseRunModel::stepIndexOf(const QString& name) const
{
    for (int i = 0; i < m_steps.count(); ++i) {
        if (m_steps.at(i).name == name) {
            return i;
        }
    }
    return -1;
}

void TestCaseRunModel::updateStep(const StepInfo& stepInfo, const Ret& ret)
{
    auto stepStatusToString = [](const StepStatus& stepStatus) {
        switch (stepStatus) {
        case StepStatus::Undefined: return "Undefined";
        case StepStatus::Started: return "Started";
        case StepStatus::Paused: return "Paused";
        case StepStatus::Finished: return "Finished";
        case StepStatus::Skipped: return "Skipped";
        case StepStatus::Aborted: return "Aborted";
        case StepStatus::Error: return "Error";
        }
        return "";
    };

    int idx = stepIndexOf(stepInfo.name);
    if (idx == -1) {
        LOGE() << "not found step: " << stepInfo.name;
        return;
    }

    StepItem& s = m_steps[idx];
    s.status = stepStatusToString(stepInfo.status);
    if (stepInfo.status == StepStatus::Error) {
        s.status = ret.data<QString>("err", QString("Unknown error"));
    }

    if (stepInfo.durationMsec) {
        s.duration = QString("%1 msec").arg(stepInfo.durationMsec);
    }

    emit stepsChanged();
    emit currentStepChanged(idx);
}

QString TestCaseRunModel::status() const
{
    ITestflow::Status st = testflow()->status();
    return ITestflow::statusToString(st);
}

QVariantList TestCaseRunModel::steps() const
{
    QVariantList list;
    for (const StepItem& s : m_steps) {
        QVariantMap obj;
        obj["name"] = s.name;
        obj["status"] = s.status;
        obj["duration"] = s.duration;
        list.append(obj);
    }
    return list;
}
