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
#include "pianoroll/ipianorollcontroller.h"
#include "pianorolltool.h"

namespace mu::pianoroll {

class PianorollGeneral;

class PianorollView : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT
public:
    enum class PianorollTool : char { SELECT, EDIT, CUT, ERASE };
    Q_ENUM(PianorollTool)

private:
    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollController, controller)

    Q_PROPERTY(double wholeNoteWidth READ wholeNoteWidth WRITE setWholeNoteWidth NOTIFY wholeNoteWidthChanged)
    Q_PROPERTY(int noteHeight READ noteHeight WRITE setNoteHeight NOTIFY noteHeightChanged)
    Q_PROPERTY(PianorollTool tool READ tool WRITE setTool NOTIFY toolChanged)

public:



    PianorollView(QQuickItem* parent = nullptr);

//    QVariant icon() const;
//    void setIcon(QVariant val);

//    bool selected() const;
//    void setSelected(bool val);

    //    bool active() const;
    //    void setActive(bool val);

    Q_INVOKABLE void load();

    QColor color() const { return m_color; }
    void setColor(QColor val) { m_color = val; }

//    PianorollGeneral* common() const { return m_common; }
//    void setCommon(PianorollGeneral* val) { m_common = val; }

    double wholeNoteWidth() const { return m_wholeNoteWidth; }
    void setWholeNoteWidth(double value);
    int noteHeight() const { return m_noteHeight; }
    void setNoteHeight(int value);
    PianorollTool tool() const { return m_tool; }
    void setTool(PianorollTool value);


    void paint(QPainter*) override;

signals:
    void wholeNoteWidthChanged();
    void noteHeightChanged();
    void toolChanged();


private:
    void onNotationChanged();
    void updateBoundingSize();
    
    notation::INotationPtr m_notation;

    QColor m_color = Qt::red;
//    PianorollGeneral* m_common = nullptr;
    double m_wholeNoteWidth;
    int m_noteHeight;
    PianorollTool m_tool = PianorollTool::SELECT;

};
}

#endif // MU_PIANOROLL_PIANOROLLVIEW_H
