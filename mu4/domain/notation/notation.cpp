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

//    Ms::MStyle& styleRef = m_score->style();
//    styleRef.set(Ms::Sid::pageWidth, params.pageSize.width / Ms::DPI);
// styleRef.set(Ms::Sid::pageHeight, params.pageSize.height / Ms::DPI);

//    styleRef.set(Ms::Sid::pagePrintableWidth, (pageSize.pageWidth - pageSize.margingLeft
//                                               - pageSize.margingRight) / Ms::DPI);
//    styleRef.set(Ms::Sid::pageEvenLeftMargin, pageSize.margingLeft / Ms::DPI);
//    styleRef.set(Ms::Sid::pageOddLeftMargin, pageSize.margingLeft / Ms::DPI);

//    styleRef.set(Ms::Sid::pageEvenTopMargin, pageSize.margingTop / Ms::DPI);
//    styleRef.set(Ms::Sid::pageOddTopMargin, pageSize.margingTop / Ms::DPI);
//    styleRef.set(Ms::Sid::pageEvenBottomMargin, pageSize.margingBottom / Ms::DPI);
//    styleRef.set(Ms::Sid::pageOddBottomMargin, pageSize.margingBottom / Ms::DPI);

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
