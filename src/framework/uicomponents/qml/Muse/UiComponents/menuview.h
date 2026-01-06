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

    Q_PROPERTY(QVariant model READ model WRITE setModel NOTIFY modelChanged)

    Q_PROPERTY(int viewMargins READ viewMargins CONSTANT)

    Q_PROPERTY(int contentWidth READ contentWidth WRITE setContentWidth NOTIFY contentWidthChanged)
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight NOTIFY contentHeightChanged)

    Q_PROPERTY(Qt::AlignmentFlag cascadeAlign READ cascadeAlign WRITE setCascadeAlign NOTIFY cascadeAlignChanged)

public:
    explicit MenuView(QQuickItem* parent = nullptr);
    ~MenuView() override = default;

    QVariant model() const;
    void setModel(const QVariant& model);

    Q_INVOKABLE void setFilterText(const QString& filterText);

    int viewMargins() const;

    int contentWidth() const;
    void setContentWidth(int newContentWidth);

    int contentHeight() const;
    void setContentHeight(int newContentHeight);

    Qt::AlignmentFlag cascadeAlign() const;

public slots:
    void setCascadeAlign(Qt::AlignmentFlag cascadeAlign);

signals:
    void modelChanged();
    void cascadeAlignChanged(Qt::AlignmentFlag cascadeAlign);

private:
    void componentComplete() override;

    void updateGeometry() override;
    void updateContentPosition() override;

    QRect viewGeometry() const override;

    Qt::AlignmentFlag parentCascadeAlign(const QQuickItem* parent) const;

    QQuickItem* parentMenuContentItem() const;

private:
    QVariant m_treeModel;

    //! NOTE: We use separate models here so that we don't need to re-build filtered lists every time the
    //! filter text changes. The flattened model is built every time setModel is called, and the filtered
    //! model is set every time the text changes...
    QVariant m_flattenedModel;
    QVariant m_filteredModel;

    QString m_filterText;

    int m_contentWidth = -1;
    int m_contentHeight = -1;

    Qt::AlignmentFlag m_cascadeAlign = Qt::AlignmentFlag::AlignRight;
};
}
