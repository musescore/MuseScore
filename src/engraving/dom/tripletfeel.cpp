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

#include "tripletfeel.h"

#include "types/string.h"
#include "types/translatablestring.h"

#include "score.h"

namespace mu::engraving {
static const ElementStyle tripletFeelStyle {
    { Sid::subTitleFontSize, Pid::FONT_SIZE }, // TODO: add styles
    { Sid::tempoFontStyle, Pid::FONT_STYLE }
};

static const std::map<TripletFeelType, String> tupletSymbols =
{
    { TripletFeelType::TRIPLET_8TH,     String(u"%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textTupletBracketStartShortStem%2"
                                               "%1textTuplet3ShortStem%2"
                                               "%1note8thUp%2"
                                               "%1textTupletBracketEndShortStem%2") },

    { TripletFeelType::TRIPLET_16TH,    String(u"%1textBlackNoteShortStem%2"
                                               "%1textCont16thBeamShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textTuplet3ShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2") },

    { TripletFeelType::DOTTED_8TH,      String(u"%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textAugmentationDot%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2") },

    { TripletFeelType::DOTTED_16TH,     String(u"%1textBlackNoteShortStem%2"
                                               "%1textCont16thBeamShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2"
                                               " = "
                                               "%1textBlackNoteLongStem%2"
                                               "%1textCont16thBeamLongStem%2"
                                               "%1textAugmentationDot%2"
                                               "%1textCont16thBeamLongStem%2"
                                               "%1textBlackNoteFrac32ndLongStem%2") },

    { TripletFeelType::SCOTTISH_8TH,    String(u"%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textCont16thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               "%1textAugmentationDot%2") },

    { TripletFeelType::SCOTTISH_16TH,   String(u"%1textBlackNoteShortStem%2"
                                               "%1textCont16thBeamShortStem%2"
                                               "%1textBlackNoteFrac16thShortStem%2"
                                               " = "
                                               "%1textBlackNoteLongStem%2"
                                               "%1textCont32ndBeamLongStem%2"
                                               "%1textBlackNoteFrac16thLongStem%2"
                                               "%1textAugmentationDot%2") },

    { TripletFeelType::NONE,            String(u"%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2"
                                               " = "
                                               "%1textBlackNoteShortStem%2"
                                               "%1textCont8thBeamShortStem%2"
                                               "%1textBlackNoteFrac8thShortStem%2") }
};

static const std::map<TripletFeelType, TranslatableString> tripletFeelNames =
{
    { TripletFeelType::TRIPLET_8TH,     TranslatableString("engraving/tripletfeel", "Triplet 8th") },
    { TripletFeelType::TRIPLET_16TH,    TranslatableString("engraving/tripletfeel", "Triplet 16th") },
    { TripletFeelType::DOTTED_8TH,      TranslatableString("engraving/tripletfeel", "Dotted 8th") },
    { TripletFeelType::DOTTED_16TH,     TranslatableString("engraving/tripletfeel", "Dotted 16th") },
    { TripletFeelType::SCOTTISH_8TH,    TranslatableString("engraving/tripletfeel", "Scottish 8th") },
    { TripletFeelType::SCOTTISH_16TH,   TranslatableString("engraving/tripletfeel", "Scottish 16th") },
    { TripletFeelType::NONE,            TranslatableString("engraving/tripletfeel", "No triplet feel") }
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
        setSwingParameters(Constants::DIVISION / eightDivision, tripletRatio);
        break;

    case TripletFeelType::TRIPLET_16TH:
        setSwingParameters(Constants::DIVISION / sixteenthDivision, tripletRatio);
        break;

    case TripletFeelType::DOTTED_8TH:
        setSwingParameters(Constants::DIVISION / eightDivision, dottedRatio);
        break;

    case TripletFeelType::DOTTED_16TH:
        setSwingParameters(Constants::DIVISION / sixteenthDivision, dottedRatio);
        break;

    case TripletFeelType::SCOTTISH_8TH:
        setSwingParameters(Constants::DIVISION / eightDivision, scottishRatio);
        break;

    case TripletFeelType::SCOTTISH_16TH:
        setSwingParameters(Constants::DIVISION / sixteenthDivision, scottishRatio);
        break;

    case TripletFeelType::NONE:
        setSwingParameters(0, zeroRatio);
        break;
    }

    setXmlText(tupletSymbols.at(m_tripletFeelType).arg(String(u"<sym>staffPosLower3</sym><sym>"), String(u"</sym>")));
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue TripletFeel::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::FONT_SIZE:
        return style().styleV(Sid::subTitleFontSize);

    case Pid::FONT_STYLE:
        return style().styleV(Sid::tempoFontStyle);

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

TranslatableString TripletFeel::subtypeUserName() const
{
    return tripletFeelNames.at(m_tripletFeelType);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String TripletFeel::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedTypeUserName());
}
}
