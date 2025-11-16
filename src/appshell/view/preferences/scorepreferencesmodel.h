/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "notation/inotationconfiguration.h"

namespace mu::appshell {
class ScorePreferencesModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString defaultStylePath READ defaultStylePath WRITE setDefaultStylePath NOTIFY defaultStylePathChanged)
    Q_PROPERTY(QString defaultPartStylePath READ defaultPartStylePath WRITE setDefaultPartStylePath NOTIFY defaultPartStylePathChanged)

    muse::Inject<notation::INotationConfiguration> notationConfiguration = { this };

public:
    explicit ScorePreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QString defaultStylePath() const;
    void setDefaultStylePath(const QString& path);

    QString defaultPartStylePath() const;
    void setDefaultPartStylePath(const QString& path);

signals:
    void defaultStylePathChanged();
    void defaultPartStylePathChanged();
};
}
