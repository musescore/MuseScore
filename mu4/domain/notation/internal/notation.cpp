//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "notation.h"

#include <QPointF>
#include <QPainter>

#include "config.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/shadownote.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"
#include "libmscore/rest.h"
#include "libmscore/slur.h"
#include "libmscore/system.h"
#include "libmscore/chord.h"
#include "libmscore/elementgroup.h"

#include "scorecallbacks.h"

#ifdef BUILD_UI_MU4
//! HACK Temporary hack to link libmscore
Q_LOGGING_CATEGORY(undoRedo, "undoRedo", QtCriticalMsg)

namespace Ms {
QString revision;
MasterSynthesizer* synti;
QString dataPath;
QString mscoreGlobalShare;
}
//! ---------
#endif

using namespace mu::domain::notation;
using namespace Ms;

Notation::Notation()
{
    m_scoreGlobal = new MScore(); //! TODO May be static?
    m_score = new MasterScore(m_scoreGlobal->baseStyle());
    m_scoreCallbacks = new ScoreCallbacks();

    m_shadowNote = new ShadowNote(m_score);
    m_shadowNote->setVisible(false);

    m_inputState = new NotationInputState(this);
    m_selection = new NotationSelection(this);
    m_inputController = new NotationInputController(this);

    m_inputController->dragChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });
}

Notation::~Notation()
{
    delete m_inputState;
    delete m_selection;
    delete m_shadowNote;
    delete m_scoreCallbacks;
    delete m_score;
}

void Notation::init()
{
    MScore::init();         // initialize libmscore
}

bool Notation::load(const std::string& path)
{
    Score::FileError rv = m_score->loadMsc(QString::fromStdString(path), true);
    if (rv != Score::FileError::FILE_NO_ERROR) {
        return false;
    }

    m_score->setUpdateAll();
    m_score->doLayout();

    return true;
}

void Notation::setViewSize(const QSizeF& vs)
{
    m_viewSize = vs;
}

void Notation::paint(QPainter* p, const QRect& r)
{
    const QList<Ms::Page*>& mspages = m_score->pages();

    if (mspages.isEmpty()) {
        p->drawText(10, 10, "no pages");
        return;
    }

    Ms::Page* page = mspages.first();
    page->draw(p);

    p->fillRect(page->bbox(), QColor("#ffffff"));

    QList<Ms::Element*> ell = page->elements();
    for (const Ms::Element* e : ell) {
        if (!e->visible()) {
            continue;
        }

        e->itemDiscovered = false;
        QPointF pos(e->pagePos());
        //LOGI() << e->name() << ", x: " << pos.x() << ", y: " << pos.y() << "\n";

        p->translate(pos);

        e->draw(p);

        p->translate(-pos);
    }
}

INotationInputState* Notation::inputState() const
{
    return m_inputState;
}

void Notation::startNoteEntry()
{
    //! NOTE Coped from `void ScoreView::startNoteEntry()`
    Ms::InputState& is = score()->inputState();
    is.setSegment(0);

    if (score()->selection().isNone()) {
        selectFirstTopLeftOrLast();
    }

    //! TODO Find out what does and why.
    Element* el = score()->selection().element();
    if (!el) {
        el = score()->selection().firstChordRest();
    }

    if (el == nullptr
        || (el->type() != ElementType::CHORD && el->type() != ElementType::REST && el->type() != ElementType::NOTE)) {
        // if no note/rest is selected, start with voice 0
        int track = is.track() == -1 ? 0 : (is.track() / VOICES) * VOICES;
        // try to find an appropriate measure to start in
        Fraction tick = el ? el->tick() : Fraction(0,1);
        el = score()->searchNote(tick, track);
        if (!el) {
            el = score()->searchNote(Fraction(0,1), track);
        }
    }

    if (!el) {
        return;
    }

    if (el->type() == ElementType::CHORD) {
        Chord* c = static_cast<Chord*>(el);
        Note* note = c->selectedNote();
        if (note == 0) {
            note = c->upNote();
        }
        el = note;
    }
    //! ---

    TDuration d(is.duration());
    if (!d.isValid() || d.isZero() || d.type() == TDuration::DurationType::V_MEASURE) {
        is.setDuration(TDuration(TDuration::DurationType::V_QUARTER));
    }
    is.setAccidentalType(AccidentalType::NONE);

    select(el, SelectType::SINGLE, 0);

    is.setRest(false);
    is.setNoteEntryMode(true);

    //! TODO Find out why.
    score()->setUpdateAll();
    score()->update();
    //! ---

    Staff* staff = score()->staff(is.track() / VOICES);
    switch (staff->staffType(is.tick())->group()) {
    case StaffGroup::STANDARD:
        break;
    case StaffGroup::TAB: {
        int strg = 0;                           // assume topmost string as current string
        // if entering note entry with a note selected and the note has a string
        // set InputState::_string to note physical string
        if (el->type() == ElementType::NOTE) {
            strg = (static_cast<Note*>(el))->string();
        }
        is.setString(strg);
        break;
    }
    case StaffGroup::PERCUSSION:
        break;
    }

    m_inputStateChanged.notify();
}

void Notation::selectFirstTopLeftOrLast()
{
    // choose page in current view (favor top left quadrant if possible)
    // select first (top/left) chordrest of that page in current view
    // or, CR at last selected position if that is in view
    Page* page = nullptr;
    QList<QPointF> points;
    points.append(QPoint(m_viewSize.width() * 0.25, m_viewSize.height() * 0.25));
    points.append(QPoint(0.0, 0.0));
    points.append(QPoint(0.0, m_viewSize.height()));
    points.append(QPoint(m_viewSize.width(), 0.0));
    points.append(QPoint(m_viewSize.width(), m_viewSize.height()));
    for (const QPointF& point : points) {
        page = m_inputController->point2page(point);
        if (page) {
            break;
        }
    }

    if (page) {
        ChordRest* topLeft = nullptr;
        qreal tlY = 0.0;
        Fraction tlTick = Fraction(0,1);
        QRectF viewRect  = QRectF(0.0, 0.0, m_viewSize.width(), m_viewSize.height());
        QRectF pageRect  = page->bbox().translated(page->x(), page->y());
        QRectF intersect = viewRect & pageRect;
        intersect.translate(-page->x(), -page->y());
        QList<Element*> el = page->items(intersect);
        ChordRest* lastSelected = score()->selection().currentCR();
        for (Element* e : el) {
            // loop through visible elements
            // looking for the CR in voice 1 with earliest tick and highest staff position
            // but stop we find the last selected CR
            ElementType et = e->type();
            if (et == ElementType::NOTE || et == ElementType::REST) {
                if (e->voice()) {
                    continue;
                }
                ChordRest* cr;
                if (et == ElementType::NOTE) {
                    cr = static_cast<ChordRest*>(e->parent());
                    if (!cr) {
                        continue;
                    }
                } else {
                    cr = static_cast<ChordRest*>(e);
                }
                if (cr == lastSelected) {
                    topLeft = cr;
                    break;
                }
                // compare ticks rather than x position
                // to make sure we favor earlier rather than later systems
                // even though later system might have note farther to left
                Fraction crTick = Fraction(0,1);
                if (cr->segment()) {
                    crTick = cr->segment()->tick();
                } else {
                    continue;
                }
                // compare staff Y position rather than note Y position
                // to be sure we do not reject earliest note
                // just because it is lower in pitch than subsequent notes
                qreal crY = 0.0;
                if (cr->measure() && cr->measure()->system()) {
                    crY = cr->measure()->system()->staffYpage(cr->staffIdx());
                } else {
                    continue;
                }
                if (topLeft) {
                    if (crTick <= tlTick && crY <= tlY) {
                        topLeft = cr;
                        tlTick = crTick;
                        tlY = crY;
                    }
                } else {
                    topLeft = cr;
                    tlTick = crTick;
                    tlY = crY;
                }
            }
        }

        if (topLeft) {
            select(topLeft, SelectType::SINGLE);
        }
    }
}

void Notation::endNoteEntry()
{
    InputState& is = score()->inputState();
    is.setNoteEntryMode(false);
    if (is.slur()) {
        const std::vector<SpannerSegment*>& el = is.slur()->spannerSegments();
        if (!el.empty()) {
            el.front()->setSelected(false);
        }
        is.setSlur(0);
    }

    hideShadowNote();
    m_inputStateChanged.notify();
}

void Notation::padNote(const Pad& pad)
{
    Ms::EditData ed;
    ed.view = m_scoreCallbacks;
    score()->startCmd();
    score()->padToggle(pad, ed);
    score()->endCmd();
    m_inputStateChanged.notify();
}

void Notation::putNote(const QPointF& pos, bool replace, bool insert)
{
    score()->startCmd();
    score()->putNote(pos, replace, insert);
    score()->endCmd();
    notifyAboutNotationChanged();
}

void Notation::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

mu::async::Notification Notation::notationChanged() const
{
    return m_notationChanged;
}

mu::async::Notification Notation::inputStateChanged() const
{
    return m_inputStateChanged;
}

mu::async::Notification Notation::selectionChanged() const
{
    return m_selectionChanged;
}

Ms::Score* Notation::score() const
{
    return m_score;
}

void Notation::showShadowNote(const QPointF& p)
{
    //! NOTE This method coped from ScoreView::setShadowNote

    const InputState& is = score()->inputState();
    Position pos;
    if (!score()->getPosition(&pos, p, is.voice())) {
        m_shadowNote->setVisible(false);
        return;
    }

    // in any empty measure, pos will be right next to barline
    // so pad this by barNoteDistance
    qreal mag = score()->staff(pos.staffIdx)->staffMag(Fraction(0,1));
    qreal relX = pos.pos.x() - pos.segment->measure()->canvasPos().x();
    pos.pos.rx() -= qMin(relX - score()->styleP(Sid::barNoteDistance) * mag, 0.0);

    m_shadowNote->setVisible(true);

    Staff* staff = score()->staff(pos.staffIdx);
    m_shadowNote->setMag(staff->staffMag(Fraction(0,1)));
    const Instrument* instr = staff->part()->instrument();
    NoteHead::Group noteheadGroup = NoteHead::Group::HEAD_NORMAL;
    int line = pos.line;
    NoteHead::Type noteHead = is.duration().headType();

    if (instr->useDrumset()) {
        const Drumset* ds  = instr->drumset();
        int pitch    = is.drumNote();
        if (pitch >= 0 && ds->isValid(pitch)) {
            line     = ds->line(pitch);
            noteheadGroup = ds->noteHead(pitch);
        }
    }

    m_shadowNote->setLine(line);

    int voice;
    if (is.drumNote() != -1 && is.drumset() && is.drumset()->isValid(is.drumNote())) {
        voice = is.drumset()->voice(is.drumNote());
    } else {
        voice = is.voice();
    }

    SymId symNotehead;
    TDuration d(is.duration());

    if (is.rest()) {
        int yo;
        Rest rest(gscore, d.type());
        rest.setTicks(d.fraction());
        symNotehead = rest.getSymbol(is.duration().type(), 0, staff->lines(pos.segment->tick()), &yo);
        m_shadowNote->setState(symNotehead, voice, d, true);
    } else {
        if (NoteHead::Group::HEAD_CUSTOM == noteheadGroup) {
            symNotehead = instr->drumset()->noteHeads(is.drumNote(), noteHead);
        } else {
            symNotehead = Note::noteHead(0, noteheadGroup, noteHead);
        }

        m_shadowNote->setState(symNotehead, voice, d);
    }

    m_shadowNote->layout();
    m_shadowNote->setPos(pos.pos);
}

void Notation::hideShadowNote()
{
    m_shadowNote->setVisible(false);
}

void Notation::paintShadowNote(QPainter* p)
{
    m_shadowNote->draw(p);
}

INotationSelection* Notation::selection() const
{
    return m_selection;
}

void Notation::select(Element* e, SelectType type, int staffIdx)
{
    score()->select(e, type, staffIdx);
    m_selectionChanged.notify();
}

INotationInputController* Notation::inputController() const
{
    return m_inputController;
}
