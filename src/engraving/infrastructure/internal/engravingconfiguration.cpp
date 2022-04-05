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
#include "draw/color.h"
#include "libmscore/mscore.h"

#include "log.h"

using namespace mu; //needed so async::Notification is detected as a valid type
using namespace mu::engraving;
using namespace mu::framework;
using namespace mu::draw;

static const Settings::Key DEFAULT_STYLE_FILE_PATH("engraving", "engraving/style/defaultStyleFile");
static const Settings::Key PART_STYLE_FILE_PATH("engraving", "engraving/style/partStyleFile");

static const Settings::Key INVERT_SCORE_COLOR("engraving", "engraving/scoreColorInversion");

struct VoiceColorKey {
    Settings::Key key;
    Color color;
};

static VoiceColorKey voiceColorKeys[Ms::VOICES];

void EngravingConfiguration::init()
{
    Color defaultVoiceColors[Ms::VOICES] {
        "#0065BF",
        "#007F00",
        "#C53F00",
        "#C31989"
    };

    settings()->setDefaultValue(INVERT_SCORE_COLOR, Val(false));
    settings()->valueChanged(INVERT_SCORE_COLOR).onReceive(nullptr, [this](const Val&) {
        m_scoreInversionChanged.notify();
    });

    for (int voice = 0; voice < Ms::VOICES; ++voice) {
        Settings::Key key("engraving", "engraving/colors/voice" + std::to_string(voice + 1));

        settings()->setDefaultValue(key, Val(defaultVoiceColors[voice].toQColor()));
        settings()->setCanBeMannualyEdited(key, true);
        settings()->valueChanged(key).onReceive(this, [this, voice](const Val& val) {
            Color color = val.toQColor();
            voiceColorKeys[voice].color = color;
            m_voiceColorChanged.send(voice, color);
        });

        Color currentColor = settings()->value(key).toQColor();
        voiceColorKeys[voice] = VoiceColorKey { std::move(key), currentColor };
    }
}

QString EngravingConfiguration::defaultStyleFilePath() const
{
    return settings()->value(DEFAULT_STYLE_FILE_PATH).toQString();
}

void EngravingConfiguration::setDefaultStyleFilePath(const QString& path)
{
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

std::string EngravingConfiguration::iconsFontFamily() const
{
    return uiConfiguration()->iconsFontFamily();
}

Color EngravingConfiguration::defaultColor() const
{
    return Color::black;
}

Color EngravingConfiguration::scoreInversionColor() const
{
    // slightly dulled white for less strain on the eyes
    return Color(220, 220, 220);
}

Color EngravingConfiguration::invisibleColor() const
{
    return "#808080";
}

Color EngravingConfiguration::lassoColor() const
{
    return "#00323200";
}

Color EngravingConfiguration::warningColor() const
{
    return "#808000";
}

Color EngravingConfiguration::warningSelectedColor() const
{
    return "#565600";
}

Color EngravingConfiguration::criticalColor() const
{
    return Color::redColor;
}

Color EngravingConfiguration::criticalSelectedColor() const
{
    return "#8B0000";
}

Color EngravingConfiguration::formattingMarksColor() const
{
    return "#A0A0A4";
}

double EngravingConfiguration::guiScaling() const
{
    return uiConfiguration()->guiScaling();
}

Color EngravingConfiguration::selectionColor(int voice) const
{
    return voiceColorKeys[voice].color;
}

void EngravingConfiguration::setSelectionColor(int voiceIndex, Color color)
{
    settings()->setSharedValue(voiceColorKeys[voiceIndex].key, Val(color.toQColor()));
}

mu::async::Channel<int, Color> EngravingConfiguration::selectionColorChanged() const
{
    return m_voiceColorChanged;
}

Color EngravingConfiguration::highlightSelectionColor(int voice) const
{
    return Color::fromQColor(selectionColor(voice).toQColor().lighter(135));
}

bool EngravingConfiguration::scoreInversionEnabled() const
{
    return settings()->value(INVERT_SCORE_COLOR).toBool();
}

void EngravingConfiguration::setScoreInversionEnabled(bool value)
{
    settings()->setSharedValue(INVERT_SCORE_COLOR, Val(value));
}

async::Notification EngravingConfiguration::scoreInversionChanged() const
{
    return m_scoreInversionChanged;
}
