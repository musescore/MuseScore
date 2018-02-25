//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2015 Werner Schweer and others
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

#ifndef __IMPORTMXMLPASS1_H__
#define __IMPORTMXMLPASS1_H__

#include "libmscore/score.h"
#include "importxmlfirstpass.h"
#include "musicxml.h" // for the creditwords and MusicXmlPartGroupList definitions
#include "musicxmlsupport.h"

namespace Ms {

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

struct PageFormat {
      QSizeF size;
      qreal printableWidth;        // _width - left margin - right margin
      qreal evenLeftMargin;        // values in inch
      qreal oddLeftMargin;
      qreal evenTopMargin;
      qreal evenBottomMargin;
      qreal oddTopMargin;
      qreal oddBottomMargin;
      bool twosided;
      };

typedef QMap<QString, Part*> PartMap;
typedef std::map<int,MusicXmlPartGroup*> MusicXmlPartGroupMap;

//---------------------------------------------------------
//   MxmlOctaveShiftDesc
//---------------------------------------------------------

struct MxmlOctaveShiftDesc {
      enum class Type : char { UP, DOWN, STOP, NONE };
      Type tp;
      short size;
      Fraction time;
      short num;
      MxmlOctaveShiftDesc() : tp(Type::NONE), size(0), num(-1) {}
      MxmlOctaveShiftDesc(Type _tp, short _size, Fraction _tm) : tp(_tp), size(_size), time(_tm), num(-1) {}
      };

//---------------------------------------------------------
//   MusicXMLParserPass1
//---------------------------------------------------------

class MxmlLogger;

class MusicXMLParserPass1 {
public:
      MusicXMLParserPass1(Score* score, MxmlLogger* logger);
      void initPartState(const QString& partId);
      Score::FileError parse(QIODevice* device);
      Score::FileError parse();
      void scorePartwise();
      void identification();
      void credit(CreditWordsList& credits);
      void defaults(int& pageWidth, int& pageHeight);
      void pageLayout(PageFormat& pf, const qreal conversion, int& pageWidth, int& pageHeight);
      void partList(MusicXmlPartGroupList& partGroupList);
      void partGroup(const int scoreParts, MusicXmlPartGroupList& partGroupList, MusicXmlPartGroupMap& partGroups);
      void scorePart();
      void scoreInstrument(const QString& partId);
      void midiInstrument(const QString& partId);
      void part();
      void measure(const QString& partId, const Fraction cTime, Fraction& mdur, VoiceOverlapDetector& vod);
      void attributes(const QString& partId, const Fraction cTime);
      void clef(const QString& partId);
      void time(const Fraction cTime);
      void divisions();
      void staves(const QString& partId);
      void direction(const QString& partId, const Fraction cTime);
      void directionType(const Fraction cTime, QList<MxmlOctaveShiftDesc>& starts, QList<MxmlOctaveShiftDesc>& stops);
      void handleOctaveShift(const Fraction cTime, const QString& type, short size, MxmlOctaveShiftDesc& desc);
      void note(const QString& partId, const Fraction cTime, Fraction& dura, VoiceOverlapDetector& vod);
      void notePrintSpacingNo(Fraction& dura);
      void duration(Fraction& dura);
      void forward(Fraction& dura);
      void backup(Fraction& dura);
      void timeModification(Fraction& timeMod);
      void pitch(int& step, float& alter, int& oct);
      void rest();
      void skipLogCurrElem();
      bool determineMeasureLength(QVector<Fraction>& ml) const;
      VoiceList getVoiceList(const QString id) const;
      bool determineStaffMoveVoice(const QString& id, const int mxStaff, const QString& mxVoice,
                                   int& msMove, int& msTrack, int& msVoice) const;
      int trackForPart(const QString& id) const;
      bool hasPart(const QString& id) const;
      Part* getPart(const QString& id) const { return _partMap.value(id); }
      MusicXmlPart getMusicXmlPart(const QString& id) const { return _parts.value(id); }
      MusicXMLDrumset getDrumset(const QString& id) const { return _drumsets.value(id); }
      void setDrumsetDefault(const QString& id, const QString& instrId, const NoteHead::Group hg, const int line, const Direction sd);
      MusicXmlInstrList getInstrList(const QString id) const;
      Fraction getMeasureStart(const int i) const;
      int octaveShift(const QString& id, const int staff, const Fraction f) const;

private:
      // functions
      void setFirstInstr(const QString& id, const Fraction stime);

      // generic pass 1 data
      QXmlStreamReader _e;
      int _divs;                                ///< Current MusicXML divisions value
      QMap<QString, MusicXmlPart> _parts;       ///< Parts data, mapped on part id
      QVector<Fraction> _measureLength;         ///< Length of each measure
      QVector<Fraction> _measureStart;          ///< Start time of each measure
      PartMap _partMap;                         ///< TODO merge into MusicXmlPart ??
      QMap<QString, MusicXMLDrumset> _drumsets; ///< Drumset for each part, mapped on part id
      Score* _score;                            ///< MuseScore score
      MxmlLogger* _logger;                      ///< Error logger

      // part specific data (TODO: move to part-specific class)
      Fraction _timeSigDura;                    ///< Measure duration according to last timesig read
      QMap<int, MxmlOctaveShiftDesc> _octaveShifts; ///< Pending octave-shifts
      Fraction _firstInstrSTime;                ///< First instrument start time
      QString _firstInstrId;                    ///< First instrument id
      };

} // namespace Ms
#endif
