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

#include "engraving/types/fraction.h"
#include "engraving/dom/interval.h"
#include "engraving/types/typesconv.h"

#include "importmusicxmllogger.h"
#include "importmusicxmlnoteduration.h"
#include "importmusicxmlpass1.h"

using namespace mu::engraving;

namespace mu::iex::musicxml {
//---------------------------------------------------------
//   noteTypeToFraction
//---------------------------------------------------------

/**
 Convert MusicXML note type to fraction.
 */

static Fraction noteTypeToFraction(const String& type)
{
    if (type == u"1024th") {
        return Fraction(1, 1024);
    } else if (type == u"512th") {
        return Fraction(1, 512);
    } else if (type == u"256th") {
        return Fraction(1, 256);
    } else if (type == u"128th") {
        return Fraction(1, 128);
    } else if (type == u"64th") {
        return Fraction(1, 64);
    } else if (type == u"32nd") {
        return Fraction(1, 32);
    } else if (type == u"16th") {
        return Fraction(1, 16);
    } else if (type == u"eighth") {
        return Fraction(1, 8);
    } else if (type == u"quarter") {
        return Fraction(1, 4);
    } else if (type == u"half") {
        return Fraction(1, 2);
    } else if (type == u"whole") {
        return Fraction(1, 1);
    } else if (type == u"breve") {
        return Fraction(2, 1);
    } else if (type == u"long") {
        return Fraction(4, 1);
    } else if (type == u"maxima") {
        return Fraction(8, 1);
    } else {
        return Fraction(0, 0);
    }
}

//---------------------------------------------------------
//   calculateFraction
//---------------------------------------------------------

/**
 Convert note type, number of dots and actual and normal notes into a duration
 */

static Fraction calculateFraction(const String& type, const int dots, const Fraction timeMod)
{
    // type
    Fraction f = noteTypeToFraction(type);
    if (f.isValid()) {
        // dot(s)
        Fraction ff = f;
        for (int i = 0; i < dots; ++i) {
            ff = ff * Fraction(1, 2);
            f += ff;
        }
        // tuplet
        if (timeMod.isValid()) {
            f *= timeMod;
        }
        // clean up (just in case)
        f.reduce();
    }
    return f;
}

//---------------------------------------------------------
//   checkTiming
//---------------------------------------------------------

/**
 Do timing error checks.
 Return empty string if OK, message in case of error.
 */

String MusicXmlNoteDuration::checkTiming(const String& type, const bool rest, const bool grace)
{
    //LOGD("type %s rest %d grace %d", muPrintable(type), rest, grace);
    String errorStr;

    // normalize duration
    if (m_specDura.isValid()) {
        m_specDura.reduce();
    }

    // default: use calculated duration
    m_calcDura = calculateFraction(type, m_dots, m_timeMod);
    if (m_calcDura.isValid()) {
        m_dura = m_calcDura;
    }

    if (m_specDura.isValid() && m_calcDura.isValid()) {
        if (m_specDura != m_calcDura) {
            errorStr = String(u"calculated duration (%1) not equal to specified duration (%2)")
                       .arg(m_calcDura.toString(), m_specDura.toString());
            //LOGD("rest %d type '%s' timemod %s", rest, muPrintable(type), muPrintable(_timeMod.print()));

            if (rest && type == "whole" && m_specDura.isValid()) {
                // Sibelius whole measure rest (not an error)
                errorStr = u"";
                m_dura = m_specDura;
            } else if (grace && m_specDura == Fraction(0, 1)) {
                // grace note (not an error)
                errorStr = u"";
                m_dura = m_specDura;
            } else {
                if (std::abs(m_calcDura.ticks() - m_specDura.ticks()) <= m_pass1->maxDiff()) {
                    errorStr += u" -> assuming rounding error";
                    m_pass1->insertAdjustedDuration(m_dura, m_calcDura);
                    m_dura = m_calcDura;
                    m_specDura = m_calcDura; // prevent changing off time
                } else {
                    errorStr += u" -> using calculated duration";
                }
            }

            // Special case:
            // Encore generates rests in tuplets w/o <tuplet> or <time-modification>.
            // Detect this by comparing the actual duration with the expected duration
            // based on note type. If actual is 2/3 of expected, the rest is part
            // of a tuplet.
            if (rest) {
                if (2 * m_calcDura.ticks() == 3 * m_specDura.ticks()) {
                    m_timeMod = Fraction(2, 3);
                    errorStr += errorStr.isEmpty() ? u" ->" : u",";
                    errorStr += u" assuming triplet";
                    m_dura = m_specDura;
                }
            }
        }
    } else if (m_specDura.isValid() && !m_calcDura.isValid()) {
        // do not report an error for typeless (whole measure) rests
        if (!(rest && type == "")) {
            errorStr = u"calculated duration invalid, using specified duration";
        }
        m_dura = m_specDura;
    } else if (!m_specDura.isValid() && m_calcDura.isValid()) {
        if (!grace) {
            errorStr = u"specified duration invalid, using calculated duration";
        }
    } else {
        errorStr = u"calculated and specified duration invalid, using 4/4";
        m_dura = Fraction(4, 4);
    }

    m_pass1->insertSeenDenominator(m_dura.reduced().denominator());
    return errorStr;
}

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/duration node.
 */

void MusicXmlNoteDuration::duration(muse::XmlStreamReader& e)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::duration", &e);

    m_specDura.set(0, 0);          // invalid unless set correctly
    int intDura = e.readAsciiText().toInt();
    m_specDura = m_pass1->calcTicks(intDura, m_divs, &e); // Duration reading (and rounding) code consolidated to pass1
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

/**
 Handle selected child elements of the /score-partwise/part/measure/note/duration node.
 Return true if handled.
 */

bool MusicXmlNoteDuration::readProperties(muse::XmlStreamReader& e)
{
    const AsciiStringView tag(e.name());
    //LOGD("tag %s", muPrintable(tag.toString()));
    if (tag == "dot") {
        m_dots++;
        e.skipCurrentElement();  // skip but don't log
        return true;
    } else if (tag == "duration") {
        duration(e);
        return true;
    } else if (tag == "time-modification") {
        timeModification(e);
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   timeModification
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/time-modification node.
 */

void MusicXmlNoteDuration::timeModification(muse::XmlStreamReader& e)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::timeModification", &e);

    int intActual = 0;
    int intNormal = 0;
    String strActual;
    String strNormal;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "actual-notes") {
            strActual = e.readText();
        } else if (tag == "normal-notes") {
            strNormal = e.readText();
        } else if (tag == "normal-type") {
            // "measure" is not a valid normal-type,
            // but would be accepted by setType()
            String strNormalType = e.readText();
            if (strNormalType != u"measure") {
                muse::ByteArray ba = strNormalType.toAscii();
                m_normalType.setType(TConv::fromXml(ba.constChar(), DurationType::V_INVALID));
            }
        } else {
            m_logger->logDebugInfo(String(u"skipping '%1'").arg(String::fromAscii(e.name().ascii())), &e);
            e.skipCurrentElement();
        }
    }

    intActual = strActual.toInt();
    intNormal = strNormal.toInt();
    if (intActual > 0 && intNormal > 0) {
        m_timeMod.set(intNormal, intActual);
    } else {
        m_timeMod.set(1, 1);
        m_logger->logError(String(u"illegal time-modification: actual-notes %1 normal-notes %2")
                           .arg(strActual, strNormal), &e);
    }
}
}
