/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "percussionpanelmodel.h"
#include "types/translatablestring.h"
#include "ui/view/iconcodes.h"

static const QString INSTRUMENT_NAMES_CODE("percussion-instrument-names");
static const QString NOTATION_PREVIEW_CODE("percussion-notation-preview");
static const QString EDIT_LAYOUT_CODE("percussion-edit-layout");
static const QString RESET_LAYOUT_CODE("percussion-reset-layout");

using namespace muse;
using namespace ui;

PercussionPanelModel::PercussionPanelModel(QObject* parent)
    : QObject(parent)
{
    m_padListModel = new PercussionPanelPadListModel(this);
}

PanelMode::Mode PercussionPanelModel::currentPanelMode() const
{
    return m_currentPanelMode;
}

void PercussionPanelModel::setCurrentPanelMode(const PanelMode::Mode& panelMode)
{
    if (m_currentPanelMode == panelMode) {
        return;
    }

    // After editing, return to the last non-edit mode...
    if (panelMode != PanelMode::Mode::EDIT_LAYOUT && m_panelModeToRestore != panelMode) {
        m_panelModeToRestore = panelMode;
    }

    m_currentPanelMode = panelMode;
    emit currentPanelModeChanged(m_currentPanelMode);
}

bool PercussionPanelModel::useNotationPreview() const
{
    return m_useNotationPreview;
}

void PercussionPanelModel::setUseNotationPreview(bool useNotationPreview)
{
    if (m_useNotationPreview == useNotationPreview) {
        return;
    }

    m_useNotationPreview = useNotationPreview;
    emit useNotationPreviewChanged(m_useNotationPreview);
}

PercussionPanelPadListModel* PercussionPanelModel::padListModel() const
{
    return m_padListModel;
}

QList<QVariantMap> PercussionPanelModel::layoutMenuItems() const
{
    const TranslatableString instrumentNamesTitle("notation", "Instrument names");
    // Using IconCode for this instead of "checked" because we want the tick to display on the left
    const int instrumentNamesIcon = static_cast<int>(m_useNotationPreview ? IconCode::Code::NONE : IconCode::Code::TICK_RIGHT_ANGLE);

    const TranslatableString notationPreviewTitle("notation", "Notation preview");
    // Using IconCode for this instead of "checked" because we want the tick to display on the left
    const int notationPreviewIcon = static_cast<int>(m_useNotationPreview ? IconCode::Code::TICK_RIGHT_ANGLE : IconCode::Code::NONE);

    const TranslatableString editLayoutTitle("notation", "Edit layout");
    const int editLayoutIcon = static_cast<int>(IconCode::Code::CONFIGURE);

    const TranslatableString resetLayoutTitle("notation", "Reset layout");
    const int resetLayoutIcon = static_cast<int>(IconCode::Code::UNDO);

    QList<QVariantMap> menuItems = {
        { { "id", INSTRUMENT_NAMES_CODE },
            { "title", instrumentNamesTitle.qTranslated() }, { "icon", instrumentNamesIcon }, { "enabled", true } },

        { { "id", NOTATION_PREVIEW_CODE },
            { "title", notationPreviewTitle.qTranslated() }, { "icon", notationPreviewIcon }, { "enabled", true } },

        { }, // separator

        { { "id", EDIT_LAYOUT_CODE },
            { "title", editLayoutTitle.qTranslated() }, { "icon", editLayoutIcon }, { "enabled", true } },

        { { "id", RESET_LAYOUT_CODE },
            { "title", resetLayoutTitle.qTranslated() }, { "icon", resetLayoutIcon }, { "enabled", true } },
    };

    return menuItems;
}

void PercussionPanelModel::handleMenuItem(const QString& itemId)
{
    if (itemId == INSTRUMENT_NAMES_CODE) {
        setUseNotationPreview(false);
    } else if (itemId == NOTATION_PREVIEW_CODE) {
        setUseNotationPreview(true);
    } else if (itemId == EDIT_LAYOUT_CODE) {
        setCurrentPanelMode(PanelMode::Mode::EDIT_LAYOUT);
    } else if (itemId == RESET_LAYOUT_CODE) {
        m_padListModel->resetLayout();
    }
}

void PercussionPanelModel::finishEditing()
{
    setCurrentPanelMode(m_panelModeToRestore);
}
