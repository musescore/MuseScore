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

/**
\file
 Output writer for BWW to MusicXML converter.
 */

#include <QtCore/QtDebug>

#include "mxmlwriter.h"

namespace Bww {
/**
   MxmlWriter constructor.
   */

MxmlWriter::MxmlWriter()
    : beats(4),
    beat(4),
    regularMeasureNumber(0),
    irregularMeasureNumber(0),
    tempo(0),
    ending(0)
{
    qDebug() << "MxmlWriter::MxmlWriter()";

    stepAlterOctMap["LG"] = StepAlterOct('G', 0, 4);
    stepAlterOctMap["LA"] = StepAlterOct('A', 0, 4);
    stepAlterOctMap["B"] = StepAlterOct('B', 0, 4);
    stepAlterOctMap["C"] = StepAlterOct('C', 1, 5);
    stepAlterOctMap["D"] = StepAlterOct('D', 0, 5);
    stepAlterOctMap["E"] = StepAlterOct('E', 0, 5);
    stepAlterOctMap["F"] = StepAlterOct('F', 1, 5);
    stepAlterOctMap["HG"] = StepAlterOct('G', 0, 5);
    stepAlterOctMap["HA"] = StepAlterOct('A', 0, 5);

    typeMap["1"] = "whole";
    typeMap["2"] = "half";
    typeMap["4"] = "quarter";
    typeMap["8"] = "eighth";
    typeMap["16"] = "16th";
    typeMap["32"] = "32nd";
}

/**
   Begin a new measure.
   */

void MxmlWriter::beginMeasure(const Bww::MeasureBeginFlags mbf)
{
    // qDebug() << "MxmlWriter::beginMeasure()";
    QString implicit;
    QString strMeasureNumber;
    if (mbf.irregular) {
        implicit = " implicit=\"yes\"";
        if (irregularMeasureNumber == 0) {
            strMeasureNumber = "0";
        } else {
            strMeasureNumber = QString("X%1").arg(irregularMeasureNumber);
        }
        ++irregularMeasureNumber;
    } else {
        ++regularMeasureNumber;
        strMeasureNumber.setNum(regularMeasureNumber);
    }
    out << "    <measure number=\"" << strMeasureNumber << "\"" << implicit << ">" << Qt::endl;
    if ((regularMeasureNumber + irregularMeasureNumber) != 1 && mbf.firstOfSystem) {
        out << "      <print new-system=\"yes\"/>" << Qt::endl;
    }
    if (mbf.repeatBegin || mbf.endingFirst || mbf.endingSecond) {
        out << "      <barline location=\"left\">" << Qt::endl;
        if (mbf.repeatBegin) {
            out << "        <bar-style>heavy-light</bar-style>" << Qt::endl;
            out << "        <repeat direction=\"forward\"/>" << Qt::endl;
        }
        if (mbf.endingFirst) {
            out << "        <ending number=\"1\" type=\"start\"/>" << Qt::endl;
            ending = 1;
        }
        if (mbf.endingSecond) {
            out << "        <ending number=\"2\" type=\"start\"/>" << Qt::endl;
            ending = 2;
        }
        out << "      </barline>" << Qt::endl;
    }
    if ((regularMeasureNumber + irregularMeasureNumber) == 1) {
        out << "      <attributes>" << Qt::endl;
        out << "        <divisions>" << wholeDur() / 4 << "</divisions>" << Qt::endl;
//      out << "        <key print-object=\"no\">" << Qt::endl;
        out << "        <key>" << Qt::endl;
        out << "          <fifths>2</fifths>" << Qt::endl;
        out << "          <mode>major</mode>" << Qt::endl;
        out << "        </key>" << Qt::endl;
        out << "        <time>" << Qt::endl;
        out << "          <beats>" << beats << "</beats>" << Qt::endl;
        out << "          <beat-type>" << beat << "</beat-type>" << Qt::endl;
        out << "        </time>" << Qt::endl;
        out << "        <clef>" << Qt::endl;
        out << "          <sign>G</sign>" << Qt::endl;
        out << "          <line>2</line>" << Qt::endl;
        out << "        </clef>" << Qt::endl;
        out << "      </attributes>" << Qt::endl;
        if (tempo) {
            out << "      <direction placement=\"above\">" << Qt::endl;
            out << "        <direction-type>" << Qt::endl;
            out << "          <metronome parentheses=\"no\">" << Qt::endl;
            out << "            <beat-unit>quarter</beat-unit>" << Qt::endl;
            out << "            <per-minute>" << tempo << "</per-minute>" << Qt::endl;
            out << "          </metronome>" << Qt::endl;
            out << "        </direction-type>" << Qt::endl;
            out << "        <sound tempo=\"" << tempo << "\"/>" << Qt::endl;
            out << "      </direction>" << Qt::endl;
        }
    }
}

/**
   End the current measure.
   */

void MxmlWriter::endMeasure(const Bww::MeasureEndFlags mef)
{
    // qDebug() << "MxmlWriter::endMeasure()";
    if (mef.repeatEnd || mef.endingEnd || mef.lastOfPart || mef.doubleBarLine) {
        out << "      <barline location=\"right\">" << Qt::endl;
        if (mef.repeatEnd || mef.lastOfPart) {
            out << "        <bar-style>light-heavy</bar-style>" << Qt::endl;
        } else if (mef.doubleBarLine) {
            out << "        <bar-style>light-light</bar-style>" << Qt::endl;
        }
        if (mef.endingEnd) {
            if (ending == 1) {
                out << "        <ending number=\"1\" type=\"stop\"/>" << Qt::endl;
            } else if (ending == 2) {
                out << "        <ending number=\"2\" type=\"discontinue\"/>" << Qt::endl;
            } else {
                // TODO: error message
            }
        }
        if (mef.repeatEnd) {
            out << "        <repeat direction=\"backward\"/>" << Qt::endl;
        }
        out << "      </barline>" << Qt::endl;
    }
    out << "    </measure>" << Qt::endl;
}

/**
   Write a single note.
   */

void MxmlWriter::note(const QString pitch, QVector<BeamType> beamList,
                      const QString type, const int dots,
                      bool tieStart, bool tieStop,
                      StartStop triplet,
                      bool grace)
{
    // qDebug() << "MxmlWriter::note()";

    if (!stepAlterOctMap.contains(pitch)
        || !typeMap.contains(type)) {
        // TODO: error message
        return;
    }
    StepAlterOct sao = stepAlterOctMap.value(pitch);

    int dur = wholeDur() / type.toInt();
    if (dots == 1) {
        dur = 3 * dur / 2;
    }
    if (triplet != StartStop::ST_NONE) {
        dur = 2 * dur / 3;
    }
    out << "      <note>" << Qt::endl;
    if (grace) {
        out << "        <grace/>" << Qt::endl;
    }
    out << "        <pitch>" << Qt::endl;
    out << "          <step>" << sao.s << "</step>" << Qt::endl;
    if (sao.a) {
        out << "          <alter>" << sao.a << "</alter>" << Qt::endl;
    }
    out << "          <octave>" << sao.o << "</octave>" << Qt::endl;
    out << "        </pitch>" << Qt::endl;
    if (!grace) {
        out << "        <duration>" << dur << "</duration>" << Qt::endl;
    }
    if (tieStart) {
        out << "        <tie type=\"start\"/>" << Qt::endl;
    }
    if (tieStop) {
        out << "        <tie type=\"stop\"/>" << Qt::endl;
    }
    out << "        <type>" << typeMap.value(type) << "</type>" << Qt::endl;
    if (dots == 1) {
        out << "        <dot/>" << Qt::endl;
    }
    if (triplet != StartStop::ST_NONE) {
        out << "        <time-modification>" << Qt::endl;
        out << "          <actual-notes>3</actual-notes>" << Qt::endl;
        out << "          <normal-notes>2</normal-notes>" << Qt::endl;
        out << "        </time-modification>" << Qt::endl;
    }
    if (grace) {
        out << "        <stem>up</stem>" << Qt::endl;
    } else {
        if (type != "1") {
            out << "        <stem>down</stem>" << Qt::endl;
        }
    }
    for (int i = 0; i < maxBeamLevel; ++i) {
        QString s;
        switch (beamList.at(i)) {
        case BeamType::BM_BEGIN:         s = "begin";
            break;
        case BeamType::BM_CONTINUE:      s = "continue";
            break;
        case BeamType::BM_END:           s = "end";
            break;
        case BeamType::BM_FORWARD_HOOK:  s = "forward hook";
            break;
        case BeamType::BM_BACKWARD_HOOK: s = "backward hook";
            break;
        default:               s = "";
            break;
        }
        if (!s.empty()) {
            out << "        <beam number=\"" << i + 1 << "\">"
                << s << "</beam>" << Qt::endl;
        } else {
            break;
        }
    }
    if (tieStart || tieStop || triplet == StartStop::ST_START || triplet == StartStop::ST_STOP) {
        out << "        <notations>" << Qt::endl;
        if (tieStart) {
            out << "          <tied type=\"start\"/>" << Qt::endl;
        }
        if (tieStop) {
            out << "          <tied type=\"stop\"/>" << Qt::endl;
        }
        if (triplet == StartStop::ST_START) {
            if (type == "1" || type == "2" || type == "4") {
                out << "          <tuplet type=\"start\" bracket=\"yes\"/>" << Qt::endl;
            } else {
                out << "          <tuplet type=\"start\" bracket=\"no\"/>" << Qt::endl;
            }
        }
        if (triplet == StartStop::ST_STOP) {
            out << "          <tuplet type=\"stop\"/>" << Qt::endl;
        }
        out << "        </notations>" << Qt::endl;
    }
    out << "      </note>" << Qt::endl;
}

/**
   Write the header.
   */

void MxmlWriter::header(const QString title, const QString /* type */,
                        const QString composer, const QString footer,
                        const unsigned int temp)
{
    // qDebug() << "MxmlWriter::header()"
    //    << "title:" << title
    //    << "type:" << type
    //    << "composer:" << composer
    //    << "footer:" << footer
    //    << "temp:" << temp
    //    ;

    // save tempo for later use
    tempo = temp;

    // write the header
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << Qt::endl;
    out << "<!DOCTYPE score-partwise PUBLIC "
        << "\"-//Recordare//DTD MusicXML 2.0 Partwise//EN\" "
        << "\"http://www.musicxml.org/dtds/partwise.dtd\">" << Qt::endl;
    out << "<score-partwise>" << Qt::endl;
    out << "  <work>" << Qt::endl;
    out << "    <work-title>" << title << "</work-title>" << Qt::endl;
    // TODO work-number is not allowed, replace
    // out << "    <work-number>" << type << "</work-number>" << Qt::endl;
    out << "  </work>" << Qt::endl;
    out << "  <identification>" << Qt::endl;
    out << "    <creator type=\"composer\">" << composer << "</creator>" << Qt::endl;
    out << "    <rights>" << footer << "</rights>" << Qt::endl;
    out << "    <encoding>" << Qt::endl;
    out << "      <software>bww2mxml</software>" << Qt::endl;
    // TODO fill in real date
    // out << "      <encoding-date>TBD</encoding-date>" << Qt::endl;
    out << "    </encoding>" << Qt::endl;
    out << "  </identification>" << Qt::endl;
    out << "  <part-list>" << Qt::endl;
    out << "    <score-part id=\"P1\">" << Qt::endl;
    out << "      <part-name>" << instrumentName() << "</part-name>" << Qt::endl;
    out << "      <score-instrument id=\"P1-I1\">" << Qt::endl;
    out << "        <instrument-name>" << instrumentName() << "</instrument-name>" << Qt::endl;
    out << "      </score-instrument>" << Qt::endl;
    out << "      <midi-instrument id=\"P1-I1\">" << Qt::endl;
    out << "        <midi-channel>1</midi-channel>" << Qt::endl;
    out << "        <midi-program>" << midiProgram() << "</midi-program>" << Qt::endl;
    out << "      </midi-instrument>" << Qt::endl;
    out << "    </score-part>" << Qt::endl;
    out << "  </part-list>" << Qt::endl;
    out << "  <part id=\"P1\">" << Qt::endl;
}

/**
   Store beats and beat type for later use.
   */

void MxmlWriter::tsig(const int bts, const int bt)
{
    // qDebug() << "MxmlWriter::tsig()"
    //    << "beats:" << bts
    //    << "beat:" << bt
    //   ;

    beats = bts;
    beat  = bt;
}

/**
   Write the trailer.
   */

void MxmlWriter::trailer()
{
    // qDebug() << "MxmlWriter::trailer()"
    //    ;

    out << "  </part>" << Qt::endl;
    out << "</score-partwise>" << Qt::endl;
}
} // namespace Bww
