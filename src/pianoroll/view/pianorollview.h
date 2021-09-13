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

#ifndef MU_PIANOROLL_PIANOROLLVIEW_H
#define MU_PIANOROLL_PIANOROLLVIEW_H

#include <QQuickPaintedItem>
#include <QIcon>
#include <QColor>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"

namespace mu::pianoroll {
class PianoRollView : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

    INJECT(pianoroll, context::IGlobalContext, globalContext)

//    Q_PROPERTY(QVariant icon READ icon WRITE setIcon)
//    Q_PROPERTY(bool selected READ selected WRITE setSelected)
//    Q_PROPERTY(bool active READ active WRITE setActive)
    Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    PianoRollView(QQuickItem* parent = nullptr);

//    QVariant icon() const;
//    void setIcon(QVariant val);

//    bool selected() const;
//    void setSelected(bool val);

    //    bool active() const;
    //    void setActive(bool val);

    Q_INVOKABLE void load();

    QColor color() const { return m_color; }
    void setColor(QColor val) { m_color = val; }

    void paint(QPainter*) override;

private:
    void onCurrentNotationChanged();

    notation::INotationPtr m_notation;

    QColor m_color = Qt::red;
//    QIcon m_icon;
//    bool m_selected { false };
//    bool m_active   { false };
};
}

#endif // MU_PIANOROLL_PIANOROLLVIEW_H
