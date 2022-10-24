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
    NOTE_LENGTH_END,
    DRAW_NOTE,
    EVENT_ONTIME,
    EVENT_MOVE,
    EVENT_LENGTH,
    MOVE_VIEWPORT
};

struct BarPattern
{
    QString name;
    char isWhiteKey[12];  //Set to 1 for white keys, 0 for black
};

struct NoteBlock
{
    engraving::Note* note;
    int voice;
    int staffIdx;
};

class PianorollView : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

public:
    enum class PianorollTool : char {
        SELECT, ADD, CUT, ERASE, EVENT_ADJUST
    };
    Q_ENUM(PianorollTool)

private:
    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollController, controller)
    INJECT(pianoroll, audio::IPlayback, playback)

    Q_PROPERTY(double wholeNoteWidth READ wholeNoteWidth WRITE setWholeNoteWidth NOTIFY wholeNoteWidthChanged)
    Q_PROPERTY(double noteHeight READ noteHeight WRITE setNoteHeight NOTIFY noteHeightChanged)
    Q_PROPERTY(PianorollTool tool READ tool WRITE setTool NOTIFY toolChanged)
    Q_PROPERTY(int tuplet READ tuplet WRITE setTuplet NOTIFY tupletChanged)
    Q_PROPERTY(int subdivision READ subdivision WRITE setSubdivision NOTIFY subdivisionChanged)
    Q_PROPERTY(int stripePattern READ stripePattern WRITE setStripePattern NOTIFY stripePatternChanged)
    Q_PROPERTY(double centerX READ centerX WRITE setCenterX NOTIFY centerXChanged)
    Q_PROPERTY(double centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(double displayObjectWidth READ displayObjectWidth WRITE setDisplayObjectWidth NOTIFY displayObjectWidthChanged)
    Q_PROPERTY(double displayObjectHeight READ displayObjectHeight WRITE setDisplayObjectHeight NOTIFY displayObjectHeightChanged)

public:
    static const int NUM_PITCHES = 128;
    static const int VOICES = 4;

    PianorollView(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load();

    bool displayEventAdjustment() { return m_tool == PianorollTool::EVENT_ADJUST; }
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
    int stripePattern() const { return m_stripePattern; }
    void setStripePattern(int value);
    double centerX() const { return m_centerX; }
    void setCenterX(double value);
    double centerY() const { return m_centerY; }
    void setCenterY(double value);
    double displayObjectWidth() const { return m_displayObjectWidth; }
    void setDisplayObjectWidth(double value);
    double displayObjectHeight() const { return m_displayObjectHeight; }
    void setDisplayObjectHeight(double value);

    engraving::Fraction playbackPosition() { return m_playbackPosition; }
    void setPlaybackPosition(engraving::Fraction value);

    void paint(QPainter*) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void wheelEvent(QWheelEvent* event);

    int wholeNoteToPixelX(engraving::Fraction tick) const { return wholeNoteToPixelX(tick.numerator() / (double)tick.denominator()); }
    int wholeNoteToPixelX(double tick) const;
    double pixelXToWholeNote(int pixelX) const;
    int pitchToPixelY(double pitch) const;
    double pixelYToPitch(int tick) const;

    void finishNoteGroupDrag();
    void finishNoteEventAdjustDrag();
    void selectNotes(double startTick, double endTick, double lowPitch, double highPitch, NoteSelectType selType);
    void handleSelectionClick();

signals:
    void wholeNoteWidthChanged();
    void noteHeightChanged();
    void toolChanged();
    void tweaksChanged();
    void tupletChanged();
    void subdivisionChanged();
    void stripePatternChanged();
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
    QRect boundingRect(engraving::Note* note, bool applyEvents = false);
    QRect boundingRect(engraving::Note* note, engraving::NoteEvent* evt, bool applyEvents = false);
    QString serializeSelectedNotes();

    void buildNoteData();
    void addChord(engraving::Chord* chrd, int voice, int staffIdx);

    void drawNoteBlock(QPainter* p, NoteBlock* block);
    void drawDraggedNotes(QPainter* painter);
    void drawDraggedNote(QPainter* painter, engraving::Fraction startTick, engraving::Fraction frac, int pitch, int track, QColor color);

    NoteBlock* pickNote(int pixX, int pixY);
    bool intersects(NoteBlock* block, int pixX, int pixY);
    bool intersectsPixel(NoteBlock* block, int x, int y, int width, int height);

    void insertNote(int modifiers);
    void cutChord(const QPointF& pos);
    void eraseNote(const QPointF& pos);
    engraving::Fraction roundToSubdivision(double wholeNote, bool down = true);

    void pasteNotes(const QString& copiedNotes, engraving::Fraction pasteStartTick, engraving::Fraction lengthOffset, int pitchOffset,
                    bool xIsOffset);
    std::vector<engraving::Note*> addNote(engraving::Fraction startTick, engraving::Fraction duration, int pitch, int track);
    bool cutChordRest(engraving::ChordRest* targetCr, int track, engraving::Fraction cutTick, engraving::ChordRest*& cr0,
                      engraving::ChordRest*& cr1);
    std::vector<engraving::Note*> getSegmentNotes(engraving::Segment* seg, int track);

    static void append(std::vector<engraving::Note*>& a, const std::vector<engraving::Note*>& b)
    {
        a.insert(a.end(), b.begin(), b.end());
    }

    engraving::Score* score();
    engraving::Staff* activeStaff();

    notation::INotationPtr m_notation;

    QList<NoteBlock*> m_noteList;
    std::vector<int> m_selectedStaves;
    int m_activeStaff;

    double m_centerX = 0;  //viewport center of focus
    double m_centerY = 0;  //viewport center of focus
    QPointF m_dragViewportStart;

    double m_displayObjectWidth = 0;  //Set to note grid in pixels
    double m_displayObjectHeight = 0;  //Set to note grid in pixels

    double m_wholeNoteWidth;
    double m_noteHeight;
    int m_tuplet = 1;
    int m_subdivision = 0;
    int m_stripePattern = 0b101010110101;  //bitflag indicating which rows are indicated as white keys
    PianorollTool m_tool = PianorollTool::SELECT;

    int m_editNoteVoice = 0;  //Voice to use when adding notes
    engraving::Fraction m_editNoteLength = engraving::Fraction(1, 4);  //Length of note used when adding notes

    bool m_mouseDown;
    bool m_dragStarted;
    QString m_dragNoteCache;
    QPointF m_mouseDownPos;
    QPointF m_lastMousePos;
    QPointF m_popupMenuPos;
    DragStyle m_dragStyle;
    int m_dragStartPitch;
    engraving::Fraction m_dragStartTick;
    engraving::Fraction m_dragEndTick;
    int m_dragNoteLengthMargin = 4;
    bool m_inProgressUndoEvent;

    engraving::Fraction m_playbackPosition;

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

    QColor m_colorTweaks = QColor(0xfd63fcc);
    QColor m_colorNoteDrag = QColor(0xffbb33);
    QColor m_colorText = QColor(0x111111);
    QColor m_colorTie = QColor(0xff0000);

    float m_noteRectRoundedRadius = 3;
};
}

#endif // MU_PIANOROLL_PIANOROLLVIEW_H
