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

#ifndef MUSE_UICOMPONENTS_DIALOGVIEW_H
#define MUSE_UICOMPONENTS_DIALOGVIEW_H

#include <QEventLoop>

#include "popupview.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"

namespace muse::uicomponents {
class DialogView : public PopupView
{
    Q_OBJECT

    Inject<IApplication> application;

public:
    explicit DialogView(QQuickItem* parent = nullptr);
    ~DialogView() override = default;

    Q_INVOKABLE void exec();
    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();
    Q_INVOKABLE void raise();
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject(int code = -1);

private:
    bool isDialog() const override;
    void beforeOpen() override;
    void onHidden() override;

    QScreen* resolveScreen() const override;

    void updateGeometry() override;

    QRect viewGeometry() const override;

    QEventLoop m_loop;
};
}

#endif // MUSE_UICOMPONENTS_DIALOGVIEW_H
