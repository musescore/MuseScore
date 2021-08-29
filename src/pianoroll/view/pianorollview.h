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

//class PianorollGeneral;


struct BarPattern {
      QString name;
      char isWhiteKey[12];  //Set to 1 for white keys, 0 for black
      };


class PianorollView : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT
public:
    enum class PianorollTool : char { SELECT, EDIT, CUT, ERASE };
    Q_ENUM(PianorollTool)

    static const BarPattern barPatterns[];

private:
    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollController, controller)

    Q_PROPERTY(double wholeNoteWidth READ wholeNoteWidth WRITE setWholeNoteWidth NOTIFY wholeNoteWidthChanged)
    Q_PROPERTY(int noteHeight READ noteHeight WRITE setNoteHeight NOTIFY noteHeightChanged)
    Q_PROPERTY(PianorollTool tool READ tool WRITE setTool NOTIFY toolChanged)
    Q_PROPERTY(int tuplet READ tuplet WRITE setTuplet NOTIFY tupletChanged)
    Q_PROPERTY(int subdivision READ subdivision WRITE setSubdivision NOTIFY subdivisionChanged)
    Q_PROPERTY(double centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(double centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(double displayObjectWidth READ displayObjectWidth WRITE setDisplayObjectWidth NOTIFY displayObjectWidthChanged)
    Q_PROPERTY(double displayObjectHeight READ displayObjectHeight WRITE setDisplayObjectHeight NOTIFY displayObjectHeightChanged)

public:

    PianorollView(QQuickItem* parent = nullptr);


    Q_INVOKABLE void load();


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
    double centerX() const { return m_centerX; }
    void setCenterX(double value);
    double centerY() const { return m_centerY; }
    void setCenterY(double value);
    double displayObjectWidth() const { return m_displayObjectWidth; }
    void setDisplayObjectWidth(double value);
    double displayObjectHeight() const { return m_displayObjectHeight; }
    void setDisplayObjectHeight(double value);


    void paint(QPainter*) override;

    int wholeNoteToPixelX(Ms::Fraction tick) const { return wholeNoteToPixelX(tick.numerator() / (double)tick.denominator()); }
    int wholeNoteToPixelX(double tick) const;
    double pixelXToWholeNote(int pixelX) const;
    int pitchToPixelY(double pitch) const;
    double pixelYToPitch(int tick) const;

signals:
    void wholeNoteWidthChanged();
    void noteHeightChanged();
    void toolChanged();
    void tupletChanged();
    void subdivisionChanged();
    void centerXChanged();
    void centerYChanged();
    void displayObjectWidthChanged();
    void displayObjectHeightChanged();


private:
    void onNotationChanged();
    void onCurrentNotationChanged();
    void onSelectionChanged();
    void updateBoundingSize();
    QRect boundingRect(Ms::Note* note, Ms::NoteEvent* evt = nullptr);

    void drawChord(QPainter* p, Ms::Chord* chrd, int voice, bool active);
    
    notation::INotationPtr m_notation;



    double m_centerX = 0;  //fraction of note grid camera is focused on
    double m_centerY = 0;  //fraction of note grid camera is focused on

    double m_displayObjectWidth = 0;  //Set to note grid in pixels
    double m_displayObjectHeight = 0;  //Set to note grid in pixels

    double m_wholeNoteWidth;
    int m_noteHeight;
    int m_tuplet = 1;
    int m_subdivision = 0;
    PianorollTool m_tool = PianorollTool::SELECT;
    int m_barPattern = 0;

    quint8 m_pitchHighlight[128];

    QColor m_colorBackground = Qt::gray;
    QColor m_colorKeyWhite = QColor(0xffffff);
    QColor m_colorKeyBlack = QColor(0xe6e6e6);
    QColor m_colorKeyHighlight = QColor(0xaaaaff);
    QColor m_colorSelectionBox = QColor(0x2085c3);
    QColor m_colorGridLine = QColor(0xa2a2a6);
    QColor m_colorNoteSel = QColor(0xffff00);
    QColor m_colorNoteVoice1 = QColor(0x9bcdff);
    QColor m_colorNoteVoice2 = QColor(0x80d580);
    QColor m_colorNoteVoice3 = QColor(0xffac85);
    QColor m_colorNoteVoice4 = QColor(0xff94db);
//    QColor m_colorNoteGhost = QColor(0x1dcca0);

    QColor m_colorNoteDrag = QColor(0xffbb33);
    QColor m_colorText = QColor(0x111111);
    QColor m_colorTie = QColor(0xff0000);


    const int NUM_PITCHES = 128;
    const int VOICES = 4;

};
}

#endif // MU_PIANOROLL_PIANOROLLVIEW_H
