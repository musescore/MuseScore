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

namespace mu::engraving {
class EngravingConfiguration : public IEngravingConfiguration
{
public:
    EngravingConfiguration() = default;

    QString defaultStyleFilePath() const override;
    void setDefaultStyleFilePath(const QString& path) override;

    QString partStyleFilePath() const override;
    void setPartStyleFilePath(const QString& path) override;

    mu::draw::Color keysigColor() const override;
    mu::draw::Color defaultColor() const override;
    mu::draw::Color whiteColor() const override;
    mu::draw::Color blackColor() const override;
    mu::draw::Color redColor() const override;
    mu::draw::Color invisibleColor() const override;
    mu::draw::Color lassoColor() const override;
    mu::draw::Color figuredBassColor() const override;
    mu::draw::Color selectionColor() const override;
    mu::draw::Color warningColor() const override;
    mu::draw::Color warningSelectedColor() const override;
    mu::draw::Color criticalColor() const override;
    mu::draw::Color criticalSelectedColor() const override;
    mu::draw::Color editColor() const override;
    mu::draw::Color harmonyColor() const override;
    mu::draw::Color textBaseFrameColor() const override;
    mu::draw::Color textBaseBgColor() const override;
    mu::draw::Color shadowNoteColor() const override;
};
}

#endif // MU_ENGRAVING_ENGRAVINGCONFIGURATION_H
