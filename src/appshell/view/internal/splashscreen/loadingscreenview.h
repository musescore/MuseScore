/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include <QWidget>

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "languages/ilanguagesservice.h"
#include "global/iapplication.h"

class QSvgRenderer;

namespace mu::appshell {
class LoadingScreenView : public QWidget, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };
    muse::Inject<muse::languages::ILanguagesService> languagesService = { this };
    muse::Inject<muse::IApplication> application = { this };

public:
    explicit LoadingScreenView(QWidget* parent = nullptr);

private:
    bool event(QEvent* event) override;
    void draw(QPainter* painter);

    QString m_message;
    QSvgRenderer* m_backgroundRenderer = nullptr;
};
}
