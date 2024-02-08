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

#include "importmxmlnotepitch.h"

#include "draw/types/color.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/score.h"

#include "musicxmlsupport.h"
#include "importmxmllogger.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   accidental
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/accidental node.
 Return the result as an Accidental.
 */

// TODO: split in reading parameters versus creation

static Accidental* accidental(XmlStreamReader& e, Score* score)
{
    const bool cautionary = e.asciiAttribute("cautionary") == "yes";
    const bool editorial = e.asciiAttribute("editorial") == "yes";
    const bool parentheses = e.asciiAttribute("parentheses") == "yes";
    const bool brackets = e.asciiAttribute("bracket") == "yes";
    const Color accColor = Color(e.asciiAttribute("color").ascii());
    String smufl = e.attribute("smufl");

    const String s = e.readText();
    const AccidentalType type = mxmlString2accidentalType(s, smufl);

    if (type != AccidentalType::NONE) {
        auto a = Factory::createAccidental(score->dummy());
        a->setAccidentalType(type);
        if (cautionary || parentheses) {
            a->setBracket(AccidentalBracket(AccidentalBracket::PARENTHESIS));
            a->setRole(AccidentalRole::USER);
        } else if (editorial || brackets) {
            a->setBracket(AccidentalBracket(AccidentalBracket::BRACKET));
            a->setRole(AccidentalRole::USER);
        }
        if (accColor.isValid()) {
            a->setColor(accColor);
        }
        return a;
    }

    return 0;
}

//---------------------------------------------------------
//   displayStepOctave
//---------------------------------------------------------

/**
 Handle <display-step> and <display-octave> for <rest> and <unpitched>
 */

void MxmlNotePitch::displayStepOctave(XmlStreamReader& e)
{
    while (e.readNextStartElement()) {
        if (e.name() == "display-step") {
            const String step = e.readText();
            int pos = static_cast<int>(String(u"CDEFGAB").indexOf(step));
            if (step.size() == 1 && pos >= 0 && pos < 7) {
                m_displayStep = pos;
            } else {
                //logError(String("invalid step '%1'").arg(strStep));
                LOGD("invalid step '%s'", muPrintable(step));                // TODO
            }
        } else if (e.name() == "display-octave") {
            const String oct = e.readText();
            bool ok;
            m_displayOctave = oct.toInt(&ok);
            if (!ok || m_displayOctave < 0 || m_displayOctave > 9) {
                //logError(String("invalid octave '%1'").arg(strOct));
                LOGD("invalid octave '%s'", muPrintable(oct));               // TODO
                m_displayOctave = -1;
            }
        } else {
            e.skipCurrentElement();                         // TODO log
        }
    }
}

//---------------------------------------------------------
//   pitch
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/pitch node.
 */

void MxmlNotePitch::pitch(XmlStreamReader& e)
{
    // defaults
    m_step = -1;
    m_alter = 0;
    m_octave = -1;

    while (e.readNextStartElement()) {
        if (e.name() == "alter") {
            const String alter = e.readText();
            bool ok;
            m_alter = MxmlSupport::stringToInt(alter, &ok);             // fractions not supported by mscore
            if (!ok || m_alter < -2 || m_alter > 2) {
                m_logger->logError(String(u"invalid alter '%1'").arg(alter), &e);
                bool ok2;
                const auto altervalue = alter.toDouble(&ok2);
                if (ok2 && (std::abs(altervalue) < 2.0) && (m_accType == AccidentalType::NONE)) {
                    // try to see if a microtonal accidental is needed
                    m_accType = microtonalGuess(altervalue);
                }
                m_alter = 0;
            }
        } else if (e.name() == "octave") {
            const String oct = e.readText();
            bool ok;
            m_octave = oct.toInt(&ok);
            if (!ok || m_octave < 0 || m_octave > 9) {
                m_logger->logError(String(u"invalid octave '%1'").arg(oct), &e);
                m_octave = -1;
            }
        } else if (e.name() == "step") {
            const String step = e.readText();
            const size_t pos = String(u"CDEFGAB").indexOf(step);
            if (step.size() == 1 && pos < 7) {
                m_step = int(pos);
            } else {
                m_logger->logError(String(u"invalid step '%1'").arg(step), &e);
            }
        } else {
            // TODO skipLogCurrElem();
        }
    }
    //LOGD("pitch step %d alter %d oct %d accid %hhd", step, alter, oct, accid);
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

/**
 Handle selected child elements of the /score-partwise/part/measure/note node.
 Return true if handled.
 */

bool MxmlNotePitch::readProperties(XmlStreamReader& e, Score* score)
{
    const AsciiStringView tag(e.name());

    if (tag == "accidental") {
        m_acc = accidental(e, score);
        return true;
    } else if (tag == "unpitched") {
        m_unpitched = true;
        displayStepOctave(e);
        return true;
    } else if (tag == "pitch") {
        pitch(e);
        return true;
    }
    return false;
}
}
