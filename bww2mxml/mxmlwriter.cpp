//=============================================================================
//  BWW to MusicXML converter
//  Part of MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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
    if (mbf.irregular)
    {
      implicit = " implicit=\"yes\"";
      if (irregularMeasureNumber == 0)
        strMeasureNumber = "0";
      else
        strMeasureNumber = QString("X%1").arg(irregularMeasureNumber);
      ++irregularMeasureNumber;
    }
    else
    {
      ++regularMeasureNumber;
      strMeasureNumber.setNum(regularMeasureNumber);
    }
    out << "    <measure number=\"" << strMeasureNumber << "\"" << implicit << ">" << endl;
    if ((regularMeasureNumber + irregularMeasureNumber) != 1 && mbf.firstOfSystem)
    {
      out << "      <print new-system=\"yes\"/>" << endl;
    }
    if (mbf.repeatBegin || mbf.endingFirst || mbf.endingSecond)
    {
      out << "      <barline location=\"left\">" << endl;
      if (mbf.repeatBegin)
      {
        out << "        <bar-style>heavy-light</bar-style>" << endl;
        out << "        <repeat direction=\"forward\"/>" << endl;
      }
      if (mbf.endingFirst)
      {
        out << "        <ending number=\"1\" type=\"start\"/>" << endl;
        ending = 1;
      }
      if (mbf.endingSecond)
      {
        out << "        <ending number=\"2\" type=\"start\"/>" << endl;
        ending = 2;
      }
      out << "      </barline>" << endl;
    }
    if ((regularMeasureNumber + irregularMeasureNumber) == 1)
    {
      out << "      <attributes>" << endl;
      out << "        <divisions>" << wholeDur() / 4 << "</divisions>" << endl;
//      out << "        <key print-object=\"no\">" << endl;
      out << "        <key>" << endl;
      out << "          <fifths>2</fifths>" << endl;
      out << "          <mode>major</mode>" << endl;
      out << "        </key>" << endl;
      out << "        <time>" << endl;
      out << "          <beats>" << beats << "</beats>" << endl;
      out << "          <beat-type>" << beat << "</beat-type>" << endl;
      out << "        </time>" << endl;
      out << "        <clef>" << endl;
      out << "          <sign>G</sign>" << endl;
      out << "          <line>2</line>" << endl;
      out << "        </clef>" << endl;
      out << "      </attributes>" << endl;
      if (tempo)
      {
        out << "      <direction placement=\"above\">" << endl;
        out << "        <direction-type>" << endl;
        out << "          <metronome parentheses=\"no\">" << endl;
        out << "            <beat-unit>quarter</beat-unit>" << endl;
        out << "            <per-minute>" << tempo << "</per-minute>" << endl;
        out << "          </metronome>" << endl;
        out << "        </direction-type>" << endl;
        out << "        <sound tempo=\"" << tempo << "\"/>" << endl;
        out << "      </direction>" << endl;
      }
    }
  }

  /**
   End the current measure.
   */

  void MxmlWriter::endMeasure(const Bww::MeasureEndFlags mef)
  {
    // qDebug() << "MxmlWriter::endMeasure()";
    if (mef.repeatEnd || mef.endingEnd || mef.lastOfPart || mef.doubleBarLine)
    {
      out << "      <barline location=\"right\">" << endl;
      if (mef.repeatEnd || mef.lastOfPart)
      {
        out << "        <bar-style>light-heavy</bar-style>" << endl;
      }
      else if (mef.doubleBarLine)
      {
        out << "        <bar-style>light-light</bar-style>" << endl;
      }
      if (mef.endingEnd)
      {
        if (ending == 1)
          out << "        <ending number=\"1\" type=\"stop\"/>" << endl;
        else if (ending == 2)
          out << "        <ending number=\"2\" type=\"discontinue\"/>" << endl;
        else
        {
          // TODO: error message
        }
      }
      if (mef.repeatEnd)
      {
        out << "        <repeat direction=\"backward\"/>" << endl;
      }
      out << "      </barline>" << endl;
    }
    out << "    </measure>" << endl;
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
      || !typeMap.contains(type))
      {
      // TODO: error message
      return;
    }
    StepAlterOct sao = stepAlterOctMap.value(pitch);

    int dur = wholeDur() / type.toInt();
    if (dots == 1) dur = 3 * dur / 2;
    if (triplet != ST_NONE) dur = 2 * dur / 3;
    out << "      <note>" << endl;
    if (grace) out << "        <grace/>" << endl;
    out << "        <pitch>" << endl;
    out << "          <step>" << sao.s << "</step>" << endl;
    if (sao.a) out << "          <alter>" << sao.a << "</alter>" << endl;
    out << "          <octave>" << sao.o << "</octave>" << endl;
    out << "        </pitch>" << endl;
    if (!grace)
      out << "        <duration>" << dur << "</duration>" << endl;
    if (tieStart)
      out << "        <tie type=\"start\"/>" << endl;
    if (tieStop)
      out << "        <tie type=\"stop\"/>" << endl;
    out << "        <type>" << typeMap.value(type) << "</type>" << endl;
    if (dots == 1) out << "        <dot/>" << endl;
    if (triplet != ST_NONE)
    {
      out << "        <time-modification>" << endl;
      out << "          <actual-notes>3</actual-notes>" << endl;
      out << "          <normal-notes>2</normal-notes>" << endl;
      out << "        </time-modification>" << endl;
    }
    if (grace)
      out << "        <stem>up</stem>" << endl;
    else
    {
      if (type != "1")
        out << "        <stem>down</stem>" << endl;
    }
    for (int i = 0; i < maxBeamLevel; ++i)
    {
      QString s;
      switch (beamList.at(i))
      {
      case BM_BEGIN:         s = "begin"; break;
      case BM_CONTINUE:      s = "continue"; break;
      case BM_END:           s = "end"; break;
      case BM_FORWARD_HOOK:  s = "forward hook"; break;
      case BM_BACKWARD_HOOK: s = "backward hook"; break;
      default:               s = ""; break;
      }
      if (s != "")
        out << "        <beam number=\"" << i + 1 << "\">"
            << s << "</beam>" << endl;
      else
        break;
    }
    if (tieStart || tieStop || triplet == ST_START || triplet == ST_STOP)
    {
      out << "        <notations>" << endl;
      if (tieStart)
        out << "          <tied type=\"start\"/>" << endl;
      if (tieStop)
        out << "          <tied type=\"stop\"/>" << endl;
      if (triplet == ST_START)
      {
        if (type == "1" || type == "2" || type == "4")
          out << "          <tuplet type=\"start\" bracket=\"yes\"/>" << endl;
        else
          out << "          <tuplet type=\"start\" bracket=\"no\"/>" << endl;
      }
      if (triplet == ST_STOP)
        out << "          <tuplet type=\"stop\"/>" << endl;
      out << "        </notations>" << endl;
    }
    out << "      </note>" << endl;
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
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    out << "<!DOCTYPE score-partwise PUBLIC "
        << "\"-//Recordare//DTD MusicXML 2.0 Partwise//EN\" "
        << "\"http://www.musicxml.org/dtds/partwise.dtd\">" << endl;
    out << "<score-partwise>" << endl;
    out << "  <work>" << endl;
    out << "    <work-title>" << title << "</work-title>" << endl;
    // TODO work-number is not allowed, replace
    // out << "    <work-number>" << type << "</work-number>" << endl;
    out << "  </work>" << endl;
    out << "  <identification>" << endl;
    out << "    <creator type=\"composer\">" << composer << "</creator>" << endl;
    out << "    <rights>" << footer << "</rights>" << endl;
    out << "    <encoding>" << endl;
    out << "      <software>bww2mxml</software>" << endl;
    // TODO fill in real date
    // out << "      <encoding-date>TBD</encoding-date>" << endl;
    out << "    </encoding>" << endl;
    out << "  </identification>" << endl;
    out << "  <part-list>" << endl;
    out << "    <score-part id=\"P1\">" << endl;
    out << "      <part-name>" << instrumentName() << "</part-name>" << endl;
    out << "      <score-instrument id=\"P1-I1\">" << endl;
    out << "        <instrument-name>" << instrumentName() << "</instrument-name>" << endl;
    out << "      </score-instrument>" << endl;
    out << "      <midi-instrument id=\"P1-I1\">" << endl;
    out << "        <midi-channel>1</midi-channel>" << endl;
    out << "        <midi-program>" << midiProgram() << "</midi-program>" << endl;
    out << "      </midi-instrument>" << endl;
    out << "    </score-part>" << endl;
    out << "  </part-list>" << endl;
    out << "  <part id=\"P1\">" << endl;
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

    out << "  </part>" << endl;
    out << "</score-partwise>" << endl;
  }

} // namespace Bww
