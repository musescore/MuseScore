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
#include "ui/iuiconfiguration.h"
#include "accessibility/iaccessibilityconfiguration.h"

#include "../iengravingconfiguration.h"

namespace mu::engraving {
class EngravingConfiguration : public IEngravingConfiguration, public async::Asyncable
{
    INJECT(engraving, mu::ui::IUiConfiguration, uiConfiguration)
    INJECT(engraving, mu::accessibility::IAccessibilityConfiguration, accessibilityConfiguration)

public:
    EngravingConfiguration() = default;

    void init();

    io::path_t defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const io::path_t& path) override;

    io::path_t partStyleFilePath() const override;
    void setPartStyleFilePath(const io::path_t& path) override;

    String iconsFontFamily() const override;

    draw::Color defaultColor() const override;
    draw::Color scoreInversionColor() const override;
    draw::Color invisibleColor() const override;
    draw::Color lassoColor() const override;
    draw::Color warningColor() const override;
    draw::Color warningSelectedColor() const override;
    draw::Color criticalColor() const override;
    draw::Color criticalSelectedColor() const override;
    draw::Color formattingMarksColor() const override;

    double guiScaling() const override;

    draw::Color selectionColor(engraving::voice_idx_t voiceIndex = 0) const override;
    void setSelectionColor(engraving::voice_idx_t voiceIndex, draw::Color color) override;
    async::Channel<engraving::voice_idx_t, draw::Color> selectionColorChanged() const override;

    draw::Color highlightSelectionColor(engraving::voice_idx_t voice = 0) const override;

    bool scoreInversionEnabled() const override;
    void setScoreInversionEnabled(bool value) override;

    async::Notification scoreInversionChanged() const override;

    const DebuggingOptions& debuggingOptions() const override;
    void setDebuggingOptions(const DebuggingOptions& options) override;
    async::Notification debuggingOptionsChanged() const override;

    bool isAccessibleEnabled() const override;

private:
    async::Channel<engraving::voice_idx_t, draw::Color> m_voiceColorChanged;
    async::Notification m_scoreInversionChanged;

    ValNt<DebuggingOptions> m_debuggingOptions;
};
}

#endif // MU_ENGRAVING_ENGRAVINGCONFIGURATION_H
