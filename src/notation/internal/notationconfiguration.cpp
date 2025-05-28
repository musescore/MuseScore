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
#include "notationconfiguration.h"

#include "engraving/dom/mscore.h"

#include "log.h"
#include "settings.h"
#include "io/path.h"

#include "notationtypes.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;
using namespace muse::async;
using namespace muse::draw;
using namespace muse::ui;

static const std::string module_name("notation");

static const Settings::Key LIGHT_SCORE_BACKGROUND_COLOR(module_name, "ui/canvas/background/lightTheme_score_background_color");
static const Settings::Key DARK_SCORE_BACKGROUND_COLOR(module_name, "ui/canvas/background/darkTheme_score_background_color");
static const Settings::Key HC_BLACK_SCORE_BACKGROUND_COLOR(module_name, "ui/canvas/background/hc_black_score_background_color");
static const Settings::Key HC_WHITE_SCORE_BACKGROUND_COLOR(module_name, "ui/canvas/background/hc_white_score_background_color");
static const Settings::Key BACKGROUND_WALLPAPER_PATH(module_name, "ui/canvas/background/wallpaper");
static const Settings::Key BACKGROUND_USE_COLOR(module_name, "ui/canvas/background/useColor");

static const Settings::Key FOREGROUND_COLOR(module_name, "ui/canvas/foreground/color");
static const Settings::Key FOREGROUND_WALLPAPER_PATH(module_name, "ui/canvas/foreground/wallpaper");
static const Settings::Key FOREGROUND_USE_COLOR(module_name, "ui/canvas/foreground/useColor");

static const Settings::Key NOTE_INPUT_PREVIEW_COLOR(module_name, "ui/canvas/noteInputPreviewColor");

static const Settings::Key USE_NOTE_INPUT_CURSOR_IN_INPUT_BY_DURATION(module_name,
                                                                      "ui/canvas/useNoteInputCursorInInputByDuration");

static const Settings::Key THIN_NOTE_INPUT_CURSOR(module_name, "ui/canvas/thinNoteInputCursor");

static const Settings::Key SELECTION_PROXIMITY(module_name, "ui/canvas/misc/selectionProximity");

static const Settings::Key DEFAULT_ZOOM_TYPE(module_name, "ui/canvas/zoomDefaultType");
static const Settings::Key DEFAULT_ZOOM(module_name, "ui/canvas/zoomDefaultLevel");
static const Settings::Key KEYBOARD_ZOOM_PRECISION(module_name, "ui/canvas/zoomPrecisionKeyboard");
static const Settings::Key MOUSE_ZOOM_PRECISION(module_name, "ui/canvas/zoomPrecisionMouse");

static const Settings::Key USER_STYLES_PATH(module_name, "application/paths/myStyles");
static const Settings::Key USER_MUSIC_FONTS_PATH(module_name, "application/paths/myMusicFonts");

static const Settings::Key DEFAULT_NOTE_INPUT_METHOD(module_name, "score/defaultInputMethod");

static const Settings::Key ADD_ACCIDENTAL_ARTICULATIONS_DOTS_TO_NEXT_NOTE_ENTERED(module_name,
                                                                                  "score/addAccidentalDotsArticulationsToNextNoteEntered");

static const Settings::Key IS_MIDI_INPUT_ENABLED(module_name, "io/midi/enableInput");
static const Settings::Key START_NOTE_INPUT_AT_SELECTION_WHEN_PRESSING_MIDI_KEY(module_name,
                                                                                "score/startNoteInputAtSelectionWhenPressingMidiKey");
static const Settings::Key USE_MIDI_INPUT_WRITTEN_PITCH(module_name, "io/midi/useWrittenPitch");
static const Settings::Key IS_AUTOMATICALLY_PAN_ENABLED(module_name, "application/playback/panPlayback");
static const Settings::Key PLAYBACK_SMOOTH_PANNING(module_name, "application/playback/smoothPan");
static const Settings::Key IS_PLAY_REPEATS_ENABLED(module_name, "application/playback/playRepeats");
static const Settings::Key IS_PLAY_CHORD_SYMBOLS_ENABLED(module_name, "application/playback/playChordSymbols");
static const Settings::Key IS_PLAY_PREVIEW_NOTES_IN_INPUT_BY_DURATION_ENABLED(module_name,
                                                                              "application/playback/playPreviewNotesInInputByDuration");
static const Settings::Key IS_METRONOME_ENABLED(module_name, "application/playback/metronomeEnabled");
static const Settings::Key IS_COUNT_IN_ENABLED(module_name, "application/playback/countInEnabled");

static const Settings::Key TOOLBAR_KEY(module_name, "ui/toolbar/");

static const Settings::Key IS_CANVAS_ORIENTATION_VERTICAL_KEY(module_name, "ui/canvas/scroll/verticalOrientation");

static const Settings::Key COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE(module_name, "score/note/warnPitchRange");
static const Settings::Key WARN_GUITAR_BENDS(module_name, "score/note/warnGuitarBends");
static const Settings::Key REALTIME_DELAY(module_name, "io/midi/realtimeDelay");
static const Settings::Key NOTE_DEFAULT_PLAY_DURATION(module_name, "score/note/defaultPlayDuration");

static const Settings::Key FIRST_SCORE_ORDER_LIST_KEY(module_name, "application/paths/scoreOrderList1");
static const Settings::Key SECOND_SCORE_ORDER_LIST_KEY(module_name, "application/paths/scoreOrderList2");

static const Settings::Key IS_SNAPPED_TO_VERTICAL_GRID_KEY(module_name,  "ui/application/raster/isSnappedToVerticalGrid");
static const Settings::Key IS_SNAPPED_TO_HORIZONTAL_GRID_KEY(module_name,  "ui/application/raster/isSnappedToHorizontalGrid");
static const Settings::Key HORIZONTAL_GRID_SIZE_KEY(module_name,  "ui/application/raster/horizontal");
static const Settings::Key VERTICAL_GRID_SIZE_KEY(module_name,  "ui/application/raster/vertical");

static const Settings::Key NEED_TO_SHOW_ADD_TEXT_ERROR_MESSAGE_KEY(module_name,  "ui/dialogs/needToShowAddTextErrorMessage");
static const Settings::Key NEED_TO_SHOW_ADD_FIGURED_BASS_ERROR_MESSAGE_KEY(module_name,  "ui/dialogs/needToShowAddFiguredBassErrorMessage");
static const Settings::Key NEED_TO_SHOW_ADD_GUITAR_BEND_ERROR_MESSAGE_KEY(module_name,  "ui/dialogs/needToShowAddGuitarBendErrorMessage");

static const Settings::Key PIANO_KEYBOARD_NUMBER_OF_KEYS(module_name,  "pianoKeyboard/numberOfKeys");

static const Settings::Key USE_NEW_PERCUSSION_PANEL_KEY(module_name,  "ui/useNewPercussionPanel");
static const Settings::Key PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY(module_name,  "ui/percussionPanelUseNotationPreview");
static const Settings::Key PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY(module_name,  "ui/percussionPanelAutoShowMode");
static const Settings::Key AUTO_CLOSE_PERCUSSION_PANEL_KEY(module_name, "ui/autoClosePercussionPanel");
static const Settings::Key SHOW_PERCUSSION_PANEL_SWAP_DIALOG(module_name,  "ui/showPercussionPanelPadSwapDialog");
static const Settings::Key PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS(module_name,  "ui/percussionPanelMoveMidiNotesAndShortcuts");

static const Settings::Key STYLE_FILE_IMPORT_PATH_KEY(module_name, "import/style/styleFile");

static constexpr int DEFAULT_GRID_SIZE_SPATIUM = 2;

static const std::string BY_NOTE_NAME_INPUT_METHOD("BY_NOTE_NAME");

static const std::map<NoteInputMethod, std::string> NOTE_INPUT_METHOD_TO_STR {
    { NoteInputMethod::BY_NOTE_NAME, BY_NOTE_NAME_INPUT_METHOD },
    { NoteInputMethod::BY_DURATION, "BY_DURATION" },
    { NoteInputMethod::REPITCH, "REPITCH" },
    { NoteInputMethod::RHYTHM, "RHYTHM" },
    { NoteInputMethod::REALTIME_AUTO, "REALTIME_AUTO" },
    { NoteInputMethod::REALTIME_MANUAL, "REALTIME_MANUAL" },
    { NoteInputMethod::TIMEWISE, "TIMEWISE" },
};

void NotationConfiguration::init()
{
    settings()->setDefaultValue(BACKGROUND_USE_COLOR, Val(true));
    settings()->valueChanged(BACKGROUND_USE_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    uiConfiguration()->currentThemeChanged().onNotify(this, [this]() {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(LIGHT_SCORE_BACKGROUND_COLOR, Val(QColor("#BCC1CC")));
    settings()->valueChanged(LIGHT_SCORE_BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(DARK_SCORE_BACKGROUND_COLOR, Val(QColor("#27272B")));
    settings()->valueChanged(DARK_SCORE_BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(HC_BLACK_SCORE_BACKGROUND_COLOR, Val(QColor("#000000")));
    settings()->valueChanged(HC_BLACK_SCORE_BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(HC_WHITE_SCORE_BACKGROUND_COLOR, Val(QColor("#000000")));
    settings()->valueChanged(HC_WHITE_SCORE_BACKGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(BACKGROUND_WALLPAPER_PATH, Val());
    settings()->valueChanged(BACKGROUND_WALLPAPER_PATH).onReceive(nullptr, [this](const Val&) {
        m_backgroundChanged.notify();
    });

    settings()->setDefaultValue(FOREGROUND_USE_COLOR, Val(true));
    settings()->valueChanged(FOREGROUND_USE_COLOR).onReceive(nullptr, [this](const Val&) {
        m_foregroundChanged.notify();
    });

    settings()->setDefaultValue(FOREGROUND_COLOR, Val(QColor("#f9f9f9")));
    settings()->valueChanged(FOREGROUND_COLOR).onReceive(nullptr, [this](const Val&) {
        m_foregroundChanged.notify();
    });

    settings()->setDefaultValue(NOTE_INPUT_PREVIEW_COLOR, Val(selectionColor()));
    settings()->setCanBeManuallyEdited(NOTE_INPUT_PREVIEW_COLOR, true);
    settings()->setDescription(NOTE_INPUT_PREVIEW_COLOR, muse::trc("notation", "Note input preview note color"));

    settings()->setDefaultValue(USE_NOTE_INPUT_CURSOR_IN_INPUT_BY_DURATION, Val(false));
    settings()->valueChanged(USE_NOTE_INPUT_CURSOR_IN_INPUT_BY_DURATION).onReceive(nullptr, [this](const Val&) {
        m_useNoteInputCursorInInputByDurationChanged.notify();
    });

    settings()->setDefaultValue(THIN_NOTE_INPUT_CURSOR, Val(false)); // accessible via DevTools/Settings

    settings()->setDefaultValue(FOREGROUND_WALLPAPER_PATH, Val());
    settings()->valueChanged(FOREGROUND_WALLPAPER_PATH).onReceive(nullptr, [this](const Val&) {
        m_foregroundChanged.notify();
    });

    settings()->setDefaultValue(DEFAULT_ZOOM_TYPE, Val(ZoomType::Percentage));
    settings()->valueChanged(DEFAULT_ZOOM_TYPE).onReceive(this, [this](const Val&) {
        m_defaultZoomChanged.notify();
    });
    settings()->setDefaultValue(DEFAULT_ZOOM, Val(100));
    settings()->valueChanged(DEFAULT_ZOOM).onReceive(this, [this](const Val&) {
        m_defaultZoomChanged.notify();
    });
    settings()->setDefaultValue(KEYBOARD_ZOOM_PRECISION, Val(2));
    settings()->setDefaultValue(MOUSE_ZOOM_PRECISION, Val(6));
    settings()->valueChanged(MOUSE_ZOOM_PRECISION).onReceive(this, [this](const Val&) {
        m_mouseZoomPrecisionChanged.notify();
    });

    settings()->setDefaultValue(USER_STYLES_PATH, Val(globalConfiguration()->userDataPath() + "/Styles"));
    settings()->valueChanged(USER_STYLES_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userStylesPathChanged.send(val.toPath());
    });

    if (!userStylesPath().empty()) {
        fileSystem()->makePath(userStylesPath());

#if !defined(Q_OS_LINUX)
        // Create shortcut or symlink to global styles folder. Doesn't overwrite an existing file
        // or link, which means it won't work on Linux because the global styles folder is inside
        // the AppImage, and the AppImage mountpoint changes on each run. TODO: Check for existing
        // link, read it, and if necessary update it. This would make it work on Linux too.
        fileSystem()->makeLink(
            globalConfiguration()->appDataPath() + "/styles",
            userStylesPath() + "/Built-in styles"
            );
#endif
    }

    settings()->setDefaultValue(USER_MUSIC_FONTS_PATH, Val(io::path_t {}));
    settings()->valueChanged(USER_MUSIC_FONTS_PATH).onReceive(this, [this](const Val& val) {
        m_userMusicFontsPathChanged.send(val.toString());
    });

    settings()->setDefaultValue(DEFAULT_NOTE_INPUT_METHOD, Val(BY_NOTE_NAME_INPUT_METHOD));
    settings()->valueChanged(DEFAULT_NOTE_INPUT_METHOD).onReceive(this, [this](const Val&) {
        m_defaultNoteInputMethodChanged.notify();
    });

    settings()->setDefaultValue(ADD_ACCIDENTAL_ARTICULATIONS_DOTS_TO_NEXT_NOTE_ENTERED, Val(true));
    settings()->valueChanged(ADD_ACCIDENTAL_ARTICULATIONS_DOTS_TO_NEXT_NOTE_ENTERED).onReceive(this, [this](const Val&) {
        m_addAccidentalDotsArticulationsToNextNoteEnteredChanged.notify();
    });

    settings()->setDefaultValue(SELECTION_PROXIMITY, Val(2));
    settings()->valueChanged(SELECTION_PROXIMITY).onReceive(this, [this](const Val& val) {
        m_selectionProximityChanged.send(val.toInt());
    });

    settings()->setDefaultValue(IS_MIDI_INPUT_ENABLED, Val(true));
    settings()->valueChanged(IS_MIDI_INPUT_ENABLED).onReceive(this, [this](const Val&) {
        m_isMidiInputEnabledChanged.notify();
    });

    settings()->setDefaultValue(START_NOTE_INPUT_AT_SELECTION_WHEN_PRESSING_MIDI_KEY, Val(true));
    settings()->valueChanged(START_NOTE_INPUT_AT_SELECTION_WHEN_PRESSING_MIDI_KEY).onReceive(this, [this](const Val&) {
        m_startNoteInputAtSelectionWhenPressingMidiKeyChanged.notify();
    });

    settings()->setDefaultValue(IS_AUTOMATICALLY_PAN_ENABLED, Val(true));
    settings()->setDefaultValue(IS_PLAY_REPEATS_ENABLED, Val(true));

    settings()->setDefaultValue(IS_METRONOME_ENABLED, Val(false));
    settings()->valueChanged(IS_METRONOME_ENABLED).onReceive(this, [this](const Val&) {
        m_isMetronomeEnabledChanged.notify();
    });

    settings()->setDefaultValue(IS_COUNT_IN_ENABLED, Val(false));

    settings()->setDefaultValue(PLAYBACK_SMOOTH_PANNING, Val(false));
    settings()->setDescription(PLAYBACK_SMOOTH_PANNING, muse::trc("notation", "Smooth panning"));
    settings()->setCanBeManuallyEdited(PLAYBACK_SMOOTH_PANNING, true);

    settings()->setDefaultValue(IS_PLAY_CHORD_SYMBOLS_ENABLED, Val(true));
    settings()->valueChanged(IS_PLAY_CHORD_SYMBOLS_ENABLED).onReceive(nullptr, [this](const Val&) {
        m_isPlayChordSymbolsChanged.notify();
    });

    settings()->setDefaultValue(IS_PLAY_PREVIEW_NOTES_IN_INPUT_BY_DURATION_ENABLED, Val(true));
    settings()->valueChanged(IS_PLAY_PREVIEW_NOTES_IN_INPUT_BY_DURATION_ENABLED).onReceive(nullptr, [this](const Val&) {
        m_isPlayNotesPreviewInInputByDurationChanged.notify();
    });

    settings()->setDefaultValue(IS_CANVAS_ORIENTATION_VERTICAL_KEY, Val(false));
    settings()->valueChanged(IS_CANVAS_ORIENTATION_VERTICAL_KEY).onReceive(nullptr, [this](const Val&) {
        m_canvasOrientationChanged.send(canvasOrientation().val);
    });

    settings()->setDefaultValue(COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE, Val(true));
    settings()->valueChanged(COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE).onReceive(this, [this](const Val& val) {
        m_colorNotesOutsideOfUsablePitchRangeChanged.send(val.toBool());
    });
    settings()->setDefaultValue(WARN_GUITAR_BENDS, Val(true));
    settings()->valueChanged(WARN_GUITAR_BENDS).onReceive(this, [this](const Val& val) {
        m_warnGuitarBendsChanged.send(val.toBool());
    });
    settings()->setDefaultValue(REALTIME_DELAY, Val(750));
    settings()->valueChanged(REALTIME_DELAY).onReceive(this, [this](const Val& val) {
        m_delayBetweenNotesInRealTimeModeMillisecondsChanged.send(val.toInt());
    });
    settings()->setDefaultValue(NOTE_DEFAULT_PLAY_DURATION, Val(500));
    settings()->valueChanged(NOTE_DEFAULT_PLAY_DURATION).onReceive(this, [this](const Val& val) {
        m_notePlayDurationMillisecondsChanged.send(val.toInt());
    });

    settings()->setDefaultValue(STYLE_FILE_IMPORT_PATH_KEY, Val(""));
    settings()->valueChanged(STYLE_FILE_IMPORT_PATH_KEY).onReceive(this, [this](const Val& val) {
        m_styleFileImportPathChanged.send(val.toString());
    });

    settings()->setDefaultValue(FIRST_SCORE_ORDER_LIST_KEY,
                                Val(globalConfiguration()->appDataPath().toStdString() + "instruments/orders.xml"));
    settings()->valueChanged(FIRST_SCORE_ORDER_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_scoreOrderListPathsChanged.notify();
    });

    settings()->setDefaultValue(SECOND_SCORE_ORDER_LIST_KEY, Val(""));
    settings()->valueChanged(SECOND_SCORE_ORDER_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_scoreOrderListPathsChanged.notify();
    });

    settings()->setDefaultValue(HORIZONTAL_GRID_SIZE_KEY, Val(DEFAULT_GRID_SIZE_SPATIUM));
    settings()->setDefaultValue(VERTICAL_GRID_SIZE_KEY, Val(DEFAULT_GRID_SIZE_SPATIUM));

    settings()->setDefaultValue(NEED_TO_SHOW_ADD_TEXT_ERROR_MESSAGE_KEY, Val(true));
    settings()->setDefaultValue(NEED_TO_SHOW_ADD_FIGURED_BASS_ERROR_MESSAGE_KEY, Val(true));
    settings()->setDefaultValue(NEED_TO_SHOW_ADD_GUITAR_BEND_ERROR_MESSAGE_KEY, Val(true));

    settings()->setDefaultValue(PIANO_KEYBOARD_NUMBER_OF_KEYS, Val(88));
    m_pianoKeyboardNumberOfKeys.val = settings()->value(PIANO_KEYBOARD_NUMBER_OF_KEYS).toInt();
    settings()->valueChanged(PIANO_KEYBOARD_NUMBER_OF_KEYS).onReceive(this, [this](const Val& val) {
        m_pianoKeyboardNumberOfKeys.set(val.toInt());
    });

    settings()->setDefaultValue(USE_MIDI_INPUT_WRITTEN_PITCH, Val(true));
    m_midiInputUseWrittenPitch.val = settings()->value(USE_MIDI_INPUT_WRITTEN_PITCH).toBool();
    settings()->valueChanged(USE_MIDI_INPUT_WRITTEN_PITCH).onReceive(this, [this](const Val& val) {
        m_midiInputUseWrittenPitch.set(val.toBool());
    });

    settings()->setDefaultValue(USE_NEW_PERCUSSION_PANEL_KEY, Val(true));
    settings()->valueChanged(USE_NEW_PERCUSSION_PANEL_KEY).onReceive(this, [this](const Val&) {
        m_useNewPercussionPanelChanged.notify();
    });

    settings()->setDefaultValue(PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY, Val(false));
    settings()->valueChanged(PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY).onReceive(this, [this](const Val&) {
        m_percussionPanelUseNotationPreviewChanged.notify();
    });

    settings()->setDefaultValue(PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY, Val(PercussionPanelAutoShowMode::UNPITCHED_STAFF));
    settings()->valueChanged(PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY).onReceive(this, [this](const Val&) {
        m_percussionPanelAutoShowModeChanged.notify();
    });

    settings()->setDefaultValue(AUTO_CLOSE_PERCUSSION_PANEL_KEY, Val(true));
    settings()->valueChanged(AUTO_CLOSE_PERCUSSION_PANEL_KEY).onReceive(this, [this](const Val&) {
        m_autoClosePercussionPanelChanged.notify();
    });

    settings()->setDefaultValue(SHOW_PERCUSSION_PANEL_SWAP_DIALOG, Val(true));
    settings()->valueChanged(SHOW_PERCUSSION_PANEL_SWAP_DIALOG).onReceive(this, [this](const Val&) {
        m_showPercussionPanelPadSwapDialogChanged.notify();
    });

    settings()->setDefaultValue(PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS, Val(true));
    settings()->valueChanged(PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS).onReceive(this, [this](const Val&) {
        m_percussionPanelMoveMidiNotesAndShortcutsChanged.notify();
    });

    engravingConfiguration()->scoreInversionChanged().onNotify(this, [this]() {
        m_foregroundChanged.notify();
    });

    engravingConfiguration()->formattingColorChanged().onReceive(this, [this](const Color&) {
        m_foregroundChanged.notify();
    });

    engravingConfiguration()->invisibleColorChanged().onReceive(this, [this](const Color&) {
        m_foregroundChanged.notify();
    });

    engravingConfiguration()->unlinkedColorChanged().onReceive(this, [this](const Color&) {
        m_foregroundChanged.notify();
    });

    mu::engraving::MScore::warnPitchRange = colorNotesOutsideOfUsablePitchRange();
    mu::engraving::MScore::warnGuitarBends = warnGuitarBends();
    mu::engraving::MScore::defaultPlayDuration = notePlayDurationMilliseconds();

    mu::engraving::MScore::setHRaster(DEFAULT_GRID_SIZE_SPATIUM);
    mu::engraving::MScore::setVRaster(DEFAULT_GRID_SIZE_SPATIUM);

    context()->currentProjectChanged().onNotify(this, [this]() {
        resetStyleDialogPageIndices();
    });
}

QColor NotationConfiguration::backgroundColor() const
{
    if (uiConfiguration()->currentTheme().codeKey == LIGHT_THEME_CODE) {
        return settings()->value(LIGHT_SCORE_BACKGROUND_COLOR).toQColor();
    } else if (uiConfiguration()->currentTheme().codeKey == DARK_THEME_CODE) {
        return settings()->value(DARK_SCORE_BACKGROUND_COLOR).toQColor();
    } else if (uiConfiguration()->currentTheme().codeKey == HIGH_CONTRAST_BLACK_THEME_CODE) {
        return settings()->value(HC_BLACK_SCORE_BACKGROUND_COLOR).toQColor();
    } else {
        return settings()->value(HC_WHITE_SCORE_BACKGROUND_COLOR).toQColor();
    }
}

void NotationConfiguration::setBackgroundColor(const QColor& color)
{
    if (uiConfiguration()->currentTheme().codeKey == LIGHT_THEME_CODE) {
        settings()->setSharedValue(LIGHT_SCORE_BACKGROUND_COLOR, Val(color));
    } else if (uiConfiguration()->currentTheme().codeKey == DARK_THEME_CODE) {
        settings()->setSharedValue(DARK_SCORE_BACKGROUND_COLOR, Val(color));
    } else if (uiConfiguration()->currentTheme().codeKey == HIGH_CONTRAST_BLACK_THEME_CODE) {
        settings()->setSharedValue(HC_BLACK_SCORE_BACKGROUND_COLOR, Val(color));
    } else {
        settings()->setSharedValue(HC_WHITE_SCORE_BACKGROUND_COLOR, Val(color));
    }
}

muse::io::path_t NotationConfiguration::backgroundWallpaperPath() const
{
    return settings()->value(BACKGROUND_WALLPAPER_PATH).toString();
}

const QPixmap& NotationConfiguration::backgroundWallpaper() const
{
    muse::io::path_t path = backgroundWallpaperPath();

    static QPixmap wallpaper;
    static muse::io::path_t lastPath;

    if (path.empty()) {
        wallpaper = QPixmap();
    } else if (path != lastPath) {
        wallpaper = QPixmap(path.toQString());
    }

    lastPath = path;

    return wallpaper;
}

void NotationConfiguration::setBackgroundWallpaperPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(BACKGROUND_WALLPAPER_PATH, Val(path.toStdString()));
}

bool NotationConfiguration::backgroundUseColor() const
{
    return settings()->value(BACKGROUND_USE_COLOR).toBool();
}

void NotationConfiguration::setBackgroundUseColor(bool value)
{
    settings()->setSharedValue(BACKGROUND_USE_COLOR, Val(value));
}

void NotationConfiguration::resetBackground()
{
    settings()->setSharedValue(LIGHT_SCORE_BACKGROUND_COLOR, settings()->defaultValue(LIGHT_SCORE_BACKGROUND_COLOR));
    settings()->setSharedValue(DARK_SCORE_BACKGROUND_COLOR, settings()->defaultValue(DARK_SCORE_BACKGROUND_COLOR));
    settings()->setSharedValue(HC_BLACK_SCORE_BACKGROUND_COLOR, settings()->defaultValue(HC_BLACK_SCORE_BACKGROUND_COLOR));
    settings()->setSharedValue(HC_WHITE_SCORE_BACKGROUND_COLOR, settings()->defaultValue(HC_WHITE_SCORE_BACKGROUND_COLOR));

    settings()->setSharedValue(BACKGROUND_USE_COLOR, settings()->defaultValue(BACKGROUND_USE_COLOR));

    settings()->setSharedValue(BACKGROUND_WALLPAPER_PATH, settings()->defaultValue(BACKGROUND_WALLPAPER_PATH));
}

muse::async::Notification NotationConfiguration::backgroundChanged() const
{
    return m_backgroundChanged;
}

QColor NotationConfiguration::foregroundColor() const
{
    if (engravingConfiguration()->scoreInversionEnabled()) {
        return QColorConstants::Black;
    }

    return settings()->value(FOREGROUND_COLOR).toQColor();
}

void NotationConfiguration::setForegroundColor(const QColor& color)
{
    settings()->setSharedValue(FOREGROUND_COLOR, Val(color));
}

muse::io::path_t NotationConfiguration::foregroundWallpaperPath() const
{
    return settings()->value(FOREGROUND_WALLPAPER_PATH).toString();
}

const QPixmap& NotationConfiguration::foregroundWallpaper() const
{
    muse::io::path_t path = foregroundWallpaperPath();

    static QPixmap wallpaper;
    static muse::io::path_t lastPath;

    if (path.empty()) {
        wallpaper = QPixmap();
    } else if (path != lastPath) {
        wallpaper = QPixmap(path.toQString());
    }

    lastPath = path;

    return wallpaper;
}

void NotationConfiguration::setForegroundWallpaperPath(const muse::io::path_t& path)
{
    return settings()->setSharedValue(FOREGROUND_WALLPAPER_PATH, Val(path.toStdString()));
}

bool NotationConfiguration::foregroundUseColor() const
{
    return settings()->value(FOREGROUND_USE_COLOR).toBool();
}

void NotationConfiguration::setForegroundUseColor(bool value)
{
    settings()->setSharedValue(FOREGROUND_USE_COLOR, Val(value));
}

void NotationConfiguration::resetForeground()
{
    settings()->setSharedValue(FOREGROUND_COLOR, settings()->defaultValue(FOREGROUND_COLOR));
    settings()->setSharedValue(FOREGROUND_USE_COLOR, settings()->defaultValue(FOREGROUND_USE_COLOR));
    settings()->setSharedValue(FOREGROUND_WALLPAPER_PATH, settings()->defaultValue(FOREGROUND_WALLPAPER_PATH));

    engravingConfiguration()->setScoreInversionEnabled(false);
}

muse::async::Notification NotationConfiguration::foregroundChanged() const
{
    return m_foregroundChanged;
}

muse::io::path_t NotationConfiguration::wallpapersDefaultDirPath() const
{
    return globalConfiguration()->appDataPath() + "/wallpapers";
}

QColor NotationConfiguration::borderColor() const
{
    if (uiConfiguration()->isHighContrast()) {
        return QColorConstants::White;
    } else {
        return QColor(0, 0, 0, 102);
    }
}

int NotationConfiguration::borderWidth() const
{
    if (uiConfiguration()->isHighContrast()) {
        return 10;
    } else {
        return 1;
    }
}

QColor NotationConfiguration::playbackCursorColor() const
{
    QColor color = selectionColor();
    color.setAlpha(178);
    return color;
}

int NotationConfiguration::cursorOpacity() const
{
    return 50;
}

bool NotationConfiguration::thinNoteInputCursor() const
{
    return settings()->value(THIN_NOTE_INPUT_CURSOR).toBool();
}

QColor NotationConfiguration::loopMarkerColor() const
{
    return QColor(0x2456AA);
}

QColor NotationConfiguration::selectionColor(engraving::voice_idx_t voiceIndex) const
{
    return engravingConfiguration()->selectionColor(voiceIndex).toQColor();
}

QColor NotationConfiguration::dropRectColor() const
{
    QColor color = selectionColor();
    color.setAlpha(80);
    return color;
}

muse::draw::Color NotationConfiguration::noteInputPreviewColor() const
{
    return settings()->value(NOTE_INPUT_PREVIEW_COLOR).toQColor();
}

bool NotationConfiguration::useNoteInputCursorInInputByDuration() const
{
    return settings()->value(USE_NOTE_INPUT_CURSOR_IN_INPUT_BY_DURATION).toBool();
}

void NotationConfiguration::setUseNoteInputCursorInInputByDuration(bool use)
{
    settings()->setSharedValue(USE_NOTE_INPUT_CURSOR_IN_INPUT_BY_DURATION, Val(use));
}

muse::async::Notification NotationConfiguration::useNoteInputCursorInInputByDurationChanged() const
{
    return m_useNoteInputCursorInInputByDurationChanged;
}

int NotationConfiguration::selectionProximity() const
{
    return settings()->value(SELECTION_PROXIMITY).toInt();
}

void NotationConfiguration::setSelectionProximity(int proximity)
{
    settings()->setSharedValue(SELECTION_PROXIMITY, Val(proximity));
}

Channel<int> NotationConfiguration::selectionProximityChanged() const
{
    return m_selectionProximityChanged;
}

ZoomType NotationConfiguration::defaultZoomType() const
{
    return settings()->value(DEFAULT_ZOOM_TYPE).toEnum<ZoomType>();
}

void NotationConfiguration::setDefaultZoomType(ZoomType zoomType)
{
    settings()->setSharedValue(DEFAULT_ZOOM_TYPE, Val(zoomType));
}

int NotationConfiguration::defaultZoom() const
{
    return settings()->value(DEFAULT_ZOOM).toInt();
}

void NotationConfiguration::setDefaultZoom(int zoomPercentage)
{
    settings()->setSharedValue(DEFAULT_ZOOM, Val(zoomPercentage));
}

Notification NotationConfiguration::defaultZoomChanged() const
{
    return m_defaultZoomChanged;
}

qreal NotationConfiguration::scalingFromZoomPercentage(int zoomPercentage) const
{
    return zoomPercentage / 100.0 * notationScaling();
}

int NotationConfiguration::zoomPercentageFromScaling(qreal scaling) const
{
    return std::round(scaling * 100.0 / notationScaling());
}

QList<int> NotationConfiguration::possibleZoomPercentageList() const
{
    return {
        5, 10, 15, 25, 50, 75, 100, 150, 200, 400, 800, 1600
    };
}

int NotationConfiguration::mouseZoomPrecision() const
{
    return settings()->value(MOUSE_ZOOM_PRECISION).toInt();
}

void NotationConfiguration::setMouseZoomPrecision(int precision)
{
    settings()->setSharedValue(MOUSE_ZOOM_PRECISION, Val(precision));
}

Notification NotationConfiguration::mouseZoomPrecisionChanged() const
{
    return m_mouseZoomPrecisionChanged;
}

std::string NotationConfiguration::fontFamily() const
{
    return uiConfiguration()->fontFamily();
}

int NotationConfiguration::fontSize() const
{
    return uiConfiguration()->fontSize(FontSizeType::BODY);
}

muse::io::path_t NotationConfiguration::userStylesPath() const
{
    return settings()->value(USER_STYLES_PATH).toPath();
}

void NotationConfiguration::setUserStylesPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(USER_STYLES_PATH, Val(path));
}

muse::async::Channel<muse::io::path_t> NotationConfiguration::userStylesPathChanged() const
{
    return m_userStylesPathChanged;
}

muse::io::path_t NotationConfiguration::defaultStyleFilePath() const
{
    return engravingConfiguration()->defaultStyleFilePath();
}

void NotationConfiguration::setDefaultStyleFilePath(const muse::io::path_t& path)
{
    engravingConfiguration()->setDefaultStyleFilePath(path.toQString());
}

async::Channel<muse::io::path_t> NotationConfiguration::defaultStyleFilePathChanged() const
{
    return engravingConfiguration()->defaultStyleFilePathChanged();
}

muse::io::path_t NotationConfiguration::partStyleFilePath() const
{
    return engravingConfiguration()->partStyleFilePath();
}

void NotationConfiguration::setPartStyleFilePath(const muse::io::path_t& path)
{
    engravingConfiguration()->setPartStyleFilePath(path.toQString());
}

async::Channel<muse::io::path_t> NotationConfiguration::partStyleFilePathChanged() const
{
    return engravingConfiguration()->partStyleFilePathChanged();
}

muse::io::path_t NotationConfiguration::userMusicFontsPath() const
{
    return settings()->value(USER_MUSIC_FONTS_PATH).toPath();
}

void NotationConfiguration::setUserMusicFontsPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(USER_MUSIC_FONTS_PATH, Val(path));
}

muse::async::Channel<muse::io::path_t> NotationConfiguration::userMusicFontsPathChanged() const
{
    return m_userMusicFontsPathChanged;
}

NoteInputMethod NotationConfiguration::defaultNoteInputMethod() const
{
    std::string str = settings()->value(DEFAULT_NOTE_INPUT_METHOD).toString();
    return muse::key(NOTE_INPUT_METHOD_TO_STR, str, NoteInputMethod::BY_NOTE_NAME);
}

void NotationConfiguration::setDefaultNoteInputMethod(NoteInputMethod method)
{
    std::string str = muse::value(NOTE_INPUT_METHOD_TO_STR, method);
    settings()->setSharedValue(DEFAULT_NOTE_INPUT_METHOD, Val(str));
}

async::Notification NotationConfiguration::defaultNoteInputMethodChanged() const
{
    return m_defaultNoteInputMethodChanged;
}

bool NotationConfiguration::addAccidentalDotsArticulationsToNextNoteEntered() const
{
    return settings()->value(ADD_ACCIDENTAL_ARTICULATIONS_DOTS_TO_NEXT_NOTE_ENTERED).toBool();
}

void NotationConfiguration::setAddAccidentalDotsArticulationsToNextNoteEntered(bool value)
{
    settings()->setSharedValue(ADD_ACCIDENTAL_ARTICULATIONS_DOTS_TO_NEXT_NOTE_ENTERED, Val(value));
}

muse::async::Notification NotationConfiguration::addAccidentalDotsArticulationsToNextNoteEnteredChanged() const
{
    return m_addAccidentalDotsArticulationsToNextNoteEnteredChanged;
}

bool NotationConfiguration::isMidiInputEnabled() const
{
    return settings()->value(IS_MIDI_INPUT_ENABLED).toBool();
}

void NotationConfiguration::setIsMidiInputEnabled(bool enabled)
{
    settings()->setSharedValue(IS_MIDI_INPUT_ENABLED, Val(enabled));
}

async::Notification NotationConfiguration::isMidiInputEnabledChanged() const
{
    return m_isMidiInputEnabledChanged;
}

bool NotationConfiguration::startNoteInputAtSelectionWhenPressingMidiKey() const
{
    return settings()->value(START_NOTE_INPUT_AT_SELECTION_WHEN_PRESSING_MIDI_KEY).toBool();
}

void NotationConfiguration::setStartNoteInputAtSelectionWhenPressingMidiKey(bool value)
{
    settings()->setSharedValue(START_NOTE_INPUT_AT_SELECTION_WHEN_PRESSING_MIDI_KEY, Val(value));
}

async::Notification NotationConfiguration::startNoteInputAtSelectionWhenPressingMidiKeyChanged() const
{
    return m_startNoteInputAtSelectionWhenPressingMidiKeyChanged;
}

bool NotationConfiguration::isAutomaticallyPanEnabled() const
{
    return settings()->value(IS_AUTOMATICALLY_PAN_ENABLED).toBool();
}

void NotationConfiguration::setIsAutomaticallyPanEnabled(bool enabled)
{
    settings()->setSharedValue(IS_AUTOMATICALLY_PAN_ENABLED, Val(enabled));
}

bool NotationConfiguration::isSmoothPanning() const
{
    return settings()->value(PLAYBACK_SMOOTH_PANNING).toBool();
}

void NotationConfiguration::setIsSmoothPanning(bool value)
{
    settings()->setSharedValue(PLAYBACK_SMOOTH_PANNING, Val(value));
}

bool NotationConfiguration::isPlayRepeatsEnabled() const
{
    return settings()->value(IS_PLAY_REPEATS_ENABLED).toBool();
}

void NotationConfiguration::setIsPlayRepeatsEnabled(bool enabled)
{
    settings()->setSharedValue(IS_PLAY_REPEATS_ENABLED, Val(enabled));
    m_isPlayRepeatsChanged.notify();
}

Notification NotationConfiguration::isPlayRepeatsChanged() const
{
    return m_isPlayRepeatsChanged;
}

bool NotationConfiguration::isPlayChordSymbolsEnabled() const
{
    return settings()->value(IS_PLAY_CHORD_SYMBOLS_ENABLED).toBool();
}

void NotationConfiguration::setIsPlayChordSymbolsEnabled(bool enabled)
{
    settings()->setSharedValue(IS_PLAY_CHORD_SYMBOLS_ENABLED, Val(enabled));
}

muse::async::Notification NotationConfiguration::isPlayChordSymbolsChanged() const
{
    return m_isPlayChordSymbolsChanged;
}

bool NotationConfiguration::isPlayPreviewNotesInInputByDuration() const
{
    return settings()->value(IS_PLAY_PREVIEW_NOTES_IN_INPUT_BY_DURATION_ENABLED).toBool();
}

void NotationConfiguration::setIsPlayPreviewNotesInInputByDuration(bool play)
{
    settings()->setSharedValue(IS_PLAY_PREVIEW_NOTES_IN_INPUT_BY_DURATION_ENABLED, Val(play));
}

muse::async::Notification NotationConfiguration::isPlayPreviewNotesInInputByDurationChanged() const
{
    return m_isPlayNotesPreviewInInputByDurationChanged;
}

bool NotationConfiguration::isMetronomeEnabled() const
{
    return settings()->value(IS_METRONOME_ENABLED).toBool();
}

void NotationConfiguration::setIsMetronomeEnabled(bool enabled)
{
    settings()->setSharedValue(IS_METRONOME_ENABLED, Val(enabled));
}

muse::async::Notification NotationConfiguration::isMetronomeEnabledChanged() const
{
    return m_isMetronomeEnabledChanged;
}

bool NotationConfiguration::isCountInEnabled() const
{
    return settings()->value(IS_COUNT_IN_ENABLED).toBool();
}

void NotationConfiguration::setIsCountInEnabled(bool enabled)
{
    settings()->setSharedValue(IS_COUNT_IN_ENABLED, Val(enabled));
}

double NotationConfiguration::guiScaling() const
{
    return uiConfiguration()->guiScaling();
}

double NotationConfiguration::notationScaling() const
{
    return uiConfiguration()->physicalDpi() / mu::engraving::DPI;
}

ValCh<muse::Orientation> NotationConfiguration::canvasOrientation() const
{
    ValCh<muse::Orientation> orientation;
    orientation.ch = m_canvasOrientationChanged;
    bool isVertical = settings()->value(IS_CANVAS_ORIENTATION_VERTICAL_KEY).toBool();
    orientation.val = isVertical ? muse::Orientation::Vertical : muse::Orientation::Horizontal;

    return orientation;
}

void NotationConfiguration::setCanvasOrientation(muse::Orientation orientation)
{
    bool isVertical = orientation == muse::Orientation::Vertical;
    mu::engraving::MScore::setVerticalOrientation(isVertical);

    settings()->setSharedValue(IS_CANVAS_ORIENTATION_VERTICAL_KEY, Val(isVertical));
}

bool NotationConfiguration::colorNotesOutsideOfUsablePitchRange() const
{
    return settings()->value(COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE).toBool();
}

void NotationConfiguration::setColorNotesOutsideOfUsablePitchRange(bool value)
{
    mu::engraving::MScore::warnPitchRange = value;
    settings()->setSharedValue(COLOR_NOTES_OUTSIDE_OF_USABLE_PITCH_RANGE, Val(value));
}

async::Channel<bool> NotationConfiguration::colorNotesOutsideOfUsablePitchRangeChanged() const
{
    return m_colorNotesOutsideOfUsablePitchRangeChanged;
}

bool NotationConfiguration::warnGuitarBends() const
{
    return settings()->value(WARN_GUITAR_BENDS).toBool();
}

void NotationConfiguration::setWarnGuitarBends(bool value)
{
    mu::engraving::MScore::warnGuitarBends = value;
    settings()->setSharedValue(WARN_GUITAR_BENDS, Val(value));
}

async::Channel<bool> NotationConfiguration::warnGuitarBendsChanged() const
{
    return m_warnGuitarBendsChanged;
}

int NotationConfiguration::delayBetweenNotesInRealTimeModeMilliseconds() const
{
    return settings()->value(REALTIME_DELAY).toInt();
}

void NotationConfiguration::setDelayBetweenNotesInRealTimeModeMilliseconds(int delayMs)
{
    settings()->setSharedValue(REALTIME_DELAY, Val(delayMs));
}

async::Channel<int> NotationConfiguration::delayBetweenNotesInRealTimeModeMillisecondsChanged() const
{
    return m_delayBetweenNotesInRealTimeModeMillisecondsChanged;
}

int NotationConfiguration::notePlayDurationMilliseconds() const
{
    return settings()->value(NOTE_DEFAULT_PLAY_DURATION).toInt();
}

void NotationConfiguration::setNotePlayDurationMilliseconds(int durationMs)
{
    mu::engraving::MScore::defaultPlayDuration = durationMs;
    settings()->setSharedValue(NOTE_DEFAULT_PLAY_DURATION, Val(durationMs));
}

async::Channel<int> NotationConfiguration::notePlayDurationMillisecondsChanged() const
{
    return m_notePlayDurationMillisecondsChanged;
}

void NotationConfiguration::setTemplateModeEnabled(std::optional<bool> enabled)
{
    mu::engraving::MScore::saveTemplateMode = enabled ? enabled.value() : false;
}

void NotationConfiguration::setTestModeEnabled(std::optional<bool> enabled)
{
    mu::engraving::MScore::testMode = enabled ? enabled.value() : false;
}

muse::io::path_t NotationConfiguration::instrumentListPath() const
{
    return globalConfiguration()->appDataPath() + "instruments/instruments.xml";
}

io::paths_t NotationConfiguration::scoreOrderListPaths() const
{
    io::paths_t paths;

    muse::io::path_t firstScoreOrderListPath = this->firstScoreOrderListPath();
    paths.push_back(firstScoreOrderListPath);

    muse::io::path_t secondScoreOrderListPath = this->secondScoreOrderListPath();
    if (!secondScoreOrderListPath.empty()) {
        paths.push_back(secondScoreOrderListPath);
    }

    return paths;
}

muse::async::Notification NotationConfiguration::scoreOrderListPathsChanged() const
{
    return m_scoreOrderListPathsChanged;
}

io::paths_t NotationConfiguration::userScoreOrderListPaths() const
{
    io::paths_t paths = {
        firstScoreOrderListPath(),
        secondScoreOrderListPath()
    };

    return paths;
}

void NotationConfiguration::setUserScoreOrderListPaths(const io::paths_t& paths)
{
    if (paths.empty()) {
        return;
    }

    setFirstScoreOrderListPath(paths[0]);
    if (paths.size() > 1) {
        setSecondScoreOrderListPath(paths[1]);
    }
}

muse::io::path_t NotationConfiguration::stringTuningsPresetsPath() const
{
    return globalConfiguration()->appDataPath() + "instruments/string_tunings_presets.json";
}

bool NotationConfiguration::isSnappedToGrid(muse::Orientation gridOrientation) const
{
    switch (gridOrientation) {
    case muse::Orientation::Horizontal: return settings()->value(IS_SNAPPED_TO_HORIZONTAL_GRID_KEY).toBool();
    case muse::Orientation::Vertical: return settings()->value(IS_SNAPPED_TO_VERTICAL_GRID_KEY).toBool();
    }

    return false;
}

void NotationConfiguration::setIsSnappedToGrid(muse::Orientation gridOrientation, bool isSnapped)
{
    switch (gridOrientation) {
    case muse::Orientation::Horizontal:
        settings()->setSharedValue(IS_SNAPPED_TO_HORIZONTAL_GRID_KEY, Val(isSnapped));
        break;
    case muse::Orientation::Vertical:
        settings()->setSharedValue(IS_SNAPPED_TO_VERTICAL_GRID_KEY, Val(isSnapped));
        break;
    }
}

int NotationConfiguration::gridSizeSpatium(muse::Orientation gridOrientation) const
{
    switch (gridOrientation) {
    case muse::Orientation::Horizontal: return settings()->value(HORIZONTAL_GRID_SIZE_KEY).toInt();
    case muse::Orientation::Vertical: return settings()->value(VERTICAL_GRID_SIZE_KEY).toInt();
    }

    return DEFAULT_GRID_SIZE_SPATIUM;
}

void NotationConfiguration::setGridSize(muse::Orientation gridOrientation, int sizeSpatium)
{
    switch (gridOrientation) {
    case muse::Orientation::Horizontal:
        mu::engraving::MScore::setHRaster(sizeSpatium);
        settings()->setSharedValue(HORIZONTAL_GRID_SIZE_KEY, Val(sizeSpatium));
        break;
    case muse::Orientation::Vertical:
        mu::engraving::MScore::setVRaster(sizeSpatium);
        settings()->setSharedValue(VERTICAL_GRID_SIZE_KEY, Val(sizeSpatium));
        break;
    }
}

bool NotationConfiguration::needToShowAddTextErrorMessage() const
{
    return settings()->value(NEED_TO_SHOW_ADD_TEXT_ERROR_MESSAGE_KEY).toBool();
}

void NotationConfiguration::setNeedToShowAddTextErrorMessage(bool show)
{
    settings()->setSharedValue(NEED_TO_SHOW_ADD_TEXT_ERROR_MESSAGE_KEY, Val(show));
}

bool NotationConfiguration::needToShowAddFiguredBassErrorMessage() const
{
    return settings()->value(NEED_TO_SHOW_ADD_FIGURED_BASS_ERROR_MESSAGE_KEY).toBool();
}

void NotationConfiguration::setNeedToShowAddFiguredBassErrorMessage(bool show)
{
    settings()->setSharedValue(NEED_TO_SHOW_ADD_FIGURED_BASS_ERROR_MESSAGE_KEY, Val(show));
}

bool NotationConfiguration::needToShowAddGuitarBendErrorMessage() const
{
    return settings()->value(NEED_TO_SHOW_ADD_GUITAR_BEND_ERROR_MESSAGE_KEY).toBool();
}

void NotationConfiguration::setNeedToShowAddGuitarBendErrorMessage(bool show)
{
    settings()->setSharedValue(NEED_TO_SHOW_ADD_GUITAR_BEND_ERROR_MESSAGE_KEY, Val(show));
}

bool NotationConfiguration::needToShowMScoreError(const std::string& errorKey) const
{
    Settings::Key key(module_name, "ui/dialogs/needToShowMScoreError/" + errorKey);

    settings()->setDefaultValue(key, Val(true));

    return settings()->value(key).toBool();
}

void NotationConfiguration::setNeedToShowMScoreError(const std::string& errorKey, bool show)
{
    Settings::Key key(module_name, "ui/dialogs/needToShowMScoreError/" + errorKey);
    settings()->setSharedValue(key, Val(show));
}

ValCh<int> NotationConfiguration::pianoKeyboardNumberOfKeys() const
{
    return m_pianoKeyboardNumberOfKeys;
}

bool NotationConfiguration::useNewPercussionPanel() const
{
    return settings()->value(USE_NEW_PERCUSSION_PANEL_KEY).toBool();
}

void NotationConfiguration::setUseNewPercussionPanel(bool use)
{
    settings()->setSharedValue(USE_NEW_PERCUSSION_PANEL_KEY, Val(use));
}

Notification NotationConfiguration::useNewPercussionPanelChanged() const
{
    return m_useNewPercussionPanelChanged;
}

bool NotationConfiguration::percussionPanelUseNotationPreview() const
{
    return settings()->value(PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY).toBool();
}

void NotationConfiguration::setPercussionPanelUseNotationPreview(bool use)
{
    settings()->setSharedValue(PERCUSSION_PANEL_USE_NOTATION_PREVIEW_KEY, Val(use));
}

Notification NotationConfiguration::percussionPanelUseNotationPreviewChanged() const
{
    return m_percussionPanelUseNotationPreviewChanged;
}

PercussionPanelAutoShowMode NotationConfiguration::percussionPanelAutoShowMode() const
{
    return settings()->value(PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY).toEnum<PercussionPanelAutoShowMode>();
}

void NotationConfiguration::setPercussionPanelAutoShowMode(PercussionPanelAutoShowMode autoShowMode)
{
    settings()->setSharedValue(PERCUSSION_PANEL_AUTO_SHOW_MODE_KEY, Val(autoShowMode));
}

Notification NotationConfiguration::percussionPanelAutoShowModeChanged() const
{
    return m_percussionPanelAutoShowModeChanged;
}

bool NotationConfiguration::autoClosePercussionPanel() const
{
    return settings()->value(AUTO_CLOSE_PERCUSSION_PANEL_KEY).toBool();
}

void NotationConfiguration::setAutoClosePercussionPanel(bool autoClose)
{
    settings()->setSharedValue(AUTO_CLOSE_PERCUSSION_PANEL_KEY, Val(autoClose));
}

Notification NotationConfiguration::autoClosePercussionPanelChanged() const
{
    return m_autoClosePercussionPanelChanged;
}

bool NotationConfiguration::showPercussionPanelPadSwapDialog() const
{
    return settings()->value(SHOW_PERCUSSION_PANEL_SWAP_DIALOG).toBool();
}

void NotationConfiguration::setShowPercussionPanelPadSwapDialog(bool show)
{
    settings()->setSharedValue(SHOW_PERCUSSION_PANEL_SWAP_DIALOG, Val(show));
}

Notification NotationConfiguration::showPercussionPanelPadSwapDialogChanged() const
{
    return m_showPercussionPanelPadSwapDialogChanged;
}

bool NotationConfiguration::percussionPanelMoveMidiNotesAndShortcuts() const
{
    return settings()->value(PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS).toBool();
}

void NotationConfiguration::setPercussionPanelMoveMidiNotesAndShortcuts(bool move)
{
    settings()->setSharedValue(PERCUSSION_PANEL_MOVE_MIDI_NOTES_AND_SHORTCUTS, Val(move));
}

Notification NotationConfiguration::percussionPanelMoveMidiNotesAndShortcutsChanged() const
{
    return m_percussionPanelMoveMidiNotesAndShortcutsChanged;
}

void NotationConfiguration::setPianoKeyboardNumberOfKeys(int number)
{
    settings()->setSharedValue(PIANO_KEYBOARD_NUMBER_OF_KEYS, Val(number));
}

ValCh<bool> NotationConfiguration::midiUseWrittenPitch() const
{
    return m_midiInputUseWrittenPitch;
}

void NotationConfiguration::setMidiUseWrittenPitch(bool useWrittenPitch)
{
    settings()->setSharedValue(USE_MIDI_INPUT_WRITTEN_PITCH, Val(useWrittenPitch));
}

muse::io::path_t NotationConfiguration::firstScoreOrderListPath() const
{
    return settings()->value(FIRST_SCORE_ORDER_LIST_KEY).toString();
}

void NotationConfiguration::setFirstScoreOrderListPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(FIRST_SCORE_ORDER_LIST_KEY, Val(path.toStdString()));
}

muse::io::path_t NotationConfiguration::secondScoreOrderListPath() const
{
    return settings()->value(SECOND_SCORE_ORDER_LIST_KEY).toString();
}

void NotationConfiguration::setSecondScoreOrderListPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(SECOND_SCORE_ORDER_LIST_KEY, Val(path.toStdString()));
}

muse::io::path_t NotationConfiguration::styleFileImportPath() const
{
    return settings()->value(STYLE_FILE_IMPORT_PATH_KEY).toString();
}

void NotationConfiguration::setStyleFileImportPath(const muse::io::path_t& path)
{
    settings()->setSharedValue(STYLE_FILE_IMPORT_PATH_KEY, Val(path.toStdString()));
}

async::Channel<std::string> NotationConfiguration::styleFileImportPathChanged() const
{
    return m_styleFileImportPathChanged;
}

int NotationConfiguration::styleDialogLastPageIndex() const
{
    return m_styleDialogLastPageIndex;
}

void NotationConfiguration::setStyleDialogLastPageIndex(int value)
{
    m_styleDialogLastPageIndex = value;
}

int NotationConfiguration::styleDialogLastSubPageIndex() const
{
    return m_styleDialogLastSubPageIndex;
}

void NotationConfiguration::setStyleDialogLastSubPageIndex(int value)
{
    m_styleDialogLastSubPageIndex = value;
}

void NotationConfiguration::resetStyleDialogPageIndices()
{
    setStyleDialogLastPageIndex(0);
    setStyleDialogLastSubPageIndex(0);
}
