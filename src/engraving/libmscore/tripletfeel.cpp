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

#include "tripletfeel.h"

#include "score.h"
#include "string.h"

namespace mu::engraving {
static const ElementStyle tripletFeelStyle {
    { Sid::subTitleFontSize, Pid::FONT_SIZE }, // TODO: add styles
    { Sid::tempoFontStyle, Pid::FONT_STYLE }
};

static std::map<TripletFeelType, String> tupletSymbols =
{
    { TripletFeelType::TRIPLET_8TH,     String("%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textTupletBracketStartShortStem%2"
                                               "%1textTuplet3ShortStem%2"
                                               "%1note8thUp%2"
                                               "%1textTupletBracketEndShortStem%2") },

    { TripletFeelType::TRIPLET_16TH,    String("%1textBlackNoteShortStem%2"
                                               "%1textCont16thBeamShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textTuplet3ShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2") },

    { TripletFeelType::DOTTED_8TH,      String("%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textAugmentationDot%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2") },

    { TripletFeelType::DOTTED_16TH,     String("%1textBlackNoteShortStem%2"
                                               "%1textCont16thBeamShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2"
                                               " = "
                                               "%1textBlackNoteLongStem%2"
                                               "%1textCont16thBeamLongStem%2"
                                               "%1textAugmentationDot%2"
                                               "%1textCont16thBeamLongStem%2"
                                               "%1textBlackNoteFrac32ndLongStem%2") },

    { TripletFeelType::SCOTTISH_8TH,    String("%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textCont16thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               "%1textAugmentationDot%2") },

    { TripletFeelType::SCOTTISH_16TH,   String("%1textBlackNoteShortStem%2"
                                               "%1textCont16thBeamShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2"
                                               " = "
                                               "%1textBlackNoteLongStem%2"
                                               "%1textCont32ndBeamLongStem%2"
                                               "%1textBlackNoteFrac16thLongStem%2"
                                               "%1textAugmentationDot%2") },

    { TripletFeelType::NONE,            String("%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2") }
};

// TODO: move to s_symNames
static std::map<TripletFeelType, String> tupletNames =
{
    { TripletFeelType::TRIPLET_8TH,     String("Triplet 8th") },
    { TripletFeelType::TRIPLET_16TH,    String("Triplet 16th") },
    { TripletFeelType::DOTTED_8TH,      String("Dotted 8th") },
    { TripletFeelType::DOTTED_16TH,     String("Dotted 16th") },
    { TripletFeelType::SCOTTISH_8TH,    String("Scottish 8th") },
    { TripletFeelType::SCOTTISH_16TH,   String("Scottish 16th") },
    { TripletFeelType::NONE,            String("No Triplet Feel") }
};

TripletFeel::TripletFeel(Segment* parent, TripletFeelType tripletFillType)
    : SystemText(parent, TextStyleType::SYSTEM, ElementType::TRIPLET_FEEL)
{
    m_tripletFeelType = tripletFillType;
    setTripletProperty();
    initElementStyle(&tripletFeelStyle);
}

void TripletFeel::setTripletProperty()
{
    setSwing(true);

    switch (m_tripletFeelType) {
    case TripletFeelType::TRIPLET_8TH:
        setSwingParameters(Constants::division / eightDivision, tripletRatio);
        break;

    case TripletFeelType::TRIPLET_16TH:
        setSwingParameters(Constants::division / sixteenthDivision, tripletRatio);
        break;

    case TripletFeelType::DOTTED_8TH:
        setSwingParameters(Constants::division / eightDivision, dottedRatio);
        break;

    case TripletFeelType::DOTTED_16TH:
        setSwingParameters(Constants::division / sixteenthDivision, dottedRatio);
        break;

    case TripletFeelType::SCOTTISH_8TH:
        setSwingParameters(Constants::division / eightDivision, scottishRatio);
        break;

    case TripletFeelType::SCOTTISH_16TH:
        setSwingParameters(Constants::division / sixteenthDivision, scottishRatio);
        break;

    case TripletFeelType::NONE:
        setSwing(false);
        setSwingParameters(0, zeroRatio);
        break;
    }

    setXmlText(tupletSymbols[m_tripletFeelType].arg(String("<sym>staffPosLower3</sym><sym>"), String("</sym>")));
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue TripletFeel::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::FONT_SIZE:
        return score()->styleV(Sid::subTitleFontSize);

    case Pid::FONT_STYLE:
        return score()->styleV(Sid::tempoFontStyle);

    default:
        return SystemText::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid TripletFeel::getPropertyStyle(Pid id) const
{
    switch (id) {
    case Pid::FONT_SIZE:
        return Sid::subTitleFontSize;

    case Pid::FONT_STYLE:
        return Sid::tempoFontStyle;
    default:
        break;
    }

    return SystemText::getPropertyStyle(id);
}

//---------------------------------------------------------
//   typeUserName
//---------------------------------------------------------

String TripletFeel::typeUserName() const
{
    // TODO: SymNames::translatedUserNameForSymId(symId());
    return tupletNames.at(m_tripletFeelType);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String TripletFeel::accessibleInfo() const
{
    return String("%1: %2").arg(EngravingItem::accessibleInfo(), typeUserName());
}
}
