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
    auto boxTarget = [](InsertMeasuresTarget target) {
        switch (target) {
        case InsertMeasuresTarget::AfterSelection:
            return AddBoxesTarget::AfterSelection;
        case InsertMeasuresTarget::BeforeSelection:
            return AddBoxesTarget::BeforeSelection;
        case InsertMeasuresTarget::AtStartOfScore:
            return AddBoxesTarget::AtStartOfScore;
        case InsertMeasuresTarget::AtEndOfScore:
            return AddBoxesTarget::AtEndOfScore;
        }
        UNREACHABLE;
        return AddBoxesTarget::AtEndOfScore;
    };

    AddBoxesCommand cmd;
    cmd.boxType = BoxType::Measure;
    cmd.count = numberOfMeasures;
    cmd.target = boxTarget(target);

    commandDispatcher()->dispatch(cmd);
}

void MeasuresSettingsModel::deleteSelectedMeasures()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->removeSelectedMeasures();
}
