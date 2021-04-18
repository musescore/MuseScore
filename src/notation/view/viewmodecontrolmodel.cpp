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

#include "viewmodecontrolmodel.h"

#include "translation.h"

using namespace mu::notation;
using namespace mu::ui;

static QMap<ViewMode, mu::actions::ActionCode> viewModes = {
    { ViewMode::PAGE, "view-mode-page" },
    { ViewMode::LINE, "view-mode-continuous" },
    { ViewMode::SYSTEM, "view-mode-single" }
};

ViewModeControlModel::ViewModeControlModel(QObject* parent)
    : QObject(parent)
{
}

QVariant ViewModeControlModel::currentViewMode()
{
    auto notation = context()->currentNotation();
    if (!notation) {
        return QVariant();
    }

    auto correctedTitle = [](ViewMode viewMode, const QString& title) {
        switch (viewMode) {
        case ViewMode::PAGE:
            return title;
        case ViewMode::LINE:
        case ViewMode::SYSTEM:
            return qtrc("notation", "Continuous View");
        default:
            return title;
        }
    };

    ViewMode viewMode = notation->viewMode();
    MenuItem viewModeItem = findItem(viewModeActionCode(viewMode));
    QVariantMap viewModeObj;
    viewModeObj["title"] = correctedTitle(viewMode, viewModeItem.title);
    viewModeObj["icon"] = static_cast<int>(viewModeItem.iconCode);

    return viewModeObj;
}

void ViewModeControlModel::load()
{
    appendItem(makeAction("view-mode-page"));
    appendItem(makeAction("view-mode-continuous"));
    appendItem(makeAction("view-mode-single"));

    context()->currentNotationChanged().onNotify(this, [this]() {
        auto notation = context()->currentNotation();
        if (!notation) {
            return;
        }

        notation->notationChanged().onNotify(this, [this]() {
            updateState();
        });

        updateState();
    });

    updateState();
}

void ViewModeControlModel::selectViewMode(const QString& actionCode)
{
    dispatcher()->dispatch(actions::codeFromQString(actionCode));
}

mu::actions::ActionCode ViewModeControlModel::viewModeActionCode(ViewMode viewMode) const
{
    return viewModes[viewMode];
}

void ViewModeControlModel::updateState()
{
    auto notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    actions::ActionCode currentViewModeActionCode = viewModeActionCode(notation->viewMode());
    for (const actions::ActionCode& actionCode: viewModes.values()) {
        MenuItem& viewModeItem = findItem(actionCode);
        viewModeItem.selectable = true;
        viewModeItem.selected = viewModeItem.code == currentViewModeActionCode;
    }

    emit itemsChanged();
    emit currentViewModeChanged();
}
