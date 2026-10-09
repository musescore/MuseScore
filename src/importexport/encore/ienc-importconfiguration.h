/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Interface for Encore import options: each user-facing toggle exposes a getter, setter and change channel.

#pragma once

#include "modularity/imoduleinterface.h"
#include "async/channel.h"

#include "internal/importer/import-options.h"

namespace mu::iex::enc {
class IEncImportConfiguration : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IEncImportConfiguration)

public:
    virtual ~IEncImportConfiguration() = default;

    virtual bool importPageLayout() const = 0;
    virtual void setImportPageLayout(bool value) = 0;
    virtual muse::async::Channel<bool> importPageLayoutChanged() const = 0;

    virtual bool importPageBreaks() const = 0;
    virtual void setImportPageBreaks(bool value) = 0;
    virtual muse::async::Channel<bool> importPageBreaksChanged() const = 0;

    virtual bool importSystemLocks() const = 0;
    virtual void setImportSystemLocks(bool value) = 0;
    virtual muse::async::Channel<bool> importSystemLocksChanged() const = 0;

    virtual bool importStaffSize() const = 0;
    virtual void setImportStaffSize(bool value) = 0;
    virtual muse::async::Channel<bool> importStaffSizeChanged() const = 0;

    virtual bool importTempoTextSemantic() const = 0;
    virtual void setImportTempoTextSemantic(bool value) = 0;
    virtual muse::async::Channel<bool> importTempoTextSemanticChanged() const = 0;

    virtual bool importUnsupportedArticulationsAsText() const = 0;
    virtual void setImportUnsupportedArticulationsAsText(bool value) = 0;
    virtual muse::async::Channel<bool> importUnsupportedArticulationsAsTextChanged() const = 0;

    virtual InstrumentSearchMode instrumentSearchMode() const = 0;
    virtual void setInstrumentSearchMode(InstrumentSearchMode value) = 0;
    virtual muse::async::Channel<InstrumentSearchMode> instrumentSearchModeChanged() const = 0;

    virtual UnderfillStrategy underfillMeasureStrategy() const = 0;
    virtual void setUnderfillMeasureStrategy(UnderfillStrategy value) = 0;
    virtual muse::async::Channel<UnderfillStrategy> underfillMeasureStrategyChanged() const = 0;

    virtual OverfillStrategy overfillMeasureStrategy() const = 0;
    virtual void setOverfillMeasureStrategy(OverfillStrategy value) = 0;
    virtual muse::async::Channel<OverfillStrategy> overfillMeasureStrategyChanged() const = 0;

    virtual bool firstMeasureIsPickup() const = 0;
    virtual void setFirstMeasureIsPickup(bool value) = 0;
    virtual muse::async::Channel<bool> firstMeasureIsPickupChanged() const = 0;

    virtual bool mergeVoices() const = 0;
    virtual void setMergeVoices(bool value) = 0;
    virtual muse::async::Channel<bool> mergeVoicesChanged() const = 0;

    virtual TablatureImportMode tablatureImportMode() const = 0;
    virtual void setTablatureImportMode(TablatureImportMode value) = 0;
    virtual muse::async::Channel<TablatureImportMode> tablatureImportModeChanged() const = 0;
};
} // namespace mu::iex::enc
