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

#ifndef MU_PIANOROLL_PIANOROLLAUTOMATIONEDITOR_H
#define MU_PIANOROLL_PIANOROLLAUTOMATIONEDITOR_H

#include <QQuickPaintedItem>
#include <QPixmap>
#include <QColor>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "pianoroll/ipianorollcontroller.h"
#include "audio/iplayback.h"

namespace mu::pianoroll {
class IPianorollAutomationModel;
struct NoteEventBlock;

class PianorollAutomationNote : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollController, controller)
    INJECT(playback, audio::IPlayback, playback)

    Q_PROPERTY(double wholeNoteWidth READ wholeNoteWidth WRITE setWholeNoteWidth NOTIFY wholeNoteWidthChanged)
    Q_PROPERTY(double centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(double displayObjectWidth READ displayObjectWidth WRITE setDisplayObjectWidth NOTIFY displayObjectWidthChanged)
    Q_PROPERTY(int tuplet READ tuplet WRITE setTuplet NOTIFY tupletChanged)
    Q_PROPERTY(int subdivision READ subdivision WRITE setSubdivision NOTIFY subdivisionChanged)
    Q_PROPERTY(AutomationType automationType READ automationType WRITE setAutomationType NOTIFY automationTypeChanged)

public:
    enum class AutomationType {
        DURATION, DURATION_MULT, VELOCITY, VELOCITY_ABS, POSITION, EXPRESSION, PAN
    };
    Q_ENUM(AutomationType)

    PianorollAutomationNote(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load();

    double wholeNoteWidth() const { return m_wholeNoteWidth; }
    void setWholeNoteWidth(double value);
    double centerX() const { return m_centerX; }
    void setCenterX(double value);
    double displayObjectWidth() const { return m_displayObjectWidth; }
    void setDisplayObjectWidth(double value);
    int tuplet() const { return m_tuplet; }
    void setTuplet(int value);
    int subdivision() const { return m_subdivision; }
    void setSubdivision(int value);
    engraving::Fraction playbackPosition() { return m_playbackPosition; }
    void setPlaybackPosition(engraving::Fraction value);
    AutomationType automationType() { return m_automationType; }
    void setAutomationType(AutomationType value);

    void paint(QPainter*) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

    int wholeNoteToPixelX(engraving::Fraction tick) const { return wholeNoteToPixelX(tick.numerator() / (double)tick.denominator()); }
    int wholeNoteToPixelX(double tick) const;
    double pixelXToWholeNote(int pixelX) const;

    engraving::Score* score();
    engraving::Staff* activeStaff();

signals:
    void wholeNoteWidthChanged();
    void centerXChanged();
    void displayObjectWidthChanged();
    void playbackPositionChanged();
    void automationTypeChanged();
    void tupletChanged();
    void subdivisionChanged();

private:
    void onNotationChanged();
    void onCurrentNotationChanged();
    void onSelectionChanged();
    void updateBoundingSize();

    void buildNoteData();
    void addChord(engraving::Chord* chrd, int voice, int staffIdx);

    double pixYToValue(double pixY, double valMin, double valMax);
    double valueToPixY(double value, double valMin, double valMax);
    NoteEventBlock* pickBlock(QPointF point);

    enum DragStyle {
        NONE,
        OFFSET
    };
    DragStyle m_dragStyle = DragStyle::NONE;

    IPianorollAutomationModel* lookupModel(AutomationType type);

    static std::vector<IPianorollAutomationModel*> m_automationModels;

    int m_activeStaff;
    std::vector<int> m_selectedStaves;

    //Note data for drawing in levels mode
    std::vector<NoteEventBlock*> m_noteLevels;

    AutomationType m_automationType = AutomationType::VELOCITY;

    bool m_mouseDown = false;
    bool m_dragging = false;
    QPoint m_mouseDownPos;
    QPoint m_lastMousePos;
    int m_pickRadius = 4;
    NoteEventBlock* m_dragBlock = nullptr;

    double m_centerX = 0;  //fraction of note grid camera is focused on
    double m_displayObjectWidth = 0;  //Set to note grid in pixels
    double m_wholeNoteWidth;

    double m_marginY = 8;

    int m_tuplet = 1;
    int m_subdivision = 0;

    engraving::Fraction m_playbackPosition;

    const int m_vertexRadius = 4;

    QColor m_colorBackground = Qt::gray;
    QColor m_colorGridBackground = QColor(0xdddddd);
    QColor m_colorPlaybackLine = QColor(0xff0000);
    QColor m_colorGridLine = QColor(0xa2a2a6);
    QColor m_colorText = Qt::black;
    QColor m_colorVertexFill = QColor(0xffffff);
    QColor m_colorVertexLine = Qt::black;
    QColor m_colorGraphFill = QColor(0x80, 0x9b, 0xcd, 0x80);
};
}

#endif // MU_PIANOROLL_PIANOROLLAUTOMATIONEDITOR_H
