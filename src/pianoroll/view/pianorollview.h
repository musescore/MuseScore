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
#include "audio/iplayback.h"
#include "notation/inotationinteraction.h"

namespace mu::pianoroll {
enum class NoteSelectType
{
    REPLACE = 0,
    XOR,
    ADD,
    SUBTRACT,
    FIRST
};

enum class DragStyle
{
    NONE = 0,
    CANCELLED,
    SELECTION_RECT,
    NOTE_POSITION,
    NOTE_LENGTH_START,
    NOTE_LENGTH_END
};

struct BarPattern
{
    QString name;
    char isWhiteKey[12];  //Set to 1 for white keys, 0 for black
};

struct NoteBlock
{
    Ms::Note* note;
    int voice;
    int staffIdx;
};

class PianorollView : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

public:
    enum class PianorollTool : char {
        SELECT, ADD, CUT, ERASE
    };
    Q_ENUM(PianorollTool)

    static const BarPattern barPatterns[];

private:
    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollController, controller)
    INJECT(pianoroll, audio::IPlayback, playback)

    Q_PROPERTY(double wholeNoteWidth READ wholeNoteWidth WRITE setWholeNoteWidth NOTIFY wholeNoteWidthChanged)
    Q_PROPERTY(double noteHeight READ noteHeight WRITE setNoteHeight NOTIFY noteHeightChanged)
    Q_PROPERTY(PianorollTool tool READ tool WRITE setTool NOTIFY toolChanged)
    Q_PROPERTY(int tuplet READ tuplet WRITE setTuplet NOTIFY tupletChanged)
    Q_PROPERTY(int subdivision READ subdivision WRITE setSubdivision NOTIFY subdivisionChanged)
    Q_PROPERTY(double centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(double centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(double displayObjectWidth READ displayObjectWidth WRITE setDisplayObjectWidth NOTIFY displayObjectWidthChanged)
    Q_PROPERTY(double displayObjectHeight READ displayObjectHeight WRITE setDisplayObjectHeight NOTIFY displayObjectHeightChanged)

public:
    static const int NUM_PITCHES = 128;
    static const int VOICES = 4;

    PianorollView(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load();

    double wholeNoteWidth() const { return m_wholeNoteWidth; }
    void setWholeNoteWidth(double value);
    int noteHeight() const { return m_noteHeight; }
    void setNoteHeight(double value);
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

    Ms::Fraction playbackPosition() { return m_playbackPosition; }
    void setPlaybackPosition(Ms::Fraction value);

    void paint(QPainter*) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;

    int wholeNoteToPixelX(Ms::Fraction tick) const { return wholeNoteToPixelX(tick.numerator() / (double)tick.denominator()); }
    int wholeNoteToPixelX(double tick) const;
    double pixelXToWholeNote(int pixelX) const;
    int pitchToPixelY(double pitch) const;
    double pixelYToPitch(int tick) const;

    void finishNoteGroupDrag();
    void selectNotes(double startTick, double endTick, double lowPitch, double highPitch, NoteSelectType selType);
    void handleSelectionClick();

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
    void playbackPositionChanged();

private:
    void onNotationChanged();
    void onCurrentNotationChanged();
    void onSelectionChanged();
    void updateBoundingSize();
    QRect boundingRect(Ms::Note* note);
    QRect boundingRect(Ms::Note* note, Ms::NoteEvent* evt);
    QString serializeSelectedNotes();

    void buildNoteData();
    void addChord(Ms::Chord* chrd, int voice, int staffIdx);

    void drawNoteBlock(QPainter* p, NoteBlock* block);
    void drawDraggedNotes(QPainter* painter);
    void drawDraggedNote(QPainter* painter, Ms::Fraction startTick, Ms::Fraction frac, int pitch, int track, QColor color);

    NoteBlock* pickNote(int pixX, int pixY);
    bool intersects(NoteBlock* block, int pixX, int pixY);
    bool intersectsPixel(NoteBlock* block, int x, int y, int width, int height);

    void insertNote(int modifiers);
    void cutChord(const QPointF& pos);
    void eraseNote(const QPointF& pos);
    Ms::Fraction roundDownToSubdivision(double wholeNote);

    void pasteNotes(const QString& copiedNotes, Ms::Fraction pasteStartTick, Ms::Fraction lengthOffset, int pitchOffset, bool xIsOffset);
    std::vector<Ms::Note*> addNote(Ms::Fraction startTick, Ms::Fraction duration, int pitch, int track);
    bool cutChordRest(Ms::ChordRest* targetCr, int track, Ms::Fraction cutTick, Ms::ChordRest*& cr0, Ms::ChordRest*& cr1);
    std::vector<Ms::Note*> getSegmentNotes(Ms::Segment* seg, int track);

    static void append(std::vector<Ms::Note*>& a, const std::vector<Ms::Note*>& b)
    {
        a.insert(a.end(), b.begin(), b.end());
    }

    Ms::Score* score();
    Ms::Staff* activeStaff();

    notation::INotationPtr m_notation;

    QList<NoteBlock*> m_noteList;
    std::vector<int> m_selectedStaves;
    int m_activeStaff;

    double m_centerX = 0;  //fraction of note grid camera is focused on
    double m_centerY = 0;  //fraction of note grid camera is focused on

    double m_displayObjectWidth = 0;  //Set to note grid in pixels
    double m_displayObjectHeight = 0;  //Set to note grid in pixels

    double m_wholeNoteWidth;
    double m_noteHeight;
    int m_tuplet = 1;
    int m_subdivision = 0;
    PianorollTool m_tool = PianorollTool::SELECT;
    int m_barPattern = 0;

    int m_editNoteVoice = 0;  //Voice to use when adding notes
    Ms::Fraction m_editNoteLength = Ms::Fraction(1, 4);  //Length of note used when adding notes

    bool m_mouseDown;
    bool m_dragStarted;
    QString m_dragNoteCache;
    QPointF m_mouseDownPos;
    QPointF m_lastMousePos;
    QPointF m_popupMenuPos;
    DragStyle m_dragStyle;
    int m_dragStartPitch;
    Ms::Fraction m_dragStartTick;
    Ms::Fraction m_dragEndTick;
    int m_dragNoteLengthMargin = 4;
    bool m_inProgressUndoEvent;

    Ms::Fraction m_playbackPosition;

    QColor m_colorBackground = Qt::gray;
    QColor m_colorKeyWhite = QColor(0xffffff);
    QColor m_colorKeyBlack = QColor(0xe6e6e6);
    QColor m_colorKeyHighlight = QColor(0xaaaaff);
    QColor m_colorSelectionBox = QColor(0x2085c3);
    QColor m_colorPlaybackLine = QColor(0xff0000);
    QColor m_colorGridLine = QColor(0xa2a2a6);
    QColor m_colorNoteSel = QColor(0xffff00);
    QColor m_colorNoteVoice1 = QColor(0x9bcdff);
    QColor m_colorNoteVoice2 = QColor(0x80d580);
    QColor m_colorNoteVoice3 = QColor(0xffac85);
    QColor m_colorNoteVoice4 = QColor(0xff94db);

    QColor m_colorNoteDrag = QColor(0xffbb33);
    QColor m_colorText = QColor(0x111111);
    QColor m_colorTie = QColor(0xff0000);
};
}

#endif // MU_PIANOROLL_PIANOROLLVIEW_H
