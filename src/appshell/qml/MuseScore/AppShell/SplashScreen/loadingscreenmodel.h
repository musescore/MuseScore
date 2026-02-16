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
#include "global/iglobalconfiguration.h"
#include "languages/ilanguagesservice.h"
#include "global/iapplication.h"

namespace mu::appshell {
class LoadingScreenModel : public QObject, public muse::Contextable
{
    Q_OBJECT
    QML_ELEMENT

    muse::GlobalInject<muse::ui::IUiConfiguration> uiConfiguration;
    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;
    muse::GlobalInject<muse::languages::ILanguagesService> languagesService;
    muse::ContextInject<muse::IApplication> application = { this };

    Q_PROPERTY(QString message READ message CONSTANT)
    Q_PROPERTY(QString website READ website CONSTANT)
    Q_PROPERTY(QString versionString READ versionString CONSTANT)
    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)
    Q_PROPERTY(int fontSize READ fontSize CONSTANT)
    Q_PROPERTY(bool isRtl READ isRtl CONSTANT)

public:
    explicit LoadingScreenModel(QObject* parent = nullptr);
    explicit LoadingScreenModel(const muse::modularity::ContextPtr& ctx, QObject* parent = nullptr);

    QString message() const;
    QString website() const;
    QString versionString() const;
    QString fontFamily() const;
    int fontSize() const;
    bool isRtl() const;
};
}
