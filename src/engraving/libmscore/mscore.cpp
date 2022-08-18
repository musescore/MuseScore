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

#include "mscore.h"

#include "figuredbass.h"
#include "hairpin.h"
#include "lyrics.h"
#include "ottava.h"
#include "score.h"
#include "spanner.h"
#include "stafftype.h"
#include "volta.h"

#include "config.h"

using namespace mu;

namespace mu::engraving {
bool MScore::debugMode = false;
bool MScore::testMode = false;
bool MScore::testWriteStyleToScore = true;

// #ifndef NDEBUG
bool MScore::noHorizontalStretch = false;
bool MScore::noVerticalStretch   = false;
bool MScore::useFallbackFont     = true;
// #endif

bool MScore::saveTemplateMode = false;
bool MScore::noGui = false;

int MScore::_vRaster;
int MScore::_hRaster;
bool MScore::_verticalOrientation = false;
double MScore::verticalPageGap = 5.0;
double MScore::horizontalPageGapEven = 1.0;
double MScore::horizontalPageGapOdd = 50.0;

bool MScore::warnPitchRange;
int MScore::pedalEventsMinTicks;

bool MScore::playRepeats;
int MScore::playbackSpeedIncrement;
double MScore::nudgeStep;
double MScore::nudgeStep10;
double MScore::nudgeStep50;
int MScore::defaultPlayDuration;

int MScore::sampleRate  = 44100;
int MScore::mtcType;

bool MScore::noExcerpts = false;
bool MScore::noImages = false;
bool MScore::pdfPrinting = false;
bool MScore::svgPrinting = false;

double MScore::pixelRatio  = 0.8;         // DPI / logicalDPI

extern void initDrumset();

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

    defaultPlayDuration = 300;        // ms
    warnPitchRange      = true;
    pedalEventsMinTicks = 1;
    playRepeats         = true;
    playbackSpeedIncrement = 5;

    //
    //  initialize styles
    //
    StaffType::initStaffTypes();
    initDrumset();
    FiguredBass::readConfigFile(String());

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
    qRegisterMetaType<TrillType>("TrillType");
    qRegisterMetaType<JumpType>("JumpType");
    qRegisterMetaType<MarkerType>("MarkerType");
    qRegisterMetaType<HairpinType>("HairpinType");
    qRegisterMetaType<Lyrics::Syllabic>("Syllabic");

#endif
}
}
