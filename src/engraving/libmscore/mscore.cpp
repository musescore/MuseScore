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

#include <QDir>
#include <QCoreApplication>

#include "config.h"
#include "translation.h"

#include "musescoreCore.h"
#include "style.h"
#include "mscore.h"
#include "sequencer.h"
#include "element.h"
#include "dynamic.h"
#include "accidental.h"
#include "figuredbass.h"
#include "stafftype.h"
#include "note.h"
#include "spanner.h"
#include "volta.h"
#include "ottava.h"
#include "trill.h"
#include "measurerepeat.h"
#include "jump.h"
#include "marker.h"
#include "layoutbreak.h"
#include "hairpin.h"
#include "glissando.h"
#include "page.h"
#include "slur.h"
#include "lyrics.h"
#include "accidental.h"
#include "notedot.h"
#include "tie.h"
#include "staff.h"
#include "beam.h"
#include "timesig.h"
#include "part.h"
#include "measure.h"
#include "score.h"
#include "keysig.h"
#include "harmony.h"
#include "stafftext.h"
#include "mscoreview.h"
#include "chord.h"
#include "hook.h"
#include "stem.h"
#include "stemslash.h"
#include "fraction.h"
#include "excerpt.h"
#include "spatium.h"
#include "barline.h"
#include "skyline.h"
#include "scorefont.h"

using namespace mu;

namespace Ms {
bool MScore::debugMode = false;
bool MScore::testMode = false;

// #ifndef NDEBUG
bool MScore::showSegmentShapes   = false;
bool MScore::showSkylines        = false;
bool MScore::showMeasureShapes   = false;
bool MScore::noHorizontalStretch = false;
bool MScore::noVerticalStretch   = false;
bool MScore::showBoundingRect    = false;
bool MScore::showSystemBoundingRect    = false;
bool MScore::showCorruptedMeasures = true;
bool MScore::useFallbackFont       = true;
// #endif

bool MScore::saveTemplateMode = false;
bool MScore::noGui = false;

MStyle* MScore::_defaultStyleForParts;

QString MScore::_globalShare;
int MScore::_vRaster;
int MScore::_hRaster;
bool MScore::_verticalOrientation = false;
qreal MScore::verticalPageGap = 5.0;
qreal MScore::horizontalPageGapEven = 1.0;
qreal MScore::horizontalPageGapOdd = 50.0;

QColor MScore::selectColor[VOICES];
QColor MScore::defaultColor;
QColor MScore::layoutBreakColor;
QColor MScore::frameMarginColor;
QColor MScore::bgColor;
QColor MScore::dropColor;
bool MScore::warnPitchRange;
int MScore::pedalEventsMinTicks;

bool MScore::harmonyPlayDisableCompatibility;
bool MScore::harmonyPlayDisableNew;
bool MScore::playRepeats;
bool MScore::panPlayback;
int MScore::playbackSpeedIncrement;
qreal MScore::nudgeStep;
qreal MScore::nudgeStep10;
qreal MScore::nudgeStep50;
int MScore::defaultPlayDuration;

QString MScore::lastError;
int MScore::division    = 480;     // 3840;   // pulses per quarter note (PPQ) // ticks per beat
int MScore::sampleRate  = 44100;
int MScore::mtcType;

bool MScore::noExcerpts = false;
bool MScore::noImages = false;
bool MScore::pdfPrinting = false;
bool MScore::svgPrinting = false;

double MScore::pixelRatio  = 0.8;         // DPI / logicalDPI

Sequencer* MScore::seq = 0;

extern void initDrumset();
extern QString mscoreGlobalShare;

std::vector<MScoreError> MScore::errorList {
    { MsError::MS_NO_ERROR,                     0,    0 },

    { MsError::NO_NOTE_SELECTED,                "s1", QT_TRANSLATE_NOOP("error", "No note selected:\nPlease select a note and retry") },
    { MsError::NO_CHORD_REST_SELECTED,          "s2", QT_TRANSLATE_NOOP("error",
                                                                        "No chord/rest selected:\nPlease select a chord or rest and retry") },
    { MsError::NO_LYRICS_SELECTED,              "s3", QT_TRANSLATE_NOOP("error",
                                                                        "No note or lyrics selected:\nPlease select a note or lyrics and retry") },
    { MsError::NO_NOTE_REST_SELECTED,           "s4", QT_TRANSLATE_NOOP("error",
                                                                        "No note or rest selected:\nPlease select a note or rest and retry") },
    { MsError::NO_FLIPPABLE_SELECTED,           "s5", QT_TRANSLATE_NOOP("error",
                                                                        "No flippable element selected:\nPlease select an element that can be flipped and retry") },
    { MsError::NO_STAFF_SELECTED,               "s6", QT_TRANSLATE_NOOP("error",
                                                                        "No staff selected:\nPlease select one or more staves and retry") },
    { MsError::NO_NOTE_FIGUREDBASS_SELECTED,    "s7", QT_TRANSLATE_NOOP("error",
                                                                        "No note or figured bass selected:\nPlease select a note or figured bass and retry") },

    { MsError::CANNOT_INSERT_TUPLET,            "t1", QT_TRANSLATE_NOOP("error", "Cannot insert chord/rest in tuplet") },
    { MsError::CANNOT_SPLIT_TUPLET,             "t2", QT_TRANSLATE_NOOP("error", "Cannot split tuplet") },
    { MsError::CANNOT_SPLIT_MEASURE_FIRST_BEAT, "m1", QT_TRANSLATE_NOOP("error", "Cannot split measure here:\n" "First beat of measure") },
    { MsError::CANNOT_SPLIT_MEASURE_TUPLET,     "m2", QT_TRANSLATE_NOOP("error", "Cannot split measure here:\n" "Cannot split tuplet") },
    { MsError::INSUFFICIENT_MEASURES,           "m3", QT_TRANSLATE_NOOP("error",
                                                                        "Measure repeat cannot be added here:\nInsufficient or unequal measures") },
    { MsError::CANNOT_SPLIT_MEASURE_REPEAT,     "m4", QT_TRANSLATE_NOOP("error", "Cannot split measure repeat") },

    { MsError::CANNOT_REMOVE_TIME_TUPLET,       "d1", QT_TRANSLATE_NOOP("error",
                                                                        "Cannot remove time from tuplet:\nPlease select the complete tuplet and retry") },
    { MsError::CANNOT_REMOVE_TIME_MEASURE_REPEAT, "d2", QT_TRANSLATE_NOOP("error",
                                                                          "Cannot remove time from measure repeat:\nPlease select the complete measure repeat and retry") },

    { MsError::NO_DEST,                         "p1", QT_TRANSLATE_NOOP("error", "No destination to paste") },
    { MsError::DEST_TUPLET,                     "p2", QT_TRANSLATE_NOOP("error", "Cannot paste into tuplet") },
    { MsError::TUPLET_CROSSES_BAR,              "p3", QT_TRANSLATE_NOOP("error", "Tuplet cannot cross barlines") },
    { MsError::DEST_LOCAL_TIME_SIGNATURE,       "p4", QT_TRANSLATE_NOOP("error", "Cannot paste in local time signature") },
    { MsError::DEST_TREMOLO,                    "p5", QT_TRANSLATE_NOOP("error", "Cannot paste in tremolo") },
    { MsError::NO_MIME,                         "p6", QT_TRANSLATE_NOOP("error", "Nothing to paste") },
    { MsError::DEST_NO_CR,                      "p7", QT_TRANSLATE_NOOP("error", "Destination is not a chord or rest") },
    { MsError::CANNOT_CHANGE_LOCAL_TIMESIG,     "l1",
      QT_TRANSLATE_NOOP("error", "Cannot change local time signature:\nMeasure is not empty") },
    { MsError::CORRUPTED_MEASURE,               "c1", QT_TRANSLATE_NOOP("error",
                                                                        "Cannot change time signature in front of a corrupted measure") },
};

MsError MScore::_error { MsError::MS_NO_ERROR };

//---------------------------------------------------------
//   Direction
//---------------------------------------------------------

Direction toDirection(const QString& s)
{
    Direction val;
    if (s == "up") {
        val = Direction::UP;
    } else if (s == "down") {
        val = Direction::DOWN;
    } else if (s == "auto") {
        val = Direction::AUTO;
    } else {
        val = Direction(s.toInt());
    }
    return val;
}

//---------------------------------------------------------
//   Direction::toString
//---------------------------------------------------------

const char* toString(Direction val)
{
    switch (val) {
    case Direction::AUTO: return "auto";
    case Direction::UP:   return "up";
    case Direction::DOWN: return "down";
    }
#if (!defined (_MSCVER) && !defined (_MSC_VER))
    __builtin_unreachable();
#else
    // The MSVC __assume() optimizer hint is similar, though not identical, to __builtin_unreachable()
    __assume(0);
#endif
}

//---------------------------------------------------------
//   Direction::toUserString
//---------------------------------------------------------

QString toUserString(Direction val)
{
    switch (val) {
    case Direction::AUTO: return qtrc("Direction", "Auto");
    case Direction::UP:   return qtrc("Direction", "Up");
    case Direction::DOWN: return qtrc("Direction", "Down");
    }
#if (!defined (_MSCVER) && !defined (_MSC_VER))
    __builtin_unreachable();
#else
    // The MSVC __assume() optimizer hint is similar, though not identical, to __builtin_unreachable()
    __assume(0);
#endif
}

//---------------------------------------------------------
//   doubleToSpatium
//---------------------------------------------------------

static Spatium doubleToSpatium(double d)
{
    return Spatium(d);
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MScore::init()
{
    static bool initDone = false;
    if (initDone) {
        return;
    }

#ifdef Q_OS_WIN
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
    _globalShare = dir.absolutePath() + "/";
#elif defined(Q_OS_IOS)
    {
        extern QString resourcePath();
        _globalShare = resourcePath();
    }

#elif defined(Q_OS_MAC)
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
    _globalShare = dir.absolutePath() + "/";
#else
    // Try relative path (needed for portable AppImage and non-standard installations)
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../share/" INSTALL_NAME));
    if (dir.exists()) {
        _globalShare = dir.absolutePath() + "/";
    } else { // Fall back to default location (e.g. if binary has moved relative to share)
        _globalShare = QString(INSTPREFIX "/share/" INSTALL_NAME);
    }
#endif

    defaultColor        = Qt::black;
    dropColor           = QColor("#1778db");
    defaultPlayDuration = 300;        // ms
    warnPitchRange      = true;
    pedalEventsMinTicks = 1;
    playRepeats         = true;
    panPlayback         = true;
    playbackSpeedIncrement = 5;

    lastError           = "";

    layoutBreakColor    = QColor("#A0A0A4");
    frameMarginColor    = QColor("#A0A0A4");
    bgColor.setNamedColor("#dddddd");

    //
    //  initialize styles
    //
    _baseStyle.precomputeValues();

    ScoreFont::initScoreFonts();
    StaffType::initStaffTypes();
    initDrumset();
    FiguredBass::readConfigFile(0);

#ifdef DEBUG_SHAPES
    testShapes();
#endif

    initDone = true;
}

void MScore::registerUiTypes()
{
    if (!QMetaType::registerConverter<Spatium, double>(&Spatium::toDouble)) {
        qFatal("registerConverter Spatium::toDouble failed");
    }
    if (!QMetaType::registerConverter<double, Spatium>(&doubleToSpatium)) {
        qFatal("registerConverter doubleToSpatium failed");
    }
//      if (!QMetaType::registerComparators<Spatium>())
//            qFatal("registerComparators for Spatium failed");

#ifdef SCRIPT_INTERFACE
    qRegisterMetaType<Note::ValueType>("ValueType");

    qRegisterMetaType<MScore::DirectionH>("DirectionH");
    qRegisterMetaType<Spanner::Anchor>("Anchor");
    qRegisterMetaType<NoteHead::Group>("NoteHeadGroup");
    qRegisterMetaType<NoteHead::Type>("NoteHeadType");
    qRegisterMetaType<SegmentType>("SegmentType");
    qRegisterMetaType<FiguredBassItem::Modifier>("Modifier");
    qRegisterMetaType<FiguredBassItem::Parenthesis>("Parenthesis");
    qRegisterMetaType<FiguredBassItem::ContLine>("ContLine");
    qRegisterMetaType<Volta::Type>("VoltaType");
    qRegisterMetaType<OttavaType>("OttavaType");
    qRegisterMetaType<Trill::Type>("TrillType");
    qRegisterMetaType<Dynamic::Range>("DynamicRange");
    qRegisterMetaType<Jump::Type>("JumpType");
    qRegisterMetaType<Marker::Type>("MarkerType");
    qRegisterMetaType<Beam::Mode>("BeamMode");
    qRegisterMetaType<HairpinType>("HairpinType");
    qRegisterMetaType<Lyrics::Syllabic>("Syllabic");
    qRegisterMetaType<LayoutBreak::Type>("LayoutBreakType");

    //classed enumerations
//      qRegisterMetaType<MSQE_StyledPropertyListIdx::E>("StyledPropertyListIdx");
//      qRegisterMetaType<MSQE_BarLineType::E>("BarLineType");
#endif
    qRegisterMetaType<Fraction>("Fraction");

    if (!QMetaType::registerConverter<Fraction, QString>(&Fraction::toString)) {
        qFatal("registerConverter Fraction::toString failed");
    }
}

//---------------------------------------------------------
//   readDefaultStyle
//---------------------------------------------------------

bool MScore::readDefaultStyle(QString file)
{
    if (file.isEmpty()) {
        return false;
    }
    MStyle style = defaultStyle();
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }
    bool rv = style.load(&f, true);
    if (rv) {
        setDefaultStyle(style);
    }
    f.close();
    return rv;
}

bool MScore::readPartStyle(QString filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    _defaultStyleForParts = new MStyle(_defaultStyle);
    bool rv = _defaultStyleForParts->load(&file, true);
    if (rv) {
        _defaultStyleForParts->precomputeValues();
    }
    file.close();
    return rv;
}

//---------------------------------------------------------
//   defaultStyleForPartsHasChanged
//---------------------------------------------------------

void MScore::defaultStyleForPartsHasChanged()
{
// TODO what is needed here?
//      delete _defaultStyleForParts;
//      _defaultStyleForParts = 0;
}

//---------------------------------------------------------
//   errorMessage
//---------------------------------------------------------

const char* MScore::errorMessage()
{
    for (MScoreError& e : errorList) {
        if (e.no == _error) {
            return e.txt;
        }
    }
    return QT_TRANSLATE_NOOP("error", "Unknown error");
}

//---------------------------------------------------------
//   errorGroup
//---------------------------------------------------------

const char* MScore::errorGroup()
{
    for (MScoreError& e : errorList) {
        if (e.no == _error) {
            return e.group;
        }
    }
    return "";
}
}
