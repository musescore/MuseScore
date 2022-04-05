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

#include "../iengravingconfiguration.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "ui/iuiconfiguration.h"

namespace mu::engraving {
class EngravingConfiguration : public IEngravingConfiguration, public async::Asyncable
{
    INJECT(engraving, mu::ui::IUiConfiguration, uiConfiguration)

public:
    EngravingConfiguration() = default;

    void init() override;

    QString defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const QString& path) override;

    QString partStyleFilePath() const override;
    void setPartStyleFilePath(const QString& path) override;

    std::string iconsFontFamily() const override;

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

    draw::Color selectionColor(int voiceIndex = 0) const override;
    void setSelectionColor(int voiceIndex, draw::Color color) override;
    async::Channel<int, draw::Color> selectionColorChanged() const override;

    draw::Color highlightSelectionColor(int voice = 0) const override;

    bool scoreInversionEnabled() const override;
    void setScoreInversionEnabled(bool value) override;

    async::Notification scoreInversionChanged() const override;

private:
    async::Channel<int, draw::Color> m_voiceColorChanged;
    async::Notification m_scoreInversionChanged;
};
}

#endif // MU_ENGRAVING_ENGRAVINGCONFIGURATION_H
