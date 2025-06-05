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
#include "engravingconfiguration.h"

#include <cstdlib>

#ifndef NO_QT_SUPPORT
#include <QLocale>
#include <QPageSize>
#endif

#include "global/settings.h"
#include "draw/types/color.h"
#include "dom/mscore.h"
#include "translation.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::draw;
using namespace mu::engraving;

static const Settings::Key DEFAULT_STYLE_FILE_PATH("engraving", "engraving/style/defaultStyleFile");
static const Settings::Key PART_STYLE_FILE_PATH("engraving", "engraving/style/partStyleFile");

static const Settings::Key INVERT_SCORE_COLOR("engraving", "engraving/scoreColorInversion");

static const Settings::Key ALL_VOICES_COLOR("engraving", "engraving/colors/allVoicesColor");
static const Settings::Key FORMATTING_COLOR("engraving", "engraving/colors/formattingColor");
static const Settings::Key FRAME_COLOR("engraving", "engraving/colors/frameColor");
static const Settings::Key SCORE_GREY_COLOR("engraving", "engraving/color/scoreGreyColor");
static const Settings::Key INVISIBLE_COLOR("engraving", "engraving/colors/invisibleColor");
static const Settings::Key UNLINKED_COLOR("engraving", "engraving/colors/unlinkedColor");

static const Settings::Key DYNAMICS_APPLY_TO_ALL_VOICES("engraving", "score/dynamicsApplyToAllVoices");

static const Settings::Key DO_NOT_SAVE_EIDS_FOR_BACK_COMPAT("engraving", "engraving/compat/doNotSaveEIDsForBackCompat");

struct VoiceColor {
    Settings::Key key;
    Color color;
};

static VoiceColor VOICE_COLORS[VOICES + 1];

static const Color UNLINKED_ITEM_COLOR = "#FF9300";

void EngravingConfiguration::init()
{
    static const Color DEFAULT_VOICE_COLORS[VOICES + 1] {
        "#0065BF",
        "#007F00",
        "#C53F00",
        "#C31989",
        "#6038FC", // "all voices"
    };

    settings()->setDefaultValue(DEFAULT_STYLE_FILE_PATH, Val(muse::io::path_t()));
    settings()->valueChanged(DEFAULT_STYLE_FILE_PATH).onReceive(this, [this](const Val& val) {
        m_defaultStyleFilePathChanged.send(val.toPath());
    });

    settings()->setDefaultValue(PART_STYLE_FILE_PATH, Val(muse::io::path_t()));
    settings()->valueChanged(PART_STYLE_FILE_PATH).onReceive(this, [this](const Val& val) {
        m_partStyleFilePathChanged.send(val.toPath());
    });

    settings()->setDefaultValue(INVERT_SCORE_COLOR, Val(false));
    settings()->valueChanged(INVERT_SCORE_COLOR).onReceive(nullptr, [this](const Val&) {
        m_scoreInversionChanged.notify();
    });

    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        Settings::Key key("engraving", "engraving/colors/voice" + std::to_string(voice + 1));

        settings()->setDefaultValue(key, Val(DEFAULT_VOICE_COLORS[voice].toQColor()));
        settings()->setDescription(key, muse::qtrc("engraving", "Voice %1 color").arg(voice + 1).toStdString());
        settings()->setCanBeManuallyEdited(key, true);
        settings()->valueChanged(key).onReceive(this, [this, voice](const Val& val) {
            Color color = val.toQColor();
            VOICE_COLORS[voice].color = color;
            m_voiceColorChanged.send(voice, color);
        });

        Color currentColor = settings()->value(key).toQColor();
        VOICE_COLORS[voice] = VoiceColor { std::move(key), currentColor };
    }

    static constexpr int ALL_VOICES_IDX = VOICES;
    settings()->setDefaultValue(ALL_VOICES_COLOR, Val(DEFAULT_VOICE_COLORS[ALL_VOICES_IDX].toQColor()));
    settings()->setDescription(ALL_VOICES_COLOR, muse::trc("engraving", "All voices color"));
    settings()->setCanBeManuallyEdited(ALL_VOICES_COLOR, true);
    settings()->valueChanged(ALL_VOICES_COLOR).onReceive(this, [&](const Val& val) {
        Color color = val.toQColor();
        VOICE_COLORS[ALL_VOICES_IDX].color = color;
        m_voiceColorChanged.send(ALL_VOICES_IDX, color);
    });
    Color currentColor = settings()->value(ALL_VOICES_COLOR).toQColor();
    VOICE_COLORS[ALL_VOICES_IDX] = VoiceColor { std::move(ALL_VOICES_COLOR), currentColor };

    settings()->setDefaultValue(DYNAMICS_APPLY_TO_ALL_VOICES, Val(true));
    settings()->valueChanged(DYNAMICS_APPLY_TO_ALL_VOICES).onReceive(this, [this](const Val& val) {
        m_dynamicsApplyToAllVoicesChanged.send(val.toBool());
    });

    settings()->setDefaultValue(FRAME_COLOR, Val(Color("#A0A0A4").toQColor()));
    settings()->setDescription(FRAME_COLOR, muse::trc("engraving", "Frame color"));
    settings()->setCanBeManuallyEdited(FRAME_COLOR, true);
    settings()->valueChanged(FRAME_COLOR).onReceive(nullptr, [this](const Val& val) {
        m_frameColorChanged.send(Color::fromQColor(val.toQColor()));
    });

    settings()->setDefaultValue(SCORE_GREY_COLOR, Val(Color("#A0A0A4").toQColor()));
    settings()->setDescription(SCORE_GREY_COLOR, muse::trc("engraving", "Score grey color"));
    settings()->setCanBeManuallyEdited(SCORE_GREY_COLOR, false);

    settings()->setDefaultValue(FORMATTING_COLOR, Val(Color("#C31989").toQColor()));
    settings()->setDescription(FORMATTING_COLOR, muse::trc("engraving", "Formatting color"));
    settings()->setCanBeManuallyEdited(FORMATTING_COLOR, true);
    settings()->valueChanged(FORMATTING_COLOR).onReceive(nullptr, [this](const Val& val) {
        m_formattingColorChanged.send(Color::fromQColor(val.toQColor()));
    });

    settings()->setDefaultValue(INVISIBLE_COLOR, Val(Color("#808080").toQColor()));
    settings()->setDescription(INVISIBLE_COLOR, muse::trc("engraving", "Invisible color"));
    settings()->setCanBeManuallyEdited(INVISIBLE_COLOR, true);
    settings()->valueChanged(INVISIBLE_COLOR).onReceive(nullptr, [this](const Val& val) {
        m_invisibleColorChanged.send(Color::fromQColor(val.toQColor()));
    });

    settings()->setDefaultValue(UNLINKED_COLOR, Val(Color(UNLINKED_ITEM_COLOR).toQColor()));
    settings()->setDescription(UNLINKED_COLOR, muse::trc("engraving", "Desynchronized color"));
    settings()->setCanBeManuallyEdited(UNLINKED_COLOR, true);
    settings()->valueChanged(UNLINKED_COLOR).onReceive(nullptr, [this](const Val& val) {
        m_unlinkedColorChanged.send(Color::fromQColor(val.toQColor()));
    });

    settings()->setDefaultValue(DO_NOT_SAVE_EIDS_FOR_BACK_COMPAT, Val(false));
    settings()->setDescription(DO_NOT_SAVE_EIDS_FOR_BACK_COMPAT, muse::trc("engraving", "Do not save EIDs"));
    settings()->setCanBeManuallyEdited(DO_NOT_SAVE_EIDS_FOR_BACK_COMPAT, false);

    setExperimentalGuitarBendImport(guitarProImportExperimental());
}

muse::io::path_t EngravingConfiguration::appDataPath() const
{
    return globalConfiguration()->appDataPath();
}

muse::io::path_t EngravingConfiguration::defaultStyleFilePath() const
{
    return settings()->value(DEFAULT_STYLE_FILE_PATH).toPath();
}

void EngravingConfiguration::setDefaultStyleFilePath(const muse::io::path_t& path)
{
    settings()->setSharedValue(DEFAULT_STYLE_FILE_PATH, Val(path.toStdString()));
}

async::Channel<muse::io::path_t> EngravingConfiguration::defaultStyleFilePathChanged() const
{
    return m_defaultStyleFilePathChanged;
}

muse::io::path_t EngravingConfiguration::partStyleFilePath() const
{
    return settings()->value(PART_STYLE_FILE_PATH).toPath();
}

void EngravingConfiguration::setPartStyleFilePath(const muse::io::path_t& path)
{
    settings()->setSharedValue(PART_STYLE_FILE_PATH, Val(path.toStdString()));
}

async::Channel<muse::io::path_t> EngravingConfiguration::partStyleFilePathChanged() const
{
    return m_partStyleFilePathChanged;
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
    switch (QLocale::system().territory()) {
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

muse::String EngravingConfiguration::iconsFontFamily() const
{
    return String::fromStdString(uiConfiguration()->iconsFontFamily());
}

Color EngravingConfiguration::defaultColor() const
{
    return Color::BLACK;
}

Color EngravingConfiguration::scoreInversionColor() const
{
    // slightly dulled white for less strain on the eyes
    return Color(220, 220, 220);
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
    return Color::RED;
}

Color EngravingConfiguration::criticalSelectedColor() const
{
    return "#8B0000";
}

Color EngravingConfiguration::thumbnailBackgroundColor() const
{
    return Color::WHITE;
}

Color EngravingConfiguration::noteBackgroundColor() const
{
    return Color::WHITE;
}

Color EngravingConfiguration::fontPrimaryColor() const
{
    return Color(uiConfiguration()->currentTheme().values[muse::ui::ThemeStyleKey::FONT_PRIMARY_COLOR].toString());
}

Color EngravingConfiguration::voiceColor(voice_idx_t voiceIdx) const
{
    return VOICE_COLORS[voiceIdx].color;
}

double EngravingConfiguration::guiScaling() const
{
    return uiConfiguration()->guiScaling();
}

Color EngravingConfiguration::selectionColor(voice_idx_t voice, bool itemVisible, bool itemIsUnlinkedFromScore) const
{
    Color color = itemIsUnlinkedFromScore ? unlinkedColor() : VOICE_COLORS[voice].color;

    if (itemVisible) {
        return color;
    }

    constexpr float tint = .6f; // Between 0 and 1. Higher means lighter, lower means darker

    color.applyTint(tint);

    return color;
}

void EngravingConfiguration::setSelectionColor(voice_idx_t voiceIndex, Color color)
{
    settings()->setSharedValue(VOICE_COLORS[voiceIndex].key, Val(color.toQColor()));
}

muse::async::Channel<voice_idx_t, Color> EngravingConfiguration::selectionColorChanged() const
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

bool EngravingConfiguration::dynamicsApplyToAllVoices() const
{
    return settings()->value(DYNAMICS_APPLY_TO_ALL_VOICES).toBool();
}

void EngravingConfiguration::setDynamicsApplyToAllVoices(bool v)
{
    settings()->setSharedValue(DYNAMICS_APPLY_TO_ALL_VOICES, Val(v));
}

muse::async::Channel<bool> EngravingConfiguration::dynamicsApplyToAllVoicesChanged() const
{
    return m_dynamicsApplyToAllVoicesChanged;
}

muse::async::Notification EngravingConfiguration::scoreInversionChanged() const
{
    return m_scoreInversionChanged;
}

Color EngravingConfiguration::formattingColor() const
{
    return Color::fromQColor(settings()->value(FORMATTING_COLOR).toQColor());
}

muse::async::Channel<Color> EngravingConfiguration::formattingColorChanged() const
{
    return m_formattingColorChanged;
}

Color EngravingConfiguration::frameColor() const
{
    return Color::fromQColor(settings()->value(FRAME_COLOR).toQColor());
}

muse::async::Channel<Color> EngravingConfiguration::frameColorChanged() const
{
    return m_frameColorChanged;
}

Color EngravingConfiguration::scoreGreyColor() const
{
    return Color::fromQColor(settings()->value(SCORE_GREY_COLOR).toQColor());
}

Color EngravingConfiguration::invisibleColor() const
{
    return Color::fromQColor(settings()->value(INVISIBLE_COLOR).toQColor());
}

muse::async::Channel<Color> EngravingConfiguration::invisibleColorChanged() const
{
    return m_invisibleColorChanged;
}

Color EngravingConfiguration::unlinkedColor() const
{
    return Color::fromQColor(settings()->value(UNLINKED_COLOR).toQColor());
}

muse::async::Channel<Color> EngravingConfiguration::unlinkedColorChanged() const
{
    return m_unlinkedColorChanged;
}

const IEngravingConfiguration::DebuggingOptions& EngravingConfiguration::debuggingOptions() const
{
    return m_debuggingOptions.val;
}

void EngravingConfiguration::setDebuggingOptions(const DebuggingOptions& options)
{
    m_debuggingOptions.set(options);
}

muse::async::Notification EngravingConfiguration::debuggingOptionsChanged() const
{
    return m_debuggingOptions.notification;
}

bool EngravingConfiguration::isAccessibleEnabled() const
{
    return accessibilityConfiguration() ? accessibilityConfiguration()->enabled() : false;
}

bool EngravingConfiguration::doNotSaveEIDsForBackCompat() const
{
    return settings()->value(DO_NOT_SAVE_EIDS_FOR_BACK_COMPAT).toBool();
}

void EngravingConfiguration::setDoNotSaveEIDsForBackCompat(bool doNotSave)
{
    settings()->setSharedValue(DO_NOT_SAVE_EIDS_FOR_BACK_COMPAT, Val(doNotSave));
}

bool EngravingConfiguration::guitarProImportExperimental() const
{
    return guitarProConfiguration() ? guitarProConfiguration()->experimental() : false;
}

bool EngravingConfiguration::experimentalGuitarBendImport() const
{
    return m_experimentalGuitarBendImport;
}

void EngravingConfiguration::setExperimentalGuitarBendImport(bool enabled)
{
    m_experimentalGuitarBendImport = enabled;
}

bool EngravingConfiguration::shouldAddParenthesisOnStandardStaff() const
{
    return guitarProImportExperimental();
}

bool EngravingConfiguration::negativeFretsAllowed() const
{
    return guitarProImportExperimental();
}

bool EngravingConfiguration::crossNoteHeadAlwaysBlack() const
{
    return guitarProImportExperimental();
}

bool EngravingConfiguration::enableExperimentalFretCircle() const
{
    return false;
}

void EngravingConfiguration::setGuitarProMultivoiceEnabled(bool multiVoice)
{
    m_multiVoice = multiVoice;
}

bool EngravingConfiguration::guitarProMultivoiceEnabled() const
{
    return m_multiVoice;
}

bool EngravingConfiguration::minDistanceForPartialSkylineCalculated() const
{
    return guitarProImportExperimental();
}

bool EngravingConfiguration::specificSlursLayoutWorkaround() const
{
    return guitarProImportExperimental();
}
