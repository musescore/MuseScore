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
    m_scoreGlobal = new MScore();
    m_score = new MasterScore(m_scoreGlobal->baseStyle());
    m_scoreCallbacks = new ScoreCallbacks();
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

void Notation::startNoteEntry()
{
    Ms::InputState& is = score()->inputState();
    is.setSegment(0);
    is.setAccidentalType(Ms::AccidentalType::NONE);
    is.setRest(false);
    is.setNoteEntryMode(true);
}

void Notation::action(const actions::ActionName& name)
{
    Ms::EditData ed;
    ed.view = m_scoreCallbacks;
    score()->cmd(name, ed);
}

void Notation::putNote(const QPointF& pos, bool replace, bool insert)
{
    score()->putNote(pos, replace, insert);
}

Ms::MasterScore* Notation::score() const
{
    return m_score;
}
