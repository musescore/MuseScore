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
#include "mu3paletteadapter.h"

#include "shortcut.h"
#include "musescore.h"
#include "scoreview.h"

using namespace Ms;

MU3PaletteAdapter::MU3PaletteAdapter()
{
    m_paletteEnabled.val = true;
}

QAction* MU3PaletteAdapter::getAction(const char* id) const
{
    return Shortcut::getActionByName(id);
}

QString MU3PaletteAdapter::actionHelp(const char* id) const
{
    Shortcut* sc = Shortcut::getShortcut(id);
    return sc ? sc->help() : QString();
}

void MU3PaletteAdapter::showMasterPalette(const QString& arg)
{
    mscore->showMasterPalette(arg);
}

void MU3PaletteAdapter::selectInstrument(Ms::InstrumentChange* i)
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->selectInstrument(i);
    }
}

Ms::Score* MU3PaletteAdapter::currentScore() const
{
    return mscore->currentScore();
}

Ms::ScriptRecorder* MU3PaletteAdapter::getScriptRecorder() const
{
    return mscore->getScriptRecorder();
}

bool MU3PaletteAdapter::editMode() const
{
    auto view = mscore->currentScoreView();
    return view ? view->editMode() : false;
}

Ms::ScoreState MU3PaletteAdapter::mscoreState() const
{
    auto view = mscore->currentScoreView();
    return view ? view->mscoreState() : Ms::ScoreState::STATE_DISABLED;
}

void MU3PaletteAdapter::changeState(Ms::ViewState s)
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->changeState(s);
    }
}

void MU3PaletteAdapter::cmdAddSlur(const Ms::Slur* slurTemplate)
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->cmdAddSlur(slurTemplate);
    }
}

void MU3PaletteAdapter::applyDrop(Ms::Score* score, Ms::Element* target, Ms::Element* e,
                                  Qt::KeyboardModifiers modifiers,
                                  QPointF pt, bool pasteMode)
{
    ScoreView* viewer = mscore->currentScoreView();
    if (!viewer) {
        return;
    }

    EditData& dropData = viewer->getEditData();
    dropData.pos         = pt.isNull() ? target->pagePos() : pt;
    dropData.dragOffset  = QPointF();
    dropData.modifiers   = modifiers;
    dropData.dropElement = e;

    if (target->acceptDrop(dropData)) {
        // use same code path as drag&drop

        QByteArray a = e->mimeData(QPointF());
//printf("<<%s>>\n", a.data());

        XmlReader n(a);
        n.setPasteMode(pasteMode);
        Fraction duration;      // dummy
        QPointF dragOffset;
        ElementType type = Element::readType(n, &dragOffset, &duration);
        dropData.dropElement = Element::create(type, score);

        dropData.dropElement->read(n);
        dropData.dropElement->styleChanged();       // update to local style

        Element* el = target->drop(dropData);
        if (el && el->isInstrumentChange()) {
            selectInstrument(toInstrumentChange(el));
        }
        if (el && !viewer->noteEntryMode()) {
            score->select(el, SelectType::SINGLE, 0);
        }
        dropData.dropElement = 0;
    }
}

void MU3PaletteAdapter::moveCursor()
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->moveCursor();
    }
}

void MU3PaletteAdapter::setFocus()
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->setFocus();
    }
}

void MU3PaletteAdapter::setDropTarget(const Ms::Element* e)
{
    auto view = mscore->currentScoreView();
    if (view) {
        view->setDropTarget(e);
    }
}

Ms::PaletteWorkspace* MU3PaletteAdapter::paletteWorkspace() const
{
    return mscore->getPaletteWorkspace();
}

mu::ValCh<bool> MU3PaletteAdapter::paletteEnabled() const
{
    return m_paletteEnabled;
}

void MU3PaletteAdapter::setPaletteEnabled(bool arg)
{
    m_paletteEnabled.set(arg);
}

void MU3PaletteAdapter::requestPaletteSearch()
{
    m_paletteSearchRequested.notify();
}

mu::async::Notification MU3PaletteAdapter::paletteSearchRequested() const
{
    return m_paletteSearchRequested;
}
