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

#include "popupview.h"

namespace muse::uicomponents {
class MenuView : public PopupView
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(bool isSearchable READ isSearchable WRITE setIsSearchable NOTIFY isSearchableChanged)
    Q_PROPERTY(bool isSearching READ isSearching WRITE setIsSearching NOTIFY isSearchingChanged)

    Q_PROPERTY(int viewMargins READ viewMargins CONSTANT)

    Q_PROPERTY(int contentWidth READ contentWidth WRITE setContentWidth NOTIFY contentWidthChanged)
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight NOTIFY contentHeightChanged)

    Q_PROPERTY(int desiredHeight READ desiredHeight WRITE setDesiredHeight NOTIFY desiredHeightChanged)
    Q_PROPERTY(int desiredWidth READ desiredWidth WRITE setDesiredWidth NOTIFY desiredWidthChanged)

    Q_PROPERTY(Qt::AlignmentFlag cascadeAlign READ cascadeAlign WRITE setCascadeAlign NOTIFY cascadeAlignChanged)

public:
    explicit MenuView(QQuickItem* parent = nullptr);
    ~MenuView() override = default;

    bool isSearchable() const;
    void setIsSearchable(bool isSearchable);

    bool isSearching() const;
    void setIsSearching(bool isSearching);

    int viewMargins() const;

    int contentWidth() const;
    void setContentWidth(int newContentWidth);

    int contentHeight() const;
    void setContentHeight(int newContentHeight);

    int desiredHeight() const;
    void setDesiredHeight(int desiredHeight);

    int desiredWidth() const;
    void setDesiredWidth(int desiredWidth);

    Qt::AlignmentFlag cascadeAlign() const;

public slots:
    void setCascadeAlign(Qt::AlignmentFlag cascadeAlign);

signals:
    void isSearchableChanged();
    void isSearchingChanged();

    void desiredHeightChanged();
    void desiredWidthChanged();

    void cascadeAlignChanged(Qt::AlignmentFlag cascadeAlign);

private:
    void componentComplete() override;

    void updateGeometry() override;
    void updateContentPosition() override;

    QRect viewGeometry() const override;

    Qt::AlignmentFlag parentCascadeAlign(const QQuickItem* parent) const;

    QQuickItem* parentMenuContentItem() const;

private:
    bool m_isSearchable = false;
    bool m_isSearching = false;

    int m_contentWidth = -1;
    int m_contentHeight = -1;

    int m_desiredHeight = -1;
    int m_desiredWidth = -1;

    Qt::AlignmentFlag m_cascadeAlign = Qt::AlignmentFlag::AlignRight;
};
}
