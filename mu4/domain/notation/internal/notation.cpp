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

    m_inputState = new NotationInputState(m_score);
}

Notation::~Notation()
{
    delete m_shadowNote;
    delete m_scoreCallbacks;
    delete m_score;
}

void Notation::init()
{
    MScore::init();         // initialize libmscore
}

bool Notation::load(const std::string& path, const Params& params)
{
    Score::FileError rv = m_score->loadMsc(QString::fromStdString(path), true);
    if (rv != Score::FileError::FILE_NO_ERROR) {
        return false;
    }

    m_score->setUpdateAll();
    m_score->doLayout();

    return true;
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
    Ms::InputState& is = score()->inputState();
    is.setSegment(0);
    is.setAccidentalType(Ms::AccidentalType::NONE);
    is.setRest(false);
    is.setNoteEntryMode(true);

    m_inputState->notifyAboutISChanged();
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
    m_inputState->notifyAboutISChanged();
}

void Notation::padNote(const actions::ActionName& name)
{
    Ms::EditData ed;
    ed.view = m_scoreCallbacks;
    score()->cmd(name, ed);
    m_inputState->notifyAboutISChanged();
}

void Notation::putNote(const QPointF& pos, bool replace, bool insert)
{
    score()->startCmd();
    score()->putNote(pos, replace, insert);
    score()->endCmd();
}

Ms::MasterScore* Notation::score() const
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
