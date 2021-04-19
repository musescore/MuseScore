/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_INSPECTOR_MU4INSPECTORADAPTER_H
#define MU_INSPECTOR_MU4INSPECTORADAPTER_H

#include "iinspectoradapter.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/inotation.h"
#include "iinteractive.h"

namespace mu::inspector {
class MU4InspectorAdapter : public IInspectorAdapter
{
    INJECT(inspector, mu::context::IGlobalContext, context)
    INJECT(inspector, mu::framework::IInteractive, interactive)

public:
    MU4InspectorAdapter() = default;

    bool isNotationExisting() const override;
    bool isTextEditingStarted() const override;
    async::Notification isTextEditingChanged() const override;

    // notation commands
    void beginCommand() override;
    void endCommand() override;

    // notation styling
    void updateStyleValue(const Ms::Sid& styleId, const QVariant& newValue) override;
    QVariant styleValue(const Ms::Sid& styleId) override;

    // dialogs
    void showSpecialCharactersDialog() override;
    void showStaffTextPropertiesDialog() override;
    void showPageSettingsDialog() override;
    void showStyleSettingsDialog() override;
    void showTimeSignaturePropertiesDialog() override;
    void showArticulationPropertiesDialog() override;
    void showGridConfigurationDialog() override;

    // actions
    void updatePageMarginsVisibility(const bool isVisible) override;
    void updateFramesVisibility(const bool isVisible) override;
    void updateHorizontalGridSnapping(const bool isSnapped) override;
    void updateVerticalGridSnapping(const bool isSnapped) override;
    void updateUnprintableElementsVisibility(const bool isVisible) override;
    void updateInvisibleElementsDisplaying(const bool isVisible) override;

    // notation layout
    void updateNotation() override;

private:
    mu::notation::INotationUndoStackPtr undoStack() const;
    mu::notation::INotationStylePtr style() const;
};
}

#endif // MU_INSPECTOR_MU4INSPECTORADAPTER_H
