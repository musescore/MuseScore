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
    Q_PROPERTY(int tuplet READ tuplet WRITE setTuplet NOTIFY tupletChanged)
    Q_PROPERTY(int subdivision READ subdivision WRITE setSubdivision NOTIFY subdivisionChanged)

public:



    PianorollView(QQuickItem* parent = nullptr);

//    QVariant icon() const;
//    void setIcon(QVariant val);

//    bool selected() const;
//    void setSelected(bool val);

    //    bool active() const;
    //    void setActive(bool val);

    Q_INVOKABLE void load();

//    QColor color() const { return m_color; }
//    void setColor(QColor val) { m_color = val; }

//    PianorollGeneral* common() const { return m_common; }
//    void setCommon(PianorollGeneral* val) { m_common = val; }

    double wholeNoteWidth() const { return m_wholeNoteWidth; }
    void setWholeNoteWidth(double value);
    int noteHeight() const { return m_noteHeight; }
    void setNoteHeight(int value);
    PianorollTool tool() const { return m_tool; }
    void setTool(PianorollTool value);
    int tuplet() const { return m_tuplet; }
    void setTuplet(int value);
    int subdivision() const { return m_subdivision; }
    void setSubdivision(int value);


    void paint(QPainter*) override;
    int tickToPixelX(int tick);
    int pixelXToTick(int tick);

signals:
    void wholeNoteWidthChanged();
    void noteHeightChanged();
    void toolChanged();
    void tupletChanged();
    void subdivisionChanged();


private:
    void onNotationChanged();
    void onCurrentNotationChanged();
    void onSelectionChanged();
    void updateBoundingSize();
    
    notation::INotationPtr m_notation;

//    QColor m_color = Qt::red;
//    PianorollGeneral* m_common = nullptr;
    double m_wholeNoteWidth;
    int m_noteHeight;
    int m_tuplet = 1;
    int m_subdivision = 0;
    PianorollTool m_tool = PianorollTool::SELECT;

    QColor m_colorKeyWhite = QColor(0xffffff);
    QColor m_colorKeyBlack = QColor(0xe6e6e6);
    QColor m_colorKeyHighlight = QColor(0xaaaaff);
    QColor m_colorSelectionBox = QColor(0x2085c3);
    QColor m_colorGridLine = QColor(0xa2a2a6);
    QColor m_colorNoteUnsel = QColor(0x1dcca0);
    QColor m_colorNoteSel = QColor(0xffff00);
    QColor m_colorNoteDrag = QColor(0xffbb33);
    QColor m_colorText = QColor(0x111111);
    QColor m_colorTie = QColor(0xff0000);


    const int NUM_PITCHES = 128;

};
}

#endif // MU_PIANOROLL_PIANOROLLVIEW_H
