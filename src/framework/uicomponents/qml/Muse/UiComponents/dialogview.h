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

#pragma once

#include <qqmlintegration.h>

#include "types/ret.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "ui/iwindowscontroller.h"

#include "windowview.h"

namespace muse::uicomponents {
class DialogView : public WindowView
{
    Q_OBJECT
    QML_ELEMENT;

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString objectId READ objectId WRITE setObjectId NOTIFY objectIdChanged)
    Q_PROPERTY(bool modal READ modal WRITE setModal NOTIFY modalChanged)
    Q_PROPERTY(bool frameless READ frameless WRITE setFrameless NOTIFY framelessChanged)
    Q_PROPERTY(bool resizable READ resizable WRITE setResizable NOTIFY resizableChanged)
    Q_PROPERTY(bool alwaysOnTop READ alwaysOnTop WRITE setAlwaysOnTop NOTIFY alwaysOnTopChanged)
    Q_PROPERTY(QVariantMap ret READ ret WRITE setRet NOTIFY retChanged)

    Inject<IApplication> application = { this };
    Inject<ui::IWindowsController> windowsController = { this };

public:
    explicit DialogView(QQuickItem* parent = nullptr);
    ~DialogView() override = default;

    QString title() const;
    void setTitle(QString title);

    QString objectId() const;
    void setObjectId(QString objectId);

    bool modal() const;
    void setModal(bool modal);

    bool frameless() const;
    void setFrameless(bool frameless);

    bool resizable() const;
    void setResizable(bool resizable);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool alwaysOnTop);

    QVariantMap ret() const;
    void setRet(QVariantMap ret);
    void setRetCode(Ret::Code code);

    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();
    Q_INVOKABLE void raise();
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject(int code = -1);

signals:
    void titleChanged(QString title);
    void objectIdChanged(QString objectId);
    void modalChanged(bool modal);
    void framelessChanged(bool frameless);
    void resizableChanged(bool resizable);
    void alwaysOnTopChanged();
    void retChanged(QVariantMap ret);

private:
    void initView() override;

    void beforeOpen() override;
    void onHidden() override;

    void updateGeometry() override;
    QRect viewGeometry() const override;

    QString m_objectId;
    QString m_title;
    bool m_modal = true;
    bool m_frameless = false;
    bool m_alwaysOnTop = false;
    QVariantMap m_ret;
};
}
