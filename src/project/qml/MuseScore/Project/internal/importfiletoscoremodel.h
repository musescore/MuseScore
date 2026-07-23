/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"

namespace mu::project {
class ImportFileToScoreModel : public QObject, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT

    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

    QML_ELEMENT

    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    explicit ImportFileToScoreModel(QObject* parent = nullptr);

    QString errorMessage() const;

    Q_INVOKABLE QStringList selectFiles();
    Q_INVOKABLE bool checkFiles(const QStringList& pathsOrUrls);
    Q_INVOKABLE QStringList localPaths(const QStringList& pathsOrUrls) const;

signals:
    void errorMessageChanged();

private:
    void setErrorMessage(const QString& message);

    QString m_errorMessage;
};
}
