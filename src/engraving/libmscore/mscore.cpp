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

#include "translation.h"
#include "style/style.h"

#include "musescoreCore.h"
#include "mscore.h"
#include "engravingitem.h"
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
#include "chord.h"
#include "hook.h"
#include "stem.h"
#include "stemslash.h"
#include "excerpt.h"
#include "barline.h"
#include "skyline.h"
#include "scorefont.h"

#include "config.h"

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

QString MScore::_globalShare;
int MScore::_vRaster;
int MScore::_hRaster;
bool MScore::_verticalOrientation = false;
qreal MScore::verticalPageGap = 5.0;
qreal MScore::horizontalPageGapEven = 1.0;
qreal MScore::horizontalPageGapOdd = 50.0;

bool MScore::warnPitchRange;
int MScore::pedalEventsMinTicks;

bool MScore::harmonyPlayDisableCompatibility;
bool MScore::harmonyPlayDisableNew;
bool MScore::playRepeats;
int MScore::playbackSpeedIncrement;
qreal MScore::nudgeStep;
qreal MScore::nudgeStep10;
qreal MScore::nudgeStep50;
int MScore::defaultPlayDuration;

QString MScore::lastError;
int MScore::sampleRate  = 44100;
int MScore::mtcType;

bool MScore::noExcerpts = false;
bool MScore::noImages = false;
bool MScore::pdfPrinting = false;
bool MScore::svgPrinting = false;

double MScore::pixelRatio  = 0.8;         // DPI / logicalDPI

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
                                                                        "Measure repeat cannot be added here:\n"
                                                                        "Insufficient or unequal measures") },
    { MsError::CANNOT_SPLIT_MEASURE_REPEAT,     "m4", QT_TRANSLATE_NOOP("error", "Cannot split measure repeat") },
    { MsError::CANNOT_SPLIT_MEASURE_TOO_SHORT,  "m5",
      QT_TRANSLATE_NOOP("error", "Cannot split measure here:\n" "Measure would be too short") },

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

    defaultPlayDuration = 300;        // ms
    warnPitchRange      = true;
    pedalEventsMinTicks = 1;
    playRepeats         = true;
    playbackSpeedIncrement = 5;

    lastError           = "";

    //
    //  initialize styles
    //

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
#ifdef SCRIPT_INTERFACE
    qRegisterMetaType<Spanner::Anchor>("Anchor");
    qRegisterMetaType<SegmentType>("SegmentType");
    qRegisterMetaType<FiguredBassItem::Modifier>("Modifier");
    qRegisterMetaType<FiguredBassItem::Parenthesis>("Parenthesis");
    qRegisterMetaType<FiguredBassItem::ContLine>("ContLine");
    qRegisterMetaType<Volta::Type>("VoltaType");
    qRegisterMetaType<OttavaType>("OttavaType");
    qRegisterMetaType<Trill::Type>("TrillType");
    qRegisterMetaType<Jump::Type>("JumpType");
    qRegisterMetaType<Marker::Type>("MarkerType");
    qRegisterMetaType<HairpinType>("HairpinType");
    qRegisterMetaType<Lyrics::Syllabic>("Syllabic");

#endif
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
