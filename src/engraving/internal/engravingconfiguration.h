/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_ENGRAVINGCONFIGURATION_H
#define MU_ENGRAVING_ENGRAVINGCONFIGURATION_H

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "ui/iuiconfiguration.h"
#include "accessibility/iaccessibilityconfiguration.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"

#include "../iengravingconfiguration.h"

namespace mu::engraving {
class EngravingConfiguration : public IEngravingConfiguration, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<muse::IGlobalConfiguration> globalConfiguration = { this };
    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };
    muse::Inject<muse::accessibility::IAccessibilityConfiguration> accessibilityConfiguration = { this };
    muse::Inject<iex::guitarpro::IGuitarProConfiguration> guitarProConfiguration = { this };

public:
    EngravingConfiguration(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void init();

    muse::io::path_t appDataPath() const override;

    muse::io::path_t defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const muse::io::path_t& path) override;

    muse::io::path_t partStyleFilePath() const override;
    void setPartStyleFilePath(const muse::io::path_t& path) override;

    SizeF defaultPageSize() const override;

    String iconsFontFamily() const override;

    Color defaultColor() const override;
    Color scoreInversionColor() const override;
    Color invisibleColor() const override;
    Color lassoColor() const override;
    Color warningColor() const override;
    Color warningSelectedColor() const override;
    Color criticalColor() const override;
    Color criticalSelectedColor() const override;
    Color formattingMarksColor() const override;
    Color thumbnailBackgroundColor() const override;
    Color noteBackgroundColor() const override;
    Color fontPrimaryColor() const override;
    Color voiceColor(voice_idx_t voiceIdx) const override;

    double guiScaling() const override;

    Color selectionColor(voice_idx_t voiceIndex = 0, bool itemVisible = true, bool itemIsUnlinkedFromScore = false) const override;
    void setSelectionColor(voice_idx_t voiceIndex, Color color) override;
    muse::async::Channel<voice_idx_t, Color> selectionColorChanged() const override;

    Color highlightSelectionColor(voice_idx_t voice = 0) const override;

    bool scoreInversionEnabled() const override;
    void setScoreInversionEnabled(bool value) override;

    bool dynamicsApplyToAllVoices() const override;
    void setDynamicsApplyToAllVoices(bool v) override;

    muse::async::Notification scoreInversionChanged() const override;

    const DebuggingOptions& debuggingOptions() const override;
    void setDebuggingOptions(const DebuggingOptions& options) override;
    muse::async::Notification debuggingOptionsChanged() const override;

    bool isAccessibleEnabled() const override;

    bool guitarProImportExperimental() const override;
    bool negativeFretsAllowed() const override;
    bool crossNoteHeadAlwaysBlack() const override;
    bool enableExperimentalFretCircle() const override;
    void setGuitarProMultivoiceEnabled(bool multiVoice) override;
    bool guitarProMultivoiceEnabled() const override;
    bool minDistanceForPartialSkylineCalculated() const override;
    bool specificSlursLayoutWorkaround() const override;

private:
    muse::async::Channel<voice_idx_t, Color> m_voiceColorChanged;
    muse::async::Notification m_scoreInversionChanged;

    muse::ValNt<DebuggingOptions> m_debuggingOptions;

    bool m_multiVoice = false;
};
}

#endif // MU_ENGRAVING_ENGRAVINGCONFIGURATION_H
