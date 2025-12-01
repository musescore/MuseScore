/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <QObject>

#include "global/types/val.h"

namespace muse::uicomponents {
class TableViewCell : public QObject
{
    Q_OBJECT
    QML_ELEMENT;

    Q_PROPERTY(QVariant value READ value_property WRITE setValue_property NOTIFY valueChanged)

    Q_PROPERTY(int row READ row WRITE setRow NOTIFY rowChanged FINAL)
    Q_PROPERTY(int column READ column WRITE setColumn NOTIFY columnChanged FINAL)

    Q_PROPERTY(bool hovered READ hovered WRITE setHovered NOTIFY hoveredChanged FINAL)
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed NOTIFY pressedChanged FINAL)

public:
    explicit TableViewCell(QObject* parent = nullptr);

    Val value() const;
    void setValue(const Val& value);

    void setRequestChangeFunction(const std::function<bool(int, int, const Val&)>& func);

    QVariant value_property() const;
    void setValue_property(const QVariant& newValue);

    int row() const;
    void setRow(int row);

    bool hovered() const;
    void setHovered(bool hovered);

    bool pressed() const;
    void setPressed(bool pressed);

    int column() const;
    void setColumn(int column);

signals:
    void valueChanged();

    void rowChanged();
    void columnChanged();

    void hoveredChanged();
    void pressedChanged();

    void requestEdit();

protected:
    Val m_val;

    std::function<bool(int /*row*/, int /*column*/, const Val& /*newValue*/)> m_requestChangeFunction;

    int m_row = -1;
    int m_column = -1;

    bool m_hovered = false;
    bool m_pressed = false;
};
}
