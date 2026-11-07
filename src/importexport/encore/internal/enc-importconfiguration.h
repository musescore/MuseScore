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

// Settings-backed implementation of the Encore import configuration.

#pragma once

#include "../ienc-importconfiguration.h"
#include "async/asyncable.h"

namespace mu::iex::enc {
class EncImportConfiguration : public IEncImportConfiguration, public muse::async::Asyncable
{
public:
    void init();

    bool importPageLayout() const override;
    void setImportPageLayout(bool value) override;
    muse::async::Channel<bool> importPageLayoutChanged() const override;

    bool importPageBreaks() const override;
    void setImportPageBreaks(bool value) override;
    muse::async::Channel<bool> importPageBreaksChanged() const override;

    bool importSystemLocks() const override;
    void setImportSystemLocks(bool value) override;
    muse::async::Channel<bool> importSystemLocksChanged() const override;

    bool importStaffSize() const override;
    void setImportStaffSize(bool value) override;
    muse::async::Channel<bool> importStaffSizeChanged() const override;

    bool importTempoTextSemantic() const override;
    void setImportTempoTextSemantic(bool value) override;
    muse::async::Channel<bool> importTempoTextSemanticChanged() const override;

    bool importUnsupportedArticulationsAsText() const override;
    void setImportUnsupportedArticulationsAsText(bool value) override;
    muse::async::Channel<bool> importUnsupportedArticulationsAsTextChanged() const override;

    InstrumentSearchMode instrumentSearchMode() const override;
    void setInstrumentSearchMode(InstrumentSearchMode value) override;
    muse::async::Channel<InstrumentSearchMode> instrumentSearchModeChanged() const override;

    UnderfillStrategy underfillMeasureStrategy() const override;
    void setUnderfillMeasureStrategy(UnderfillStrategy value) override;
    muse::async::Channel<UnderfillStrategy> underfillMeasureStrategyChanged() const override;

    OverfillStrategy overfillMeasureStrategy() const override;
    void setOverfillMeasureStrategy(OverfillStrategy value) override;
    muse::async::Channel<OverfillStrategy> overfillMeasureStrategyChanged() const override;

    bool firstMeasureIsPickup() const override;
    void setFirstMeasureIsPickup(bool value) override;
    muse::async::Channel<bool> firstMeasureIsPickupChanged() const override;

    bool mergeVoices() const override;
    void setMergeVoices(bool value) override;
    muse::async::Channel<bool> mergeVoicesChanged() const override;

    TablatureImportMode tablatureImportMode() const override;
    void setTablatureImportMode(TablatureImportMode value) override;
    muse::async::Channel<TablatureImportMode> tablatureImportModeChanged() const override;

private:
    muse::async::Channel<bool> m_importPageLayoutChanged;
    muse::async::Channel<bool> m_importPageBreaksChanged;
    muse::async::Channel<bool> m_importSystemLocksChanged;
    muse::async::Channel<bool> m_importStaffSizeChanged;
    muse::async::Channel<bool> m_importTempoTextSemanticChanged;
    muse::async::Channel<bool> m_importUnsupportedArticulationsAsTextChanged;
    muse::async::Channel<InstrumentSearchMode> m_instrumentSearchModeChanged;
    muse::async::Channel<UnderfillStrategy> m_underfillMeasureStrategyChanged;
    muse::async::Channel<OverfillStrategy> m_overfillMeasureStrategyChanged;
    muse::async::Channel<bool> m_firstMeasureIsPickupChanged;
    muse::async::Channel<bool> m_mergeVoicesChanged;
    muse::async::Channel<TablatureImportMode> m_tablatureImportModeChanged;
};
} // namespace mu::iex::enc
