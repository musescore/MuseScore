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
#include "mu4paletteadapter.h"

#include <QAction>

#include "log.h"

#include "palette/paletteworkspace.h"
#include "palette/palettetree.h"
#include "palette/palettemodel.h"
#include "palette/palettecreator.h"

using namespace mu;
using namespace mu::palette;

MU4PaletteAdapter::MU4PaletteAdapter()
{
    m_paletteEnabled.val = true;
}

const ui::UiAction& MU4PaletteAdapter::getAction(const actions::ActionCode& code) const
{
    return actionsRegister()->action(code);
}

void MU4PaletteAdapter::showMasterPalette(const QString& selectedPaletteName)
{
    QStringList params {
        "sync=false",
        QString("selectedPaletteName=%1").arg(selectedPaletteName)
    };

    QString uri = QString("musescore://palette/masterpalette?%1").arg(params.join('&'));
    interactive()->open(uri.toStdString());
}

bool MU4PaletteAdapter::isSelected() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return false;
    }

    return !notation->interaction()->selection()->isNone();
}

bool MU4PaletteAdapter::applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers)
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return false;
    }

    return notation->interaction()->applyPaletteElement(element, modifiers);
}

Ms::PaletteWorkspace* MU4PaletteAdapter::paletteWorkspace() const
{
    if (!m_paletteWorkspace) {
        Ms::PaletteTreeModel* emptyModel = new Ms::PaletteTreeModel(std::make_shared<Ms::PaletteTree>());
        Ms::PaletteTreeModel* masterPaletteModel = new Ms::PaletteTreeModel(Ms::PaletteCreator::newMasterPaletteTree());

        m_paletteWorkspace = new Ms::PaletteWorkspace(emptyModel, masterPaletteModel, /* parent */ nullptr);
        emptyModel->setParent(m_paletteWorkspace);
        masterPaletteModel->setParent(m_paletteWorkspace);
    }

    return m_paletteWorkspace;
}

ValCh<bool> MU4PaletteAdapter::paletteEnabled() const
{
    return m_paletteEnabled;
}

void MU4PaletteAdapter::setPaletteEnabled(bool arg)
{
    m_paletteEnabled.set(arg);
}

void MU4PaletteAdapter::requestPaletteSearch()
{
    m_paletteSearchRequested.notify();
}

mu::async::Notification MU4PaletteAdapter::paletteSearchRequested() const
{
    return m_paletteSearchRequested;
}

void MU4PaletteAdapter::notifyElementDraggedToScoreView()
{
    m_elementDraggedToScoreView.notify();
}

mu::async::Notification MU4PaletteAdapter::elementDraggedToScoreView() const
{
    return m_elementDraggedToScoreView;
}
