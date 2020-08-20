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
#include "notationtypes.h"

using namespace mu::notation;

ViewModeControlModel::ViewModeControlModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roleNames.insert(IdRole, "idRole");
    m_roleNames.insert(NameRole, "nameRole");
    m_viewModeOptions << ViewModeOption { "Page View", "view-mode-page", ViewMode::PAGE }
                      << ViewModeOption { "Continuous View", "view-mode-continuous", ViewMode::LINE }
                      << ViewModeOption { "Single Page", "view-mode-single", ViewMode::SYSTEM };
}

void ViewModeControlModel::load()
{
    onNotationChanged();
    m_notationChanged = globalContext()->currentNotationChanged();
    m_notationChanged.onNotify(this, [this]() {
        onNotationChanged();
    });
}

int ViewModeControlModel::viewModeToId(const ViewMode& viewMode)
{
    for (int i = 0; i < m_viewModeOptions.length(); ++i) {
        if (m_viewModeOptions.at(i).viewMode == viewMode) {
            return i;
        }
    }
    return -1;
}

int ViewModeControlModel::rowCount(const QModelIndex&) const
{
    return m_viewModeOptions.count();
}

QVariant ViewModeControlModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_viewModeOptions.isEmpty()) {
        return QVariant();
    }

    int viewModeId = index.row();

    switch (role) {
    case IdRole: return viewModeId;
    case NameRole: return m_viewModeOptions.at(viewModeId).displayString;
    default: return QVariant();
    }
}

QHash<int, QByteArray> ViewModeControlModel::roleNames() const
{
    return m_roleNames;
}

int ViewModeControlModel::currentViewModeId() const
{
    return m_currentViewModeId;
}

void ViewModeControlModel::setCurrentViewModeId(int newViewModeId)
{
    if (m_currentViewModeId != newViewModeId) {
        dispatcher()->dispatch(actions::namefromQString(m_viewModeOptions.at(newViewModeId).actionString));
        m_currentViewModeId = newViewModeId; // can remove once notify works
    }
}

void ViewModeControlModel::onNotationChanged()
{
    std::shared_ptr<INotation> notation = globalContext()->currentNotation();

    //! NOTE Unsubscribe from previous notation, if it was
    m_notationChanged.resetOnNotify(this);
    updateState();
}

void ViewModeControlModel::updateState()
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    int newViewModeId = viewModeToId(notation->viewMode());
    if (m_currentViewModeId != newViewModeId) {
        m_currentViewModeId = newViewModeId;
    }

    emit currentViewModeIdChanged(m_currentViewModeId);
}
