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

#include <cstdlib>

#ifndef NO_QT_SUPPORT
#include <QLocale>
#include <QPageSize>
#endif

#include "global/settings.h"
#include "draw/types/color.h"
#include "libmscore/mscore.h"

#include "log.h"

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

static VoiceColorKey voiceColorKeys[VOICES];

void EngravingConfiguration::init()
{
    Color defaultVoiceColors[VOICES] {
        "#0065BF",
        "#007F00",
        "#C53F00",
        "#C31989"
    };

    settings()->setDefaultValue(INVERT_SCORE_COLOR, Val(false));
    settings()->valueChanged(INVERT_SCORE_COLOR).onReceive(nullptr, [this](const Val&) {
        m_scoreInversionChanged.notify();
    });

    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        Settings::Key key("engraving", "engraving/colors/voice" + std::to_string(voice + 1));

        settings()->setDefaultValue(key, Val(defaultVoiceColors[voice].toQColor()));
        settings()->setCanBeManuallyEdited(key, true);
        settings()->valueChanged(key).onReceive(this, [this, voice](const Val& val) {
            Color color = val.toQColor();
            voiceColorKeys[voice].color = color;
            m_voiceColorChanged.send(voice, color);
        });

        Color currentColor = settings()->value(key).toQColor();
        voiceColorKeys[voice] = VoiceColorKey { std::move(key), currentColor };
    }
}

mu::io::path_t EngravingConfiguration::appDataPath() const
{
    return globalConfiguration()->appDataPath();
}

mu::io::path_t EngravingConfiguration::defaultStyleFilePath() const
{
    return settings()->value(DEFAULT_STYLE_FILE_PATH).toPath();
}

void EngravingConfiguration::setDefaultStyleFilePath(const io::path_t& path)
{
    settings()->setSharedValue(DEFAULT_STYLE_FILE_PATH, Val(path.toStdString()));
}

mu::io::path_t EngravingConfiguration::partStyleFilePath() const
{
    return settings()->value(PART_STYLE_FILE_PATH).toPath();
}

void EngravingConfiguration::setPartStyleFilePath(const io::path_t& path)
{
    settings()->setSharedValue(PART_STYLE_FILE_PATH, Val(path.toStdString()));
}

static bool defaultPageSizeIsLetter()
{
    // try PAPERSIZE environment variable
    const char* papersize = getenv("PAPERSIZE");
    if (papersize) {
        return strcmp(papersize, "letter") == 0;
    }
#ifndef NO_QT_SUPPORT
    // try locale
    switch (QLocale::system().country()) {
    case QLocale::UnitedStates:
    case QLocale::Canada:
    case QLocale::Mexico:
    case QLocale::Chile:
    case QLocale::Colombia:
    case QLocale::CostaRica:
    case QLocale::Panama:
    case QLocale::Guatemala:
    case QLocale::DominicanRepublic:
    case QLocale::Philippines:
        return true;
    default:
        return false;
    }
#else
    return false;
#endif
}

SizeF EngravingConfiguration::defaultPageSize() const
{
    // Needs to be determined only once, therefore static
    static SizeF size = SizeF::fromQSizeF(
        QPageSize::size(defaultPageSizeIsLetter() ? QPageSize::Letter : QPageSize::A4, QPageSize::Inch));

    return size;
}

mu::String EngravingConfiguration::iconsFontFamily() const
{
    return String::fromStdString(uiConfiguration()->iconsFontFamily());
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

Color EngravingConfiguration::thumbnailBackgroundColor() const
{
    return Color::white;
}

Color EngravingConfiguration::noteBackgroundColor() const
{
    return Color::white;
}

double EngravingConfiguration::guiScaling() const
{
    return uiConfiguration()->guiScaling();
}

Color EngravingConfiguration::selectionColor(voice_idx_t voice, bool itemVisible) const
{
    Color color = voiceColorKeys[voice].color;

    if (itemVisible) {
        return color;
    }

    constexpr float tint = .6f; // Between 0 and 1. Higher means lighter, lower means darker

    int red = color.red();
    int green = color.green();
    int blue = color.blue();

    return Color(red + tint * (255 - red), green + tint * (255 - green), blue + tint * (255 - blue));
}

void EngravingConfiguration::setSelectionColor(voice_idx_t voiceIndex, Color color)
{
    settings()->setSharedValue(voiceColorKeys[voiceIndex].key, Val(color.toQColor()));
}

mu::async::Channel<voice_idx_t, Color> EngravingConfiguration::selectionColorChanged() const
{
    return m_voiceColorChanged;
}

Color EngravingConfiguration::highlightSelectionColor(voice_idx_t voice) const
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

mu::async::Notification EngravingConfiguration::scoreInversionChanged() const
{
    return m_scoreInversionChanged;
}

const IEngravingConfiguration::DebuggingOptions& EngravingConfiguration::debuggingOptions() const
{
    return m_debuggingOptions.val;
}

void EngravingConfiguration::setDebuggingOptions(const DebuggingOptions& options)
{
    m_debuggingOptions.set(options);
}

mu::async::Notification EngravingConfiguration::debuggingOptionsChanged() const
{
    return m_debuggingOptions.notification;
}

bool EngravingConfiguration::isAccessibleEnabled() const
{
    return accessibilityConfiguration() ? accessibilityConfiguration()->enabled() : false;
}

bool EngravingConfiguration::guitarProImportExperimental() const
{
    return guitarProConfiguration() ? guitarProConfiguration()->experimental() : false;
}

bool EngravingConfiguration::negativeFretsAllowed() const
{
    return guitarProImportExperimental();
}

bool EngravingConfiguration::tablatureParenthesesZIndexWorkaround() const
{
    return guitarProImportExperimental();
}

bool EngravingConfiguration::crossNoteHeadAlwaysBlack() const
{
    return guitarProImportExperimental();
}

bool EngravingConfiguration::enableExperimentalFretCircle() const
{
    return guitarProImportExperimental();
}

void EngravingConfiguration::setGuitarProMultivoiceEnabled(bool multiVoice)
{
    m_multiVoice = multiVoice;
}

bool EngravingConfiguration::guitarProMultivoiceEnabled() const
{
    return m_multiVoice;
}
