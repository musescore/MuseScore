/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "measuressettingsmodel.h"

#include "translation.h"

#include "notation/inotationinteraction.h"
#include "notation/inotationselection.h"
#include "notationscene/notationcommands.h"

using namespace muse::rcommand;
using namespace mu::propertiespanel;
using namespace mu::notation;
using namespace mu::engraving;

MeasuresSettingsModel::MeasuresSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                             IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setSectionType(PropertiesPanelSectionType::SECTION_MEASURES);
    setTitle(muse::qtrc("propertiespanel", "Measures"));
}

void MeasuresSettingsModel::loadProperties()
{
}

bool MeasuresSettingsModel::shouldUpdateOnEmptyPropertyAndStyleIdSets() const
{
    return true;
}

void MeasuresSettingsModel::onNotationChanged(const engraving::PropertyIdSet&, const engraving::StyleIdSet&)
{
}

bool MeasuresSettingsModel::isEmpty() const
{
    INotationSelectionPtr selection = this->selection();
    return !selection || !selection->isRange();
}

void MeasuresSettingsModel::insertMeasures(int numberOfMeasures, InsertMeasuresTarget target)
{
    const Params params = { { "count", muse::Val(numberOfMeasures) } };

    switch (target) {
    case InsertMeasuresTarget::AfterSelection:
        commandDispatcher()->dispatch(INSERT_MEASURES_AFTER_SELECTION_COMMAND, params);
        break;
    case InsertMeasuresTarget::BeforeSelection:
        commandDispatcher()->dispatch(INSERT_MEASURES_COMMAND, params);
        break;
    case InsertMeasuresTarget::AtStartOfScore:
        commandDispatcher()->dispatch(INSERT_MEASURES_AT_START_OF_SCORE_COMMAND, params);
        break;
    case InsertMeasuresTarget::AtEndOfScore:
        commandDispatcher()->dispatch(APPEND_MEASURES_COMMAND, params);
        break;
    }
}

void MeasuresSettingsModel::deleteSelectedMeasures()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->removeSelectedMeasures();
}
