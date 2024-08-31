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
#ifndef MUSE_AUTOBOT_TESTCASERUNMODEL_H
#define MUSE_AUTOBOT_TESTCASERUNMODEL_H

#include <QObject>
#include <QVariant>

#include "modularity/ioc.h"
#include "internal/scriptengine.h"
#include "autobottypes.h"
#include "../iautobot.h"
#include "async/asyncable.h"

namespace muse::autobot {
class TestCaseRunModel : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap testCase READ testCase NOTIFY testCaseChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QVariantList steps READ steps NOTIFY stepsChanged)

    Inject<IAutobot> autobot = { this };

public:
    TestCaseRunModel(QObject* parent = nullptr);

    QVariantMap testCase() const;
    QString status() const;
    QVariantList steps() const;

    Q_INVOKABLE void init();
    Q_INVOKABLE bool loadInfo(const QString& path);
    Q_INVOKABLE void perform();
    Q_INVOKABLE void abort();

signals:
    void testCaseChanged();
    void statusChanged();
    void stepsChanged();
    void currentStepChanged(int stepIndex);

private:

    struct StepItem
    {
        QString name;
        QString status;
        QString duration;
    };

    int stepIndexOf(const QString& name) const;

    void updateStep(const StepInfo& stepInfo, const Ret& ret);

    io::path_t m_path;
    QVariantMap m_testCase;
    QList<StepItem> m_steps;
};
}

#endif // MUSE_AUTOBOT_TESTCASERUNMODEL_H
