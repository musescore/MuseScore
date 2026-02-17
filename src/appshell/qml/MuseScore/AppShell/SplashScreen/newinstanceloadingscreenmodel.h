/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"

namespace mu::appshell {
class NewInstanceLoadingScreenModel : public QObject, public muse::Contextable
{
    Q_OBJECT
    QML_ELEMENT

    muse::GlobalInject<muse::ui::IUiConfiguration> uiConfiguration;

    Q_PROPERTY(QString message READ message CONSTANT)
    Q_PROPERTY(int width READ width CONSTANT)
    Q_PROPERTY(int height READ height CONSTANT)
    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)
    Q_PROPERTY(int fontSize READ fontSize CONSTANT)
    Q_PROPERTY(QString backgroundColor READ backgroundColor CONSTANT)
    Q_PROPERTY(QString messageColor READ messageColor CONSTANT)

public:
    explicit NewInstanceLoadingScreenModel(QObject* parent = nullptr);
    explicit NewInstanceLoadingScreenModel(bool forNewScore, const QString& openingFileName, const muse::modularity::ContextPtr& ctx,
                                           QObject* parent = nullptr);

    QString message() const;
    int width() const;
    int height() const;
    QString fontFamily() const;
    int fontSize() const;
    QString backgroundColor() const;
    QString messageColor() const;

private:
    QString m_message;
    int m_width = 288;
    int m_height = 80;
};
}
