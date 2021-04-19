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
#include "autobotmodel.h"

#include "log.h"

using namespace mu::autobot;

AutobotModel::AutobotModel(QObject* parent)
    : QObject(parent)
{
    m_files = new AbFilesModel(this);

    auto status = autobot()->status();
    status.ch.onReceive(this, [this](IAutobot::Status) {
        emit statusChanged();
    });

    auto tc = autobot()->currentTestCase();
    tc.ch.onReceive(this, [this](ITestCasePtr) {
        emit currentTestCaseChanged();
    });
}

void AutobotModel::runAllFiles()
{
    autobot()->runAllFiles();
}

void AutobotModel::runFile(int fileIndex)
{
    autobot()->runFile(fileIndex);
}

void AutobotModel::stop()
{
    autobot()->stop();
}

QVariantList AutobotModel::testCases() const
{
    QVariantList list;
    std::vector<ITestCasePtr> tests = autobot()->testCases();
    if (tests.empty()) {
        QVariantMap item = { { "name", "None" } };
        list << item;
        return list;
    }

    for (const ITestCasePtr& tc : tests) {
        QVariantMap item;
        item["name"] = QString::fromStdString(tc->name());
        list << item;
    }

    return list;
}

void AutobotModel::setCurrentTestCase(const QString& testCaseName)
{
    autobot()->setCurrentTestCase(testCaseName.toStdString());
}

QString AutobotModel::currentTestCase() const
{
    return QString::fromStdString(autobot()->currentTestCase().val->name());
}

AbFilesModel* AutobotModel::files() const
{
    return m_files;
}

QString AutobotModel::status() const
{
    IAutobot::Status st = autobot()->status().val;
    switch (st) {
    case IAutobot::Status::Stoped: return "Stoped";
    case IAutobot::Status::RunningAll: return "RunningAll";
    case IAutobot::Status::RunningFile: return "RunningFile";
    }
    return "Unknown";
}
