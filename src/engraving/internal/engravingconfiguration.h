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
class EngravingConfiguration : public IEngravingConfiguration, public async::Asyncable
{
    INJECT(mu::IGlobalConfiguration, globalConfiguration)
    INJECT(muse::ui::IUiConfiguration, uiConfiguration)
    INJECT(muse::accessibility::IAccessibilityConfiguration, accessibilityConfiguration)
    INJECT(iex::guitarpro::IGuitarProConfiguration, guitarProConfiguration);

public:
    EngravingConfiguration() = default;

    void init();

    io::path_t appDataPath() const override;

    io::path_t defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const io::path_t& path) override;

    io::path_t partStyleFilePath() const override;
    void setPartStyleFilePath(const io::path_t& path) override;

    SizeF defaultPageSize() const override;

    String iconsFontFamily() const override;

    muse::draw::Color defaultColor() const override;
    muse::draw::Color scoreInversionColor() const override;
    muse::draw::Color invisibleColor() const override;
    muse::draw::Color lassoColor() const override;
    muse::draw::Color warningColor() const override;
    muse::draw::Color warningSelectedColor() const override;
    muse::draw::Color criticalColor() const override;
    muse::draw::Color criticalSelectedColor() const override;
    muse::draw::Color formattingMarksColor() const override;
    muse::draw::Color thumbnailBackgroundColor() const override;
    muse::draw::Color noteBackgroundColor() const override;
    muse::draw::Color fontPrimaryColor() const override;

    muse::draw::Color timeTickAnchorColorLighter() const override;
    muse::draw::Color timeTickAnchorColorDarker() const override;

    double guiScaling() const override;

    muse::draw::Color selectionColor(voice_idx_t voiceIndex = 0, bool itemVisible = true,
                                     bool itemIsUnlinkedFromScore = false) const override;
    void setSelectionColor(voice_idx_t voiceIndex, muse::draw::Color color) override;
    async::Channel<voice_idx_t, muse::draw::Color> selectionColorChanged() const override;

    muse::draw::Color highlightSelectionColor(voice_idx_t voice = 0) const override;

    bool scoreInversionEnabled() const override;
    void setScoreInversionEnabled(bool value) override;

    async::Notification scoreInversionChanged() const override;

    const DebuggingOptions& debuggingOptions() const override;
    void setDebuggingOptions(const DebuggingOptions& options) override;
    async::Notification debuggingOptionsChanged() const override;

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
    async::Channel<voice_idx_t, muse::draw::Color> m_voiceColorChanged;
    async::Notification m_scoreInversionChanged;

    ValNt<DebuggingOptions> m_debuggingOptions;

    bool m_multiVoice = false;
};
}

#endif // MU_ENGRAVING_ENGRAVINGCONFIGURATION_H
