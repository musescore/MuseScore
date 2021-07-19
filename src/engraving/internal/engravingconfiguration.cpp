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
#include "engravingconfiguration.h"

#include "global/settings.h"

#include "libmscore/preferences.h"

using namespace mu::engraving;
using namespace mu::framework;

static const Settings::Key DEFAULT_STYLE_FILE_PATH("engraving", "engraving/style/defaultStyleFile");
static const Settings::Key PART_STYLE_FILE_PATH("engraving", "engraving/style/partStyleFile");

QString EngravingConfiguration::defaultStyleFilePath() const
{
    return settings()->value(DEFAULT_STYLE_FILE_PATH).toQString();
}

void EngravingConfiguration::setDefaultStyleFilePath(const QString& path)
{
    preferences().setDefaultStyleFilePath(path);
    settings()->setSharedValue(DEFAULT_STYLE_FILE_PATH, Val(path.toStdString()));
}

QString EngravingConfiguration::partStyleFilePath() const
{
    return settings()->value(PART_STYLE_FILE_PATH).toQString();
}

void EngravingConfiguration::setPartStyleFilePath(const QString& path)
{
    settings()->setSharedValue(PART_STYLE_FILE_PATH, Val(path.toStdString()));
}

QString EngravingConfiguration::keysigColor() const
{
    return "#808080";
}

QString EngravingConfiguration::defaultColor() const
{
    return "#000000";
}

QString EngravingConfiguration::whiteColor() const
{
    return "#FFFFFF";
}

QString EngravingConfiguration::blackColor() const
{
    return "#000000";
}

QString EngravingConfiguration::redColor() const
{
    return "#FF0000";
}

QString EngravingConfiguration::invisibleColor() const
{
    return "#808080";
}

QString EngravingConfiguration::lassoColor() const
{
    return "#00003232";
}

QString EngravingConfiguration::figuredBassColor() const
{
    return "#D3D3D3";
}

QString EngravingConfiguration::selectionColor() const
{
    return "#4682B4";
}

QString EngravingConfiguration::warningColor() const
{
    return "#808000";
}

QString EngravingConfiguration::warningSelectedColor() const
{
    return "#565600";
}

QString EngravingConfiguration::criticalColor() const
{
    return redColor();
}

QString EngravingConfiguration::criticalSelectedColor() const
{
    return "#8B0000";
}

QString EngravingConfiguration::editColor() const
{
    return "#C0C0C0";
}

QString EngravingConfiguration::harmonyColor() const
{
    return redColor();
}

QString EngravingConfiguration::textBaseFrameColor() const
{
    return blackColor();
}

QString EngravingConfiguration::textBaseBgColor() const
{
    return "#FFFFFF00";
}

QString EngravingConfiguration::shadowNoteColor() const
{
    // value of QColor::lighter(SHADOW_NOTE_LIGHT) for MScore::selectColor[voice()] when SHADOW_NOTE_LIGHT = 135
    return "#03FF88";
}
