#include "sequencercliprow.h"

#include <algorithm>
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/chord.h"
#include "libmscore/score.h"

namespace Ms {
//---------------------------------------------------------
//   SequencerClipRow
//---------------------------------------------------------

SequencerClipRow::SequencerClipRow(Staff* staff, QWidget* parent)
    : QWidget(parent),
    _staff(staff)
{
    rebuild();
}

//---------------------------------------------------------
//   ~SequencerClipRow
//---------------------------------------------------------

SequencerClipRow::~SequencerClipRow()
{
}

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

void SequencerClipRow::addChord(Chord* chord)
{
    for (Chord* c : chord->graceNotes()) {
        addChord(c);
    }
    for (Note* note : chord->notes()) {
        if (note->tieBack()) {
            continue;
        }
        int pitch = note->pitch();
        _minPitch = std::min(_minPitch, pitch);
        _maxPitch = std::max(_maxPitch, pitch);
    }
}

//---------------------------------------------------------
//   rebuild
//---------------------------------------------------------

void SequencerClipRow::rebuild()
{
    int staffIdx = _staff->idx();
    if (staffIdx == -1) {
        return;
    }

    _minPitch = 127;
    _maxPitch = 0;

    SegmentType st = SegmentType::ChordRest;
    for (Segment* s = _staff->score()->firstSegment(st); s; s = s->next1(st)) {
        for (int voice = 0; voice < VOICES; ++voice) {
            int track = voice + staffIdx * VOICES;
            Element* e = s->element(track);
            if (e && e->isChord()) {
                addChord(toChord(e));
            }
        }
    }

    Measure* lm = _staff->score()->lastMeasure();
    Fraction ticks = lm->tick() + lm->ticks();
    int width = int(ticks.toFloat() * _pixPerWhole * _xZoom);

    setMinimumSize(QSize(width, _rowHeight));
    setMaximumSize(QSize(width, _rowHeight));

    update();
}

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void SequencerClipRow::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    const int fontSize = 8;
    QFont f("FreeSans", fontSize);
    p.setFont(f);

    Part* part = _staff->part();
    int partCol = part->color();
    QColor col((partCol >> 16) & 0xff, (partCol >> 8) & 0xff, partCol & 0xff);
    int hue = col.hsvHue();

    QColor bgColor = QColor::fromHsv(hue, 220, 255);
    QColor noteColor = QColor::fromHsv(hue, 220, 80);

    p.setBrush(bgColor);
    p.fillRect(0, 0, width(), height(), bgColor);

    //p.setBrush(noteColor);
    QBrush noteBrush(noteColor);

    int staffIdx = _staff->idx();
    if (staffIdx == -1) {
        return;
    }

    SegmentType st = SegmentType::ChordRest;
    for (Segment* s = _staff->score()->firstSegment(st); s; s = s->next1(st)) {
        for (int voice = 0; voice < VOICES; ++voice) {
            int track = voice + staffIdx * VOICES;
            Element* e = s->element(track);
            if (e && e->isChord()) {
                Chord* chord = toChord(e);

                paintChord(p, noteBrush, chord);
            }
        }
    }
}

//---------------------------------------------------------
//   paintChord
//---------------------------------------------------------

void SequencerClipRow::paintChord(QPainter& p, QBrush& brush, Chord* chord)
{
    for (Chord* c : chord->graceNotes()) {
        paintChord(p, brush, c);
    }

    float noteHeight = std::max(1.0f, height() / float(_maxPitch - _minPitch + 1));

    for (Note* note : chord->notes()) {
        int pitch = note->pitch();
        QRectF noteRect(note->tick().toFloat() * _pixPerWhole * _xZoom,
                        (1 - ((pitch - _minPitch) / float(_maxPitch - _minPitch))) * height(),
                        note->playTicksFraction().toFloat() * _pixPerWhole * _xZoom,
                        1);

        p.fillRect(noteRect, brush);
    }
}
}
