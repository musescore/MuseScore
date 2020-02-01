//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "ove.h"

namespace Ove {

#if 0
template <class T>
inline void deleteVector(QList<T*>& vec)
      {
      for (int i = 0; i < vec.size(); ++i)
            delete vec[i];
      }
      // vec.clear();
#endif

//---------------------------------------------------------
//   TickElement
//---------------------------------------------------------

TickElement::TickElement()
      {
      _tick = 0;
      }

//---------------------------------------------------------
//   MeasurePos
//---------------------------------------------------------

MeasurePos::MeasurePos()
      {
      _measure = 0;
      _offset = 0;
      }

//---------------------------------------------------------
//   MeasurePos::shiftMeasure/Offset
//---------------------------------------------------------

MeasurePos MeasurePos::shiftMeasure(int m) const
      {
      MeasurePos mp;
      mp.setMeasure(measure() + m);
      mp.setOffset(offset());

      return mp;
      }

MeasurePos MeasurePos::shiftOffset(int off) const
      {
      MeasurePos mp;
      mp.setMeasure(measure());
      mp.setOffset(offset() + off);

      return mp;
      }

//---------------------------------------------------------
//   MeasurePos operators
//---------------------------------------------------------

bool MeasurePos::operator==(const MeasurePos& mp) const
      {
      return measure() == mp.measure() && offset() == mp.offset();
      }

bool MeasurePos::operator!=(const MeasurePos& mp) const
      {
      return !(*this == mp);
      }

bool MeasurePos::operator<(const MeasurePos& mp) const
      {
      if (measure() != mp.measure())
            return measure() < mp.measure();

      return offset() < mp.offset();
      }

bool MeasurePos::operator<=(const MeasurePos& mp) const
      {
      if (measure() != mp.measure())
            return measure() <= mp.measure();

      return offset() <= mp.offset();
      }

bool MeasurePos::operator>(const MeasurePos& mp) const
      {
      return !(*this <= mp);
      }

bool MeasurePos::operator>=(const MeasurePos& mp) const
      {
      return !(*this < mp);
      }

//---------------------------------------------------------
//   PairElement
//---------------------------------------------------------

PairElement::PairElement()
      {
      _start = new MeasurePos();
      _stop  = new MeasurePos();
      }

PairElement::~PairElement()
      {
      delete _start;
      delete _stop;
      }

//---------------------------------------------------------
//   PairEnds
//---------------------------------------------------------

PairEnds::PairEnds()
      {
      _leftLine      = new LineElement();
      _rightLine     = new LineElement();
      _leftShoulder  = new OffsetElement();
      _rightShoulder = new OffsetElement();
      }

PairEnds::~PairEnds()
      {
      delete _leftLine;
      delete _rightLine;
      delete _leftShoulder;
      delete _rightShoulder;
      }

//---------------------------------------------------------
//   LineElement
//---------------------------------------------------------

LineElement::LineElement()
      {
      _line = 0;
      }

//---------------------------------------------------------
//   OffsetElement
//---------------------------------------------------------

OffsetElement::OffsetElement()
      {
      _xOffset = 0;
      _yOffset = 0;
      }

//---------------------------------------------------------
//   LengthElement
//---------------------------------------------------------

LengthElement::LengthElement()
      {
      _length = 0;
      }

//---------------------------------------------------------
//   MusicData
//---------------------------------------------------------

MusicData::MusicData()
      {
      _musicDataType = MusicDataType::None;
      _show          = true;
      _color         = 0;
      _voice         = 0;
      }

//---------------------------------------------------------
//   MusicData::xmlDataType
//---------------------------------------------------------

MusicData::XmlDataType MusicData::xmlDataType(MusicDataType type)
      {
      XmlDataType xmlType = XmlDataType::None;

      switch (type) {
            case MusicDataType::Measure_Repeat:
                  xmlType = XmlDataType::Attributes;
                  break;
            case MusicDataType::Beam:
                  xmlType = XmlDataType::NoteBeam;
                  break;
            case MusicDataType::Slur:
            case MusicDataType::Glissando:
            case MusicDataType::Tuplet:
            case MusicDataType::Tie:
                  xmlType = XmlDataType::Notations;
                  break;
            case MusicDataType::Text:
            case MusicDataType::Repeat:
            case MusicDataType::Wedge:
            case MusicDataType::Dynamic:
            case MusicDataType::Pedal:
            case MusicDataType::OctaveShift_EndPoint:
                  xmlType = XmlDataType::Direction;
                  break;
            default:
                  xmlType = XmlDataType::None;
                  break;
            }

      return xmlType;
      }

#if 0
//---------------------------------------------------------
//   MusicData::isPairElement
//---------------------------------------------------------

bool MusicData::isPairElement(MusicDataType type)
      {
      bool pair = false;
      
      switch (type) {
            case MusicDataType::Numeric_Ending:
            case MusicDataType::Measure_Repeat:
            case MusicDataType::Wedge:
            case MusicDataType::OctaveShift:
            // case MusicDataType::OctaveShift_EndPoint:
            case MusicDataType::Pedal:
            case MusicDataType::Beam:
            case MusicDataType::Glissando:
            case MusicDataType::Slur:
            case MusicDataType::Tie:
            case MusicDataType::Tuplet:
                  pair = true;
                  break;
            default:
                  break;
            }

      return pair;
      }
#endif

//---------------------------------------------------------
//   MusicData::copyCommonBlock
//---------------------------------------------------------

void MusicData::copyCommonBlock(const MusicData& source)
      {
      setTick(source.tick());
      start()->setOffset(source.start()->offset());
      setColor(source.color());
      }

//---------------------------------------------------------
//   MidiData
//---------------------------------------------------------

MidiData::MidiData()
      {
      _midiType = MidiType::None;
      }

//---------------------------------------------------------
//   OveSong
//---------------------------------------------------------

OveSong::OveSong()
      {
      _codec = nullptr;

      clear();
      }

OveSong::~OveSong()
      {
      clear();
      }

//---------------------------------------------------------
//   OveSong::track
//---------------------------------------------------------

Track* OveSong::track(int part, int staff) const
      {
      int trackId = partStaffToTrack(part, staff);

      if (trackId >= 0 && trackId < trackCount())
            return _tracks[trackId];

      return nullptr;
      }

//---------------------------------------------------------
//   OveSong::page
//---------------------------------------------------------

Page* OveSong::page(int idx) const
      {
      if (idx >= 0 && idx < pageCount())
            return _pages[idx];

      return nullptr;
      }

//---------------------------------------------------------
//   OveSong::line
//---------------------------------------------------------

Line* OveSong::line(int idx) const
      {
      if (idx >= 0 && idx < lineCount())
            return _lines[idx];

      return nullptr;
      }

//---------------------------------------------------------
//   OveSong::measure
//---------------------------------------------------------

Measure* OveSong::measure(int m) const
      {
      if (m >= 0 && m < measureCount())
            return _measures[m];

      return nullptr;
      }

//---------------------------------------------------------
//   OveSong::measureData
//---------------------------------------------------------

MeasureData* OveSong::measureData(int part, int staff/*= 0*/, int bar) const
      {
      int trackId = partStaffToTrack(part, staff);
      int count   = _trackBarCount;

      if (bar >= 0 && bar < count) {
            int measureId = count * trackId + bar;

            if (measureId >= 0 && measureId < measureDataCount())
                  return _measureDatas[measureId];
            }

      return nullptr;
      }

MeasureData* OveSong::measureData(int track, int bar) const
      {
      int count = _trackBarCount;
      int id    = count * track + bar;

      if (id >= 0 && id < measureDataCount())
            return _measureDatas[id];

      return nullptr;
      }

//---------------------------------------------------------
//   OveSong::addPartStaffCounts
//---------------------------------------------------------

void OveSong::addPartStaffCounts(const QList<int>& partStaffCounts)
      {
      for (int i = 0; i < partStaffCounts.size(); ++i)
            _partStaffCounts.push_back(partStaffCounts[i]);
      }

//---------------------------------------------------------
//   OveSong::staffCount
//---------------------------------------------------------

int OveSong::staffCount(int part) const
      {
      if (part >= 0 && part < partCount())
            return _partStaffCounts[part];

      return 0;
      }

//---------------------------------------------------------
//   OveSong::trackToPartStaff
//---------------------------------------------------------

QPair<int, int> OveSong::trackToPartStaff(int track) const
      {
      int staffCount = 0;

      for (int i = 0; i < partCount(); ++i) {
            if (staffCount + _partStaffCounts[i] > track)
                  return qMakePair(i, track - staffCount);

            staffCount += _partStaffCounts[i];
            }

      return qMakePair(partCount(), 0);
      }

//---------------------------------------------------------
//   OveSong::partStaffToTrack
//---------------------------------------------------------

int OveSong::partStaffToTrack(int part, int staff) const
      {
      int i;
      unsigned staffCount = 0;

      for (i = 0; i < partCount(); ++i) {
            if (part == i && staff >= 0 && staff < _partStaffCounts[i]) {
                  int trackId = staffCount + staff;

                  if (trackId >= 0 && trackId < trackCount())
                        return trackId;
                  }

            staffCount += _partStaffCounts[i];
            }

      return trackCount();
      }

//---------------------------------------------------------
//   OveSong::codecString
//---------------------------------------------------------

QString OveSong::codecString(const QByteArray& text)
      {
      QString s;
      if (!_codec)
            s = QString(text);
      else
            s = _codec->toUnicode(text);

      s = s.trimmed();
      return s;
      }

//---------------------------------------------------------
//   OveSong::clear
//---------------------------------------------------------

void OveSong::clear()
      {
      _version4           = true;
      _quarter            = 480;
      _showPageMargin     = false;
      _showTransposeTrack = false;
      _showLineBreak      = false;
      _showRuler          = false;
      _showColor          = true;
      _playRepeat         = true;
      _playStyle          = PlayStyle::Record;

      _annotates.clear();
      _copyrights.clear();
      _footers.clear();
      _headers.clear();
      _titles.clear();
      _writers.clear();

      for (int i = 0; i < trackCount(); ++i) 
            delete _tracks[i];
      for (int i = 0; i < pageCount(); ++i) 
            delete _pages[i];
      for (int i = 0; i < lineCount(); ++i) 
            delete _lines[i];
      for (int i = 0; i < measureCount(); ++i) 
            delete _measures[i];
      for (int i = 0; i < measureDataCount(); ++i) 
            delete _measureDatas[i];

      _tracks.clear();
      _pages.clear();
      _lines.clear();
      _measures.clear();
      _measureDatas.clear();
      _trackBarCount = 0;
      _partStaffCounts.clear();
      }

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

Voice::Voice()
      {
      _channel    = 0;
      _volume     = -1;
      _pitchShift = 0;
      _pan        = 0;
      _patch      = 0;
      _stemType   = 0;
      }

//---------------------------------------------------------
//   Track
//---------------------------------------------------------

Track::Track()
      {
      clear();
      }

Track::~Track()
      {
      clear();
      }

//---------------------------------------------------------
//   Track::clear
//---------------------------------------------------------

void Track::clear() {
      _number = 0;

      _name = QString();

      _patch          = 0;
      _channel        = 0;
      _transpose      = 0;
      _showTranspose  = false;
      _noteShift      = 0;
      _startClef      = ClefType::Treble;
      _transposeClef  = ClefType::Treble;
      _displayPercent = 100;
      _startKey       = KeyType::Key_C;
      _voiceCount     = 8;

      _showName          = true;
      _showBriefName     = false;
      _showKeyEachLine   = false;
      _showLegerLine     = true;
      _showClef          = true;
      _showTimeSignature = true;
      _showKeySignature  = true;
      _showBarline       = true;
      _showClefEachLine  = false;

      _fillWithRest = true;
      _flatTail     = false;

      _mute = false;
      _solo = false;

      _drumKit.clear();

      _part = 0;

      for (int i = 0; i < _voices.size(); ++i)
            delete _voices[i];
      _voices.clear();
      }

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page()
      {
      _beginLine = 0;
      _lineCount = 0;

      _lineInterval        = 9;
      _staffInterval       = 7;
      _staffInlineInterval = 6;

      _lineBarCount  = 4;
      _pageLineCount = 5;

      _leftMargin   = 0xA8;
      _topMargin    = 0xA8;
      _rightMargin  = 0xA8;
      _bottomMargin = 0xA8;

      _pageWidth  = 0x0B40;
      _pageHeight = 0x0E90;
      }

//---------------------------------------------------------
//   Line
//---------------------------------------------------------

Line::Line()
      {
      _beginBar     = 0;
      _barCount     = 0;
      _yOffset      = 0;
      _leftXOffset  = 0;
      _rightXOffset = 0;
      }

Line::~Line()
      {
      for (int i = 0; i < _staffs.size(); ++i)
            delete _staffs[i];
      _staffs.clear();
      }

//---------------------------------------------------------
//   Line::staff
//---------------------------------------------------------

Staff* Line::staff(int idx) const
      {
      if (idx >= 0 && idx < static_cast<int>(_staffs.size()))
            return _staffs[idx];

      return nullptr;
      }

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

Staff::Staff() {
      _clef            = ClefType::Treble;
      _keyType         = 0;
      _visible         = true;
      _groupType       = GroupType::None;
      _groupStaffCount = 0;
      }

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::Note()
      {
      _isRest         = false;
      _note           = 60;
      _accidental     = AccidentalType::Normal;
      _showAccidental = false;
      _offVelocity    = 0x40;
      _onVelocity     = 0x50;
      _headType       = NoteHeadType::Standard;
      _tiePos         = TiePos::None;
      _offsetStaff    = 0;
      _show           = true;
      _offsetTick     = 0;
      }

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation()
      {
      _type  = ArticulationType::Marcato;
      _above = true;

      _changeSoundEffect = false;
      _changeLength      = false;
      _changeVelocity    = false;
      _changeExtraLength = false;

      _soundEffect      = qMakePair(0, 0);
      _lengthPercentage = 100;
      _velocityType     = VelocityType::Offset;
      _velocity         = 0;
      _extraLength      = 0;

      _trillNoteLength = 60;
      _trillRate       = NoteType::Note_Sixteen;
      _accelerateType  = AccelerateType::None;
      _auxiliaryFirst  = false;
      _trillInterval   = TrillInterval::Chromatic;
      }

//---------------------------------------------------------
//   Articulation::setSoundEffect
//---------------------------------------------------------

void Articulation::setSoundEffect(int soundFrom, int soundTo)
      {
      _soundEffect       = qMakePair(soundFrom, soundTo);
      _changeSoundEffect = true;
      }

//---------------------------------------------------------
//   Articulation::setLengthPercentage
//---------------------------------------------------------

void Articulation::setLengthPercentage(int percentage)
      {
      _lengthPercentage = percentage;
      _changeLength     = true;
      }

//---------------------------------------------------------
//   Articulation::setVelocityType
//---------------------------------------------------------

void Articulation::setVelocityType(VelocityType type)
      {
      _velocityType   = type;
      _changeVelocity = true;
      }

//---------------------------------------------------------
//   Articulation::setExtraLength
//---------------------------------------------------------

void Articulation::setExtraLength(int length)
      {
      _extraLength       = length;
      _changeExtraLength = true;
      }

//---------------------------------------------------------
//   Articulation::willAffectNotes
//---------------------------------------------------------

bool Articulation::willAffectNotes() const
      {
      return (articulationType() == ArticulationType::Major_Trill) ||
         (articulationType() == ArticulationType::Minor_Trill) ||
         (articulationType() == ArticulationType::Trill_Section) ||
         (articulationType() == ArticulationType::Inverted_Short_Mordent) || 
         (articulationType() == ArticulationType::Inverted_Long_Mordent) || 
         (articulationType() == ArticulationType::Short_Mordent) ||
         (articulationType() == ArticulationType::Turn) ||
         (articulationType() == ArticulationType::Arpeggio) || 
         (articulationType() == ArticulationType::Tremolo_Eighth) || 
         (articulationType() == ArticulationType::Tremolo_Sixteenth) ||
         (articulationType() == ArticulationType::Tremolo_Thirty_Second) || 
         (articulationType() == ArticulationType::Tremolo_Sixty_Fourth);
      }

//---------------------------------------------------------
//   Articulation::isTrill
//---------------------------------------------------------

bool Articulation::isTrill() const
      {
      return (articulationType() == ArticulationType::Major_Trill) ||
         (articulationType() == ArticulationType::Minor_Trill) ||
         (articulationType() == ArticulationType::Trill_Section);
      }

//---------------------------------------------------------
//   Articulation::xmlType
//---------------------------------------------------------

Articulation::XmlType Articulation::xmlType() const
      {
      XmlType xmlType = XmlType::Unknown;

      switch (_type) {
            case ArticulationType::Major_Trill:
            case ArticulationType::Minor_Trill:
            case ArticulationType::Trill_Section:
            case ArticulationType::Inverted_Short_Mordent:
            case ArticulationType::Inverted_Long_Mordent:
            case ArticulationType::Short_Mordent:
            case ArticulationType::Turn:
            // case ArticulationType::Flat_Accidental_For_Trill:
            // case ArticulationType::Sharp_Accidental_For_Trill:
            // case ArticulationType::Natural_Accidental_For_Trill:
            case ArticulationType::Tremolo_Eighth:
            case ArticulationType::Tremolo_Sixteenth:
            case ArticulationType::Tremolo_Thirty_Second:
            case ArticulationType::Tremolo_Sixty_Fourth:
                  xmlType = XmlType::Ornament;
                  break;
            case ArticulationType::Marcato:
            case ArticulationType::Marcato_Dot:
            case ArticulationType::Heavy_Attack:
            case ArticulationType::SForzando:
            case ArticulationType::SForzando_Inverted:
            case ArticulationType::SForzando_Dot:
            case ArticulationType::SForzando_Dot_Inverted:
            case ArticulationType::Heavier_Attack:
            case ArticulationType::Staccatissimo:
            case ArticulationType::Staccato:
            case ArticulationType::Tenuto:
            case ArticulationType::Pause:
            case ArticulationType::Grand_Pause:
                  xmlType = XmlType::Articulation;
                  break;
            case ArticulationType::Up_Bow:
            case ArticulationType::Down_Bow:
            case ArticulationType::Up_Bow_Inverted:
            case ArticulationType::Down_Bow_Inverted:
            case ArticulationType::Natural_Harmonic:
            case ArticulationType::Artificial_Harmonic:
            case ArticulationType::Finger_1:
            case ArticulationType::Finger_2:
            case ArticulationType::Finger_3:
            case ArticulationType::Finger_4:
            case ArticulationType::Finger_5:
            case ArticulationType::Plus_Sign:
                  xmlType = XmlType::Technical;
                  break;
            case ArticulationType::Arpeggio: 
                  xmlType = XmlType::Arpeggiate;
                  break;
            case ArticulationType::Fermata:
            case ArticulationType::Fermata_Inverted:
                  xmlType = XmlType::Fermata;
                  break;
            case ArticulationType::Pedal_Down:
            case ArticulationType::Pedal_Up:
                  xmlType = XmlType::Direction;
                  break;
            // case ArticulationType::Toe_Pedal:
            // case ArticulationType::Heel_Pedal:
            // case ArticulationType::Toe_To_Heel_Pedal:
            // case ArticulationType::Heel_To_Toe_Pedal:
            // case ArticulationType::Open_String:
            default:
                  break;
            }

      return xmlType;
      }

//---------------------------------------------------------
//   NoteContainer
//---------------------------------------------------------

NoteContainer::NoteContainer()
      {
      _musicDataType = MusicDataType::Note_Container;

      _isGrace       = false;
      _isCue         = false;
      _isRest        = false;
      _isRaw         = false;
      _noteType      = NoteType::Note_Quarter;
      _dot           = 0;
      _graceNoteType = NoteType::Note_Eight;
      _up            = true;
      _showStem      = true;
      _stemLength    = 7;
      _inBeam        = false;
      _tuplet        = 0;
      _space         = 2; // div by 0
      _noteShift     = 0;
      }

NoteContainer::~NoteContainer()
      {
      for (int i = 0; i < _notes.size(); ++i)
            delete _notes[i];
      _notes.clear();
      for (int i = 0; i < _articulations.size(); ++i)
            delete _articulations[i];
      _articulations.clear();
      }

//---------------------------------------------------------
//   NoteContainer::setNoteType
//---------------------------------------------------------

void NoteContainer::setNoteType(NoteType type)
      {
      if (type != NoteType::Note_None)
            _noteType = type;
      else
            _noteType = NoteType::Note_Quarter;
      }

//---------------------------------------------------------
//   NoteContainer::offsetStaff
//---------------------------------------------------------

int NoteContainer::offsetStaff() const
      {
      if (isRest())
            return 0;

      int staffMove = 0;
      QList<Ove::Note*> notes = notesRests();
      for (int i = 0; i < notes.size(); ++i) {
            Ove::Note* notePtr = notes[i];
            staffMove = notePtr->offsetStaff();
            }

      return staffMove;
      }

//---------------------------------------------------------
//   NoteContainer::duration
//---------------------------------------------------------

int NoteContainer::duration() const
      {
      int duration = static_cast<int>(NoteDuration::D_4);

      switch (_noteType) {
            case NoteType::Note_DoubleWhole:
                  duration = static_cast<int>(NoteDuration::D_Double_Whole);
                  break;
            case NoteType::Note_Whole:
                  duration = static_cast<int>(NoteDuration::D_Whole);
                  break;
            case NoteType::Note_Half:
                  duration = static_cast<int>(NoteDuration::D_2);
                  break;
            case NoteType::Note_Quarter:
                  duration = static_cast<int>(NoteDuration::D_4);
                  break;
            case NoteType::Note_Eight:
                  duration = static_cast<int>(NoteDuration::D_8);
                  break;
            case NoteType::Note_Sixteen:
                  duration = static_cast<int>(NoteDuration::D_16);
                  break;
            case NoteType::Note_32:
                  duration = static_cast<int>(NoteDuration::D_32);
                  break;
            case NoteType::Note_64:
                  duration = static_cast<int>(NoteDuration::D_64);
                  break;
            case NoteType::Note_128:
                  duration = static_cast<int>(NoteDuration::D_128);
                  break;
            case NoteType::Note_256:
                  duration = static_cast<int>(NoteDuration::D_256);
                  break;
            default:
                  break;
            }

      int dotLength = duration;

      for (int i = 0; i < _dot; ++i)
            dotLength /= 2;

      dotLength = duration - dotLength;

      duration += dotLength;

      return duration;
      }

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam()
      {
      _musicDataType = MusicDataType::Beam;

      _isGrace       = false;
      }

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie()
      {
      _musicDataType = MusicDataType::Tie;

      _showOnTop     = true;
      _note          = 72;
      _height        = 24;
      }

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

Glissando::Glissando()
      {
      _musicDataType = MusicDataType::Glissando;

      _straight      = true;
      _text          = "gliss.";
      _lineThick     = 8;
      }

//---------------------------------------------------------
//   Decorator
//---------------------------------------------------------

Decorator::Decorator()
      {
      _musicDataType    = MusicDataType::Decorator;

      _decoratorType    = DecoratorType::Articulation;
      _articulationType = ArticulationType::Marcato;
      }

//---------------------------------------------------------
//   MeasureRepeat
//---------------------------------------------------------

MeasureRepeat::MeasureRepeat()
      {
      _musicDataType = MusicDataType::Measure_Repeat;

      _singleRepeat  = true;
      }

//---------------------------------------------------------
//   MeasureRepeat::setSingleRepeat
//---------------------------------------------------------

void MeasureRepeat::setSingleRepeat(bool single)
      {
      _singleRepeat = single;

      start()->setMeasure(0);
      start()->setOffset(0);
      stop()->setMeasure(single ? 1 : 2);
      stop()->setOffset(0);
      }

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

Tuplet::Tuplet()
      {
      _musicDataType = MusicDataType::Tuplet;

      _tuplet        = 3;
      _space         = 2;
      _height        = 0;
      _noteType      = NoteType::Note_Quarter;
      _mark          = new OffsetElement();
      }

Tuplet::~Tuplet()
      {
      delete _mark;
      }

//---------------------------------------------------------
//   Harmony
//---------------------------------------------------------

Harmony::Harmony()
      {
      _musicDataType = MusicDataType::Harmony;

      _harmonyType   = "";
      _root          = 0;
      _bass          = -1; // 0xff
      _alterRoot     = 0;
      _alterBass     = 0;
      _bassOnBottom  = false;
      _angle         = 0;
      }

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

Clef::Clef()
      {
      _musicDataType = MusicDataType::Clef;

      _clefType      = ClefType::Treble;
      }

//---------------------------------------------------------
//   Lyric
//---------------------------------------------------------

Lyric::Lyric()
      {
      _musicDataType = MusicDataType::Lyric;

      _lyric         = QString();
      _verse         = 0;
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::Slur()
      {
      _musicDataType   = MusicDataType::Slur;

      _containerCount  = 1;
      _showOnTop       = true;
      _noteTimePercent = 100;

      _handle2         = new OffsetElement();
      _handle3         = new OffsetElement();
      }

Slur::~Slur()
      {
      delete _handle2;
      delete _handle3;
      }

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic()
      {
      _musicDataType = MusicDataType::Dynamic;

      _dynamicType   = DynamicType::PPPP;
      _play          = true;
      _velocity      = 30;
      }

//---------------------------------------------------------
//   WedgeEndPoint
//---------------------------------------------------------

WedgeEndPoint::WedgeEndPoint()
      {
      _musicDataType = MusicDataType::Wedge_EndPoint;

      _wedgeType     = WedgeType::Crescendo;
      _height        = 24;
      _wedgeStart    = true;
      }

//---------------------------------------------------------
//   Wedge
//---------------------------------------------------------

Wedge::Wedge()
      {
      _musicDataType = MusicDataType::Wedge;

      _wedgeType     = WedgeType::Crescendo;
      _height        = 24;
      }

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal()
      {
      _musicDataType = MusicDataType::Pedal;

      _half        = false;
      _play        = false;
      _playOffset  = 0;

      _pedalHandle = new OffsetElement();
      }

Pedal::~Pedal()
      {
      delete _pedalHandle;
      }

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket()
      {
      _musicDataType = MusicDataType::Bracket;

      _bracketType   = BracketType::Parentheses;
      _height        = 0;
      }

//---------------------------------------------------------
//   Expression
//---------------------------------------------------------

Expression::Expression()
      {
      _musicDataType = MusicDataType::Expression;

      _text          = QString();
      }

//---------------------------------------------------------
//   HarpPedal
//---------------------------------------------------------

HarpPedal::HarpPedal()
      {
      _type          = HarpPedalType::Graph;
      _showCharFlag  = 0;
      _musicDataType = MusicDataType::Harp_Pedal;
      }

//---------------------------------------------------------
//   OctaveShift
//---------------------------------------------------------

OctaveShift::OctaveShift()
      {
      _musicDataType       = MusicDataType::OctaveShift;

      _octaveShiftType     = OctaveShiftType::OS_8;
      _octaveShiftPosition = OctaveShiftPosition::Start;
      _endTick             = 0;
      }

//---------------------------------------------------------
//   OctaveShift::noteShift
//---------------------------------------------------------

int OctaveShift::noteShift() const
      {
      int shift = 12;

      switch (octaveShiftType()) {
            case OctaveShiftType::OS_8:
                  break;
            case OctaveShiftType::OS_Minus_8:
                  shift = -12;
                  break;
            case OctaveShiftType::OS_15:
                  shift = 24;
                  break;
            case OctaveShiftType::OS_Minus_15:
                  shift = -24;
                  break;
            default:
                  break;
            }

      return shift;
      }

//---------------------------------------------------------
//   OctaveShiftEndPoint
//---------------------------------------------------------

OctaveShiftEndPoint::OctaveShiftEndPoint()
      {
      _musicDataType       = MusicDataType::OctaveShift_EndPoint;

      _octaveShiftType     = OctaveShiftType::OS_8;
      _octaveShiftPosition = OctaveShiftPosition::Start;
      _endTick             = 0;
      }

//---------------------------------------------------------
//   MultiMeasureRest
//---------------------------------------------------------

MultiMeasureRest::MultiMeasureRest()
      {
      _musicDataType = MusicDataType::Multi_Measure_Rest;
      _measureCount  = 0;
      }

//---------------------------------------------------------
//   Tempo
//---------------------------------------------------------

Tempo::Tempo()
      {
      _musicDataType   = MusicDataType::Tempo;

      _leftNoteType    = NoteType::Note_Quarter;
      _showMark        = false;
      _showBeforeText  = false;
      _showParentheses = false;
      _typeTempo       = 96;
      _leftText        = QString();
      _rightText       = QString();
      _swingEighth     = false;
      _rightNoteType   = NoteType::Note_Quarter;
      _leftNoteDot     = false;
      _rightNoteDot    = false;
      _rightSideType   = 0;
      }

//---------------------------------------------------------
//   Tempo::quarterTempo
//---------------------------------------------------------

qreal Tempo::quarterTempo() const
      {
      qreal factor = pow(2.0, int(NoteType::Note_Quarter) - int(leftNoteType()));
      if (leftNoteDot())
            factor *= 1.5;
      qreal tempo = typeTempo() * factor;

      return tempo;
      }

//---------------------------------------------------------
//  Text
//---------------------------------------------------------

Text::Text()
      {
      _musicDataType    = MusicDataType::Text;

      _textType         = TextType::Rehearsal;
      _horizontalMargin = 8;
      _verticalMargin   = 8;
      _lineThick        = 4;
      _text             = QString();
      _width            = 0;
      _height           = 0;
      }

//---------------------------------------------------------
//   TimeSignature
//---------------------------------------------------------

TimeSignature::TimeSignature()
      {
      _numerator      = 4;
      _denominator    = 4;
      _isSymbol       = false;
      _beatLength     = 480;
      _barLength      = 1920;
      _barLengthUnits = 0x400;
      _replaceFont    = false;
      _showBeatGroup  = false;

      _groupNumerator1   = 0;
      _groupNumerator2   = 0;
      _groupNumerator3   = 0;
      _groupDenominator1 = 4;
      _groupDenominator2 = 4;
      _groupDenominator3 = 4;

      _beamGroup1 = 4;
      _beamGroup2 = 0;
      _beamGroup3 = 0;
      _beamGroup4 = 0;

      _beamCount16th = 4;
      _beamCount32th = 1;
      }

//---------------------------------------------------------
//   TimeSignature::isSymbol
//---------------------------------------------------------

bool TimeSignature::isSymbol() const
      {
      if (_numerator == 2 && _denominator == 2)
            return true;

      return _isSymbol;
      }

//---------------------------------------------------------
//   TimeSignature::addBeat
//---------------------------------------------------------

void TimeSignature::addBeat(int startUnit, int lengthUnit, int startTick)
      {
      BeatNode node;
      node._startUnit = startUnit;
      node._lengthUnit = lengthUnit;
      node._startTick = startTick;
      _beats.push_back(node);
      }

//---------------------------------------------------------
//   TimeSignature::endAddBeat
//---------------------------------------------------------

void TimeSignature::endAddBeat()
      {
      int i;
      _barLengthUnits = 0;

      for (i = 0; i < _beats.size(); ++i)
            _barLengthUnits += _beats[i]._lengthUnit;
      }

//---------------------------------------------------------
//   Key
//---------------------------------------------------------

Key::Key()
      {
      _key = 0;
      _set = false;
      _previousKey = 0;
      _symbolCount = 0;
      }

//---------------------------------------------------------
//   Key::setKey
//---------------------------------------------------------

void Key::setKey(int key)
      {
      _key = key;
      _set = true;
      }

//---------------------------------------------------------
//   RepeatSymbol
//---------------------------------------------------------

RepeatSymbol::RepeatSymbol()
      {
      _musicDataType = MusicDataType::Repeat;

      _text          = "#1";
      _repeatType    = RepeatType::Segno;
      }

//---------------------------------------------------------
//   NumericEnding
//---------------------------------------------------------

NumericEnding::NumericEnding()
      {
      _musicDataType = MusicDataType::Numeric_Ending;

      _height        = 0;
      _text          = QString();
      _numericHandle = new OffsetElement();
      }

NumericEnding::~NumericEnding()
      {
      delete _numericHandle;
      }

//---------------------------------------------------------
//   NumericEnding::numbers
//---------------------------------------------------------

QList<int> NumericEnding::numbers() const
      {
      int i;
      QStringList strs = _text.split(",", QString::SkipEmptyParts);
      QList<int> endings;

      for (i = 0; i < strs.size(); ++i) {
            bool ok;
            int num = strs[i].toInt(&ok);
            endings.push_back(num);
            }

      return endings;
      }

//---------------------------------------------------------
//   NumericEnding::jumpCount
//---------------------------------------------------------

int NumericEnding::jumpCount() const
      {
      QList<int> _numbers = numbers();
      int count = 0;

      for (int i = 0; i < _numbers.size(); ++i) {
            if (i + 1 != _numbers[i])
                  break;

            count = i + 1;
            }

      return count;
      }

//---------------------------------------------------------
//   BarNumber
//---------------------------------------------------------

BarNumber::BarNumber()
      {
      _index                = 0;
      _showOnParagraphStart = false;
      _align                = 0;
      _showFlag             = 1; // staff
      _barRange             = 1; // can't be 0
      _prefix               = QString();
      }

//---------------------------------------------------------
//   MidiController
//---------------------------------------------------------

MidiController::MidiController()
      {
      _midiType   = MidiType::Controller;

      _controller = 64; // pedal
      _value      = 0;
      }

//---------------------------------------------------------
//   MidiProgramChange
//---------------------------------------------------------

MidiProgramChange::MidiProgramChange()
      {
      _midiType = MidiType::Program_Change;

      _patch    = 0; // grand piano
      }

//---------------------------------------------------------
//   MidiChannelPressure
//---------------------------------------------------------

MidiChannelPressure::MidiChannelPressure()
      {
      _midiType = MidiType::Channel_Pressure;

      _pressure = 0;
      }

//---------------------------------------------------------
//   MidiPitchWheel
//---------------------------------------------------------

MidiPitchWheel::MidiPitchWheel()
      {
      _midiType = MidiType::Pitch_Wheel;

      _value    = 0;
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::Measure(int index)
      {
      _barNumber = new BarNumber();
      _barNumber->setIndex(index);
      _timeSig   = new TimeSignature();

      clear();
      }

Measure::~Measure()
      {
      clear();

      delete _barNumber;
      delete _timeSig;
      }

//---------------------------------------------------------
//   Measure::clear
//---------------------------------------------------------

void Measure::clear()
      {
      _leftBarlineType       = BarLineType::Default;
      _rightBarlineType      = BarLineType::Default;
      _repeatCount           = 1;
      _typeTempo             = 96.00;
      setLength(0x780); // time = 4/4
      _isPickup              = false;
      _isMultiMeasureRest    = false;
      _multiMeasureRestCount = 0;
      }

//---------------------------------------------------------
//   MeasureData
//---------------------------------------------------------

MeasureData::MeasureData()
      {
      _key  = new Key();
      _clef = new Clef();
      }

MeasureData::~MeasureData()
      {
      int i;
      for (i = 0; i < _musicDatas.size(); ++i)
            delete _musicDatas[i];
      _musicDatas.clear();

      // _noteContainers also in _musicDatas, no need to destroy
      _noteContainers.clear();

      // only delete at element start
      for (i = 0; i < _crossMeasureElements.size(); ++i) {
            if (_crossMeasureElements[i].second)
                  delete _crossMeasureElements[i].first;
            }
      _crossMeasureElements.clear();

      for (i = 0; i < _midiDatas.size(); ++i)
            delete _midiDatas[i];
      _midiDatas.clear();

      delete _key;
      delete _clef;
      }

//---------------------------------------------------------
//   MeasureData::musicDatas
//---------------------------------------------------------

QList<MusicData*> MeasureData::musicDatas(MusicDataType type) {
      int i;
      QList<MusicData*> notations;

      for (i = 0; i < _musicDatas.size(); ++i) {
            if (type == MusicDataType::None || _musicDatas[i]->musicDataType() == type)
                  notations.push_back(_musicDatas[i]);
            }

      return notations;
      }

//---------------------------------------------------------
//   MeasureData::crossMeasureElements
//---------------------------------------------------------

QList<MusicData*> MeasureData::crossMeasureElements(MusicDataType type, PairType pairType)
      {
      int i;
      QList<MusicData*> pairs;

      for (i = 0; i < _crossMeasureElements.size(); ++i) {
            if ((type == MusicDataType::None || _crossMeasureElements[i].first->musicDataType() == type)
                && (pairType == PairType::All || ((_crossMeasureElements[i].second && pairType == PairType::Start)
                                                 || (!_crossMeasureElements[i].second && pairType == PairType::Stop))))
                  pairs.push_back(_crossMeasureElements[i].first);
            }

      return pairs;
      }

//---------------------------------------------------------
//   MeasureData::midiDatas
//---------------------------------------------------------

QList<MidiData*> MeasureData::midiDatas(MidiType type)
      {
      int i;
      QList<MidiData*> datas;

      for (i = 0; i < _midiDatas.size(); ++i) {
            if (type == MidiType::None || _midiDatas[i]->midiType() == type)
                  datas.push_back(_midiDatas[i]);
            }

      return datas;
      }

//---------------------------------------------------------
//   StreamHandle
//---------------------------------------------------------

StreamHandle::StreamHandle()
      {
      _size   = 0;
      _curPos = 0;
      _point  = nullptr;
      }

StreamHandle::StreamHandle(unsigned char* p, int size)
      {
      _size   = size;
      _curPos = 0;
      _point  = p;
      }

StreamHandle::~StreamHandle()
      {
      _point = nullptr;
      }

//---------------------------------------------------------
//   StreamHandle::read
//---------------------------------------------------------

bool StreamHandle::read(char* buff, int size)
      {
      if (_point && _curPos + size <= _size) {
            memcpy(buff, _point + _curPos, size);
            _curPos += size;

            return true;
            }

      return false;
      }

//---------------------------------------------------------
//   Block
//---------------------------------------------------------

Block::Block()
      {
      doResize(0);
      }

Block::Block(unsigned count)
      {
      doResize(count);
      }

//---------------------------------------------------------
//   Block::doResize
//---------------------------------------------------------

void Block::doResize(unsigned count)
      {
      _data.clear();
      for (unsigned i = 0; i < count; ++i)
            _data.push_back('\0');
      // _data.resize(count);
      }

/*
Suggestions:

> Seeing as it follows a readBuffer(buf, 1) I would go with casting the 1 byte directly
> So I'd add
`unsigned char toUnsignedChar() const;
signed char toChar() const; ` to the Block class there
> and have the first return *data() and the second return (signed char)(*data())
> then replace to to(Unsigned)Int calls that follow readBuffer(1)
> (OR truly refactor and use a real parser then;) )
*/

//---------------------------------------------------------
//   Block::toBool
//---------------------------------------------------------

bool Block::toBool() const
      {
      if (!data())
            return false;

      return size() == 1 && data()[0] == 0x01;
      }

//---------------------------------------------------------
//   Block::toUnsignedInt
//---------------------------------------------------------

unsigned Block::toUnsignedInt() const
      {
      if (!data())
            return 0;

      int num = 0;

      for (int i = 0; i < int(sizeof(int)) && i < size(); ++i)
            num = (num << CHAR_BIT) + *(data() + i);

      return num;
      }

//---------------------------------------------------------
//   Block::toInt
//---------------------------------------------------------

int Block::toInt() const
      {
      if (!data())
            return 0;

      int i;
      int num = 0;

      for (i = 0; i < static_cast<int>(sizeof(int)) && i < size(); ++i)
            num = (num << CHAR_BIT) + static_cast<int>(*(data() + i));

      size_t minSize = sizeof(int);
      minSize = qMin(size(), static_cast<int>(minSize));

      if (*(data()) & (SCHAR_MAX + 1)) {
            int maxNum = static_cast<int>(pow(qreal(SCHAR_MAX + 1), static_cast<int>(minSize)));
            num -= maxNum;
            //num *= -1;
            }

      return num;
      }

//---------------------------------------------------------
//   Block::toStrByteArray
//---------------------------------------------------------

QByteArray Block::toStrByteArray() const
      {
      if (!data())
            return QByteArray();

      QByteArray arr((char*)(data()), size());

      return arr;
      }

//---------------------------------------------------------
//   Block::fixedSizeBufferToStrByteArray
//---------------------------------------------------------

QByteArray Block::fixedSizeBufferToStrByteArray() const
      {
      QByteArray str;

      for (int i = 0; i < size(); ++i) {
            if (*(data() + i) == '\0')
                  break;

            str += char(*(data() + i));
            }

      return str;
      }

//---------------------------------------------------------
//   Block::operators
//---------------------------------------------------------

bool Block::operator==(const Block& block) const
      {
      if (size() != block.size())
            return false;

      for (int i = 0; i < size() && i < block.size(); ++i) {
            if (*(data() + i) != *(block.data() + i))
                  return false;
            }

      return true;
      }

bool Block::operator!=(const Block& block) const
      {
      return !(*this == block);
      }

//---------------------------------------------------------
//   FixedBlock
//---------------------------------------------------------

FixedBlock::FixedBlock()
   : Block() 
      {
      }

FixedBlock::FixedBlock(unsigned count)
   : Block(count)
      {
      }

//---------------------------------------------------------
//   FixedBlock::resize
//---------------------------------------------------------

void FixedBlock::resize(unsigned /*count*/)
      {
      // Block::resize(size);
      }

//---------------------------------------------------------
//   SizeBlock
//---------------------------------------------------------

SizeBlock::SizeBlock()
   : FixedBlock(4)
      {
      }

//---------------------------------------------------------
//   SizeBlock::toSize()
//---------------------------------------------------------

unsigned SizeBlock::toSize() const
      {
      unsigned i;
      unsigned num(0);

      for (i = 0; i < int(sizeof(int)); ++i)
            num = (num << CHAR_BIT) + *(data() + i);

      return num;
      }

#if 0
void SizeBlock::fromUnsignedInt(unsigned count)
      {
      unsigned_int_to_char_buffer(count, data());
      }
#endif

//---------------------------------------------------------
//   NameBlock
//---------------------------------------------------------

NameBlock::NameBlock()
   : FixedBlock(4)
      {
      }

#if 0
void NameBlock::setValue(const char* const name)
      {
      unsigned i;

      for (i = 0; i < size() && *(name+i) != '\0'; ++i)
            *(data() + i) = *(name + i);
      }
#endif

//---------------------------------------------------------
//   NameBlock::isEqual
//---------------------------------------------------------

bool NameBlock::isEqual(const QString& name) const
      {
      int nsize = name.size();

      if (nsize != size())
            return false;

      for (int i = 0; i < size() && nsize; ++i) {
            if (data()[i] != name[i])
                  return false;
            }

      return true;
      }

//---------------------------------------------------------
//   CountBlock
//---------------------------------------------------------

CountBlock::CountBlock() 
   : FixedBlock(2)
      {
      }

#if 0
void CountBlock::setValue(unsigned short count)
      {
      unsigned i;
      unsigned SIZE = sizeof(unsigned short);

      for (i = 0; i < SIZE; ++i) {
            data()[SIZE - 1 - i] = count % 256;
            count /= 256;
            }
      }
#endif

//---------------------------------------------------------
//   CountBlock::toCount
//---------------------------------------------------------

unsigned short CountBlock::toCount() const
      {
      unsigned short num = 0;

      for (int i = 0; i < size() && i < int(sizeof(unsigned short)); ++i)
            num = (num << CHAR_BIT) + *(data() + i);

      return num;
      }

const QString Chunk::TrackName   = "TRAK";
const QString Chunk::PageName    = "PAGE";
const QString Chunk::LineName    = "LINE";
const QString Chunk::StaffName   = "STAF";
const QString Chunk::MeasureName = "MEAS";
const QString Chunk::ConductName = "COND";
const QString Chunk::BdatName    = "BDAT";

//---------------------------------------------------------
//   Chunk
//---------------------------------------------------------

Chunk::Chunk()
      {
      }

const unsigned SizeChunk::version3TrackSize = 0x13a;

//---------------------------------------------------------
//   Chunk::name
//---------------------------------------------------------

NameBlock Chunk::name() const
      {
      return _nameBlock;
      }

//---------------------------------------------------------
//   SizeChunk
//---------------------------------------------------------

SizeChunk::SizeChunk()
   : Chunk()
      {
      _sizeBlock = new SizeBlock();
      _dataBlock = new Block();
      }

SizeChunk::~SizeChunk()
      {
      delete _sizeBlock;
      delete _dataBlock;
      }

//---------------------------------------------------------
//   GroupChunk
//---------------------------------------------------------

GroupChunk::GroupChunk()
   : Chunk()
      {
      _childCount = new CountBlock();
      }

GroupChunk::~GroupChunk()
      {
      delete _childCount;
      }

inline unsigned highNibble(unsigned byte) { return byte / 0x10; }
inline unsigned lowNibble(unsigned byte)  { return byte % 0x10; }

//---------------------------------------------------------
//   oveKeyToKey
//---------------------------------------------------------

static int oveKeyToKey(int oveKey)
      {
      int key = 0;

      if (oveKey == 0)
            key = 0;
      else if (oveKey > 7)
            key = oveKey - 7;
      else if (oveKey <= 7)
            key = oveKey * (-1);

      return key;
      }

//---------------------------------------------------------
//   BasicParse
//---------------------------------------------------------

BasicParse::BasicParse(OveSong* ove)
      {
      _ove    = ove;
      _handle = nullptr;
      _notify = nullptr;
      }

BasicParse::BasicParse()
      {
      _ove    = nullptr;
      _handle = nullptr;
      _notify = nullptr;
      }

BasicParse::~BasicParse()
      {
      _ove    = nullptr;
      _handle = nullptr;
      _notify = nullptr;
      }

//---------------------------------------------------------
//   BasicParse::readBuffer
//---------------------------------------------------------

bool BasicParse::readBuffer(Block& placeHolder, int size)
      {
      if (!_handle)
            return false;
      if (placeHolder.size() != size)
            placeHolder.resize(size);

      if (size > 0)
            return _handle->read((char*)(placeHolder.data()), placeHolder.size());

      return true;
      }

//---------------------------------------------------------
//   BasicParse::jump
//---------------------------------------------------------

bool BasicParse::jump(int offset)
      {
      if (!_handle || offset < 0)
            return false;

      if (offset > 0) {
            Block placeHolder(offset);
            return _handle->read((char*)(placeHolder.data()), placeHolder.size());
            }

      return true;
      }

//---------------------------------------------------------
//   BasicParse::messageOut
//---------------------------------------------------------

void BasicParse::messageOut(const QString& str)
      {
      if (_notify)
            _notify->loadInfo(str);
      }

//---------------------------------------------------------
//   OvscParse
//---------------------------------------------------------

OvscParse::OvscParse(OveSong* ove)
   : BasicParse(ove)
      {
      _chunk = nullptr;
      }

OvscParse::~OvscParse()
      {
      _chunk = nullptr;
      }

//---------------------------------------------------------
//   OvscParse::parse
//---------------------------------------------------------

bool OvscParse::parse()
      {
      Block* dataBlock = _chunk->dataBlock();
      unsigned blockSize = _chunk->sizeBlock()->toSize();
      StreamHandle handle(dataBlock->data(), blockSize);
      Block placeHolder;

      _handle = &handle;

      // version
      if (!readBuffer(placeHolder, 1)) { return false; }
      bool version4 = (placeHolder.toUnsignedInt() == 4);
      _ove->setIsVersion4(version4);

      QString str = QString("This file is created by Overture ") + (version4 ? "4" : "3") + "\n";
      messageOut(str);

      if (!jump(6)) { return false; }

      // show page margin
      if (!readBuffer(placeHolder, 1)) { return false; }
      _ove->setShowPageMargin(placeHolder.toBool());

      if (!jump(1)) { return false; }

      // transpose track
      if (!readBuffer(placeHolder, 1)) { return false; }
      _ove->setShowTransposeTrack(placeHolder.toBool());

      // play repeat
      if (!readBuffer(placeHolder, 1)) { return false; }
      _ove->setPlayRepeat(placeHolder.toBool());

      // play style
      if (!readBuffer(placeHolder, 1)) { return false; }
      OveSong::PlayStyle style = OveSong::PlayStyle::Record;
      if (placeHolder.toUnsignedInt() == 1)
            style = OveSong::PlayStyle::Swing;
      else if (placeHolder.toUnsignedInt() == 2)
            style = OveSong::PlayStyle::Notation;
      _ove->setPlayStyle(style);

      // show line break
      if (!readBuffer(placeHolder, 1)) { return false; }
      _ove->setShowLineBreak(placeHolder.toBool());

      // show ruler
      if (!readBuffer(placeHolder, 1)) { return false; }
      _ove->setShowRuler(placeHolder.toBool());

      // show color
      if (!readBuffer(placeHolder, 1)) { return false; }
      _ove->setShowColor(placeHolder.toBool());

      return true;
      }

//---------------------------------------------------------
//   TrackParse
//---------------------------------------------------------

TrackParse::TrackParse(OveSong* ove)
   : BasicParse(ove)
      {
      _chunk = nullptr;
      }

TrackParse::~TrackParse()
      {
      _chunk = nullptr;
      }

//---------------------------------------------------------
//   TrackParse::parse
//---------------------------------------------------------

bool TrackParse::parse()
      {
      Block* dataBlock = _chunk->dataBlock();
      unsigned blockSize = _ove->isVersion4() ? _chunk->sizeBlock()->toSize() : SizeChunk::version3TrackSize;
      StreamHandle handle(dataBlock->data(), blockSize);
      Block placeHolder;

      _handle = &handle;

      Track* oveTrack = new Track();
      _ove->addTrack(oveTrack);

      // 2 32bytes long track name buffer
      if (!readBuffer(placeHolder, 32)) { return false; }
      oveTrack->setName(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if (!readBuffer(placeHolder, 32)) { return false; }
      oveTrack->setBriefName(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if (!jump(8)) { return false; } // 0xfffa0012 fffa0012
      if (!jump(1)) { return false; }

      // patch
      if (!readBuffer(placeHolder, 1)) { return false; }
      unsigned thisByte = placeHolder.toInt();
      oveTrack->setPatch(thisByte & 0x7f);

      // show name
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowName(placeHolder.toBool());

      // show brief name
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowBriefName(placeHolder.toBool());

      if (!jump(1)) { return false; }

      // show transpose
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowTranspose(placeHolder.toBool());

      if (!jump(1)) { return false; }

      // mute
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setMute(placeHolder.toBool());

      // solo
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setSolo(placeHolder.toBool());

      if (!jump(1)) { return false; }

      // show key each line
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowKeyEachLine(placeHolder.toBool());

      // voice count
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setVoiceCount(placeHolder.toUnsignedInt());

      if (!jump(3)) { return false; }

      // transpose value [-127, 127]
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setTranspose(placeHolder.toInt());

      if (!jump(2)) { return false; }

      // start clef
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setStartClef(ClefType(placeHolder.toUnsignedInt()));

      // transpose clef
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setTransposeClef(ClefType(placeHolder.toUnsignedInt()));

      // start key
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setStartKey(KeyType(placeHolder.toUnsignedInt()));

      // display percent
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setDisplayPercent(placeHolder.toUnsignedInt());

      // show leger line
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowLegerLine(placeHolder.toBool());

      // show clef
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowClef(placeHolder.toBool());

      // show time signature
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowTimeSignature(placeHolder.toBool());

      // show key signature
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowKeySignature(placeHolder.toBool());

      // show barline
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowBarline(placeHolder.toBool());

      // fill with rest
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setFillWithRest(placeHolder.toBool());

      // flat tail
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setFlatTail(placeHolder.toBool());

      // show clef each line
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setShowClefEachLine(placeHolder.toBool());

      if (!jump(12)) { return false; }

      // 8 voices
      int i;
      QList<Voice*> voices;
      for (i = 0; i < 8; ++i) {
            Voice* voicePtr = new Voice();

            if (!jump(5)) { return false; }

            // channel
            if (!readBuffer(placeHolder, 1)) { return false; }
            voicePtr->setChannel(placeHolder.toUnsignedInt());

            // volume
            if (!readBuffer(placeHolder, 1)) { return false; }
            voicePtr->setVolume(placeHolder.toInt());

            // pitch shift
            if (!readBuffer(placeHolder, 1)) { return false; }
            voicePtr->setPitchShift(placeHolder.toInt());

            // pan
            if (!readBuffer(placeHolder, 1)) { return false; }
            voicePtr->setPan(placeHolder.toInt());

            if (!jump(6)) { return false; }

            // patch
            if (!readBuffer(placeHolder, 1)) { return false; }
            voicePtr->setPatch(placeHolder.toInt());

            voices.push_back(voicePtr);
            }

      // stem type
      for (i = 0; i < 8; ++i) {
            if (!readBuffer(placeHolder, 1)) { return false; }
            voices[i]->setStemType(placeHolder.toUnsignedInt());

            oveTrack->addVoice(voices[i]);
            }

      // percussion define
      QList<Track::DrumNode> nodes;
      for (i = 0; i < 16; ++i)
            nodes.push_back(Track::DrumNode());

      // line
      for (i = 0; i < 16; ++i) {
            if (!readBuffer(placeHolder, 1)) { return false; }
            nodes[i]._line = placeHolder.toInt();
            }

      // head type
      for (i = 0; i < 16; ++i) {
            if (!readBuffer(placeHolder, 1)) { return false; }
            nodes[i]._headType = placeHolder.toUnsignedInt();
            }

      // pitch
      for (i = 0; i < 16; ++i) {
            if (!readBuffer(placeHolder, 1)) { return false; }
            nodes[i]._pitch = placeHolder.toUnsignedInt();
            }

      // voice
      for (i = 0; i < 16; ++i) {
            if (!readBuffer(placeHolder, 1)) { return false; }
            nodes[i]._voice = placeHolder.toUnsignedInt();
            }

      for (i = 0; i < nodes.size(); ++i)
            oveTrack->addDrum(nodes[i]);

#if 0
      if (!jump(17)) { return false; }

       // voice 0 channel
      if (!readBuffer(placeHolder, 1)) { return false; }
      oveTrack->setChannel(placeHolder.toUnsignedInt());

      // to be continued. if anything important...
#endif

      return true;
      }

//---------------------------------------------------------
//   GroupParse
//---------------------------------------------------------

GroupParse::GroupParse(OveSong* ove)
   : BasicParse(ove)
      {
      for (int i = 0; i < int(_sizeChunks.size()); ++i)
            _sizeChunks[i] = nullptr;
      _sizeChunks.clear();
      }

GroupParse::~GroupParse()
      {
      for (int i = 0; i < int(_sizeChunks.size()); ++i)
            _sizeChunks[i] = nullptr;
      _sizeChunks.clear();
      }

//---------------------------------------------------------
//   PageGroupParse
//---------------------------------------------------------

PageGroupParse::PageGroupParse(OveSong* ove)
   : BasicParse(ove)
      {
      for (int i = 0; i < int(_pageChunks.size()); ++i)
            _pageChunks[i] = nullptr;
      _pageChunks.clear();
      }

PageGroupParse::~PageGroupParse()
      {
      for (int i = 0; i < int(_pageChunks.size()); ++i)
            _pageChunks[i] = nullptr;
      _pageChunks.clear();
      }

//---------------------------------------------------------
//   PageGroupParse::parse
//---------------------------------------------------------

bool PageGroupParse::parse()
      {
      if (_pageChunks.isEmpty())
            return false;

      int i;
      for (i = 0; i < _pageChunks.size(); ++i) {
            Page* page = new Page();
            _ove->addPage(page);

            if (!parsePage(_pageChunks[i], page))
                  return false;
            }

      return true;
      }

//---------------------------------------------------------
//   PageGroupParse::parsePage
//---------------------------------------------------------

bool PageGroupParse::parsePage(SizeChunk* chunk, Page* page)
      {
      Block placeHolder(2);
      StreamHandle handle(chunk->dataBlock()->data(), chunk->sizeBlock()->toSize());

      _handle = &handle;

      // begin line
      if (!readBuffer(placeHolder, 2)) { return false; }
      page->setBeginLine(placeHolder.toUnsignedInt());

      // line count
      if (!readBuffer(placeHolder, 2)) { return false; }
      page->setLineCount(placeHolder.toUnsignedInt());

      if (!jump(4)) { return false; }

      // staff interval
      if (!readBuffer(placeHolder, 2)) { return false; }
      page->setStaffInterval(placeHolder.toUnsignedInt());

      // line interval
      if (!readBuffer(placeHolder, 2)) { return false; }
      page->setLineInterval(placeHolder.toUnsignedInt());

      // staff inline interval
      if (!readBuffer(placeHolder, 2)) { return false; }
      page->setStaffInlineInterval(placeHolder.toUnsignedInt());

      // line bar count
      if (!readBuffer(placeHolder, 2)) { return false; }
      page->setLineBarCount(placeHolder.toUnsignedInt());

      // page line count
      if (!readBuffer(placeHolder, 2)) { return false; }
      page->setPageLineCount(placeHolder.toUnsignedInt());

      // left margin
      if (!readBuffer(placeHolder, 4)) { return false; }
      page->setLeftMargin(placeHolder.toUnsignedInt());

      // top margin
      if (!readBuffer(placeHolder, 4)) { return false; }
      page->setTopMargin(placeHolder.toUnsignedInt());

      // right margin
      if (!readBuffer(placeHolder, 4)) { return false; }
      page->setRightMargin(placeHolder.toUnsignedInt());

      // bottom margin
      if (!readBuffer(placeHolder, 4)) { return false; }
      page->setBottomMargin(placeHolder.toUnsignedInt());

      // page width
      if (!readBuffer(placeHolder, 4)) { return false; }
      page->setPageWidth(placeHolder.toUnsignedInt());

      // page height
      if (!readBuffer(placeHolder, 4)) { return false; }
      page->setPageHeight(placeHolder.toUnsignedInt());

      _handle = nullptr;

      return true;
      }

//---------------------------------------------------------
//   StaffCountGetter
//---------------------------------------------------------

StaffCountGetter::StaffCountGetter(OveSong* ove)
   : BasicParse(ove)
      {
      }

//---------------------------------------------------------
//   StaffCountGetter::staffCount
//---------------------------------------------------------

unsigned StaffCountGetter::staffCount(SizeChunk* chunk)
      {
      StreamHandle handle(chunk->dataBlock()->data(), chunk->sizeBlock()->toSize());
      Block placeHolder;

      _handle = &handle;

      if (!jump(6)) { return false; }

      // staff count
      if (!readBuffer(placeHolder, 2)) { return false; }
      return placeHolder.toUnsignedInt();
      }

//---------------------------------------------------------
//   LineGroupParse
//---------------------------------------------------------

LineGroupParse::LineGroupParse(OveSong* ove)
   : BasicParse(ove)
      {
      _chunk = nullptr;
      for (int i = 0; i < _lineChunks.size(); ++i)
            _lineChunks[i] = nullptr;
      _lineChunks.clear();
      for (int i = 0; i < _staffChunks.size(); ++i)
            _staffChunks[i] = nullptr;
      _staffChunks.clear();
      }

LineGroupParse::~LineGroupParse()
      {
      _chunk = nullptr;
      for (int i = 0; i < _lineChunks.size(); ++i)
            _lineChunks[i] = nullptr;
      _lineChunks.clear();
      for (int i = 0; i < _staffChunks.size(); ++i)
            _staffChunks[i] = nullptr;
      _staffChunks.clear();
      }

//---------------------------------------------------------
//   LineGroupParse::parse
//---------------------------------------------------------

bool LineGroupParse::parse()
      {
      if (_lineChunks.isEmpty() || _staffChunks.size() % _lineChunks.size() != 0)
            return false;

      int i;
      unsigned j;
      unsigned lineStaffCount = _staffChunks.size() / _lineChunks.size();

      for (i = 0; i < _lineChunks.size(); ++i) {
            Line* linePtr = new Line();

            _ove->addLine(linePtr);

            if (!parseLine(_lineChunks[i], linePtr))
                  return false;

            for ( j= lineStaffCount * i; j < lineStaffCount * (i + 1); ++j) {
                  Staff* staffPtr = new Staff();

                  linePtr->addStaff(staffPtr);

                  if (!parseStaff(_staffChunks[j], staffPtr))
                        return false;
                  }
            }

      return true;
      }

//---------------------------------------------------------
//   LineGroupParse::parseLine
//---------------------------------------------------------

bool LineGroupParse::parseLine(SizeChunk* chunk, Line* line)
      {
      Block placeHolder;

      StreamHandle handle(chunk->dataBlock()->data(), chunk->sizeBlock()->toSize());

      _handle = &handle;

      if (!jump(2)) { return false; }

      // begin bar
      if (!readBuffer(placeHolder, 2)) { return false; }
      line->setBeginBar(placeHolder.toUnsignedInt());

      // bar count
      if (!readBuffer(placeHolder, 2)) { return false; }
      line->setBarCount(placeHolder.toUnsignedInt());

      if (!jump(6)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      line->setYOffset(placeHolder.toInt());

      // left x offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      line->setLeftXOffset(placeHolder.toInt());

      // right x offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      line->setRightXOffset(placeHolder.toInt());

      if (!jump(4)) { return false; }

      _handle = nullptr;

      return true;
      }

//---------------------------------------------------------
//   LineGroupParse::parseStaff
//---------------------------------------------------------

bool LineGroupParse::parseStaff(SizeChunk* chunk, Staff* staff)
      {
      Block placeHolder;

      StreamHandle handle(chunk->dataBlock()->data(), chunk->sizeBlock()->toSize());

      _handle = &handle;

      if (!jump(7)) { return false; }

      // clef
      if (!readBuffer(placeHolder, 1)) { return false; }
      staff->setClefType((ClefType)placeHolder.toUnsignedInt());

      // key
      if (!readBuffer(placeHolder, 1)) { return false; }
      staff->setKeyType(oveKeyToKey(placeHolder.toUnsignedInt()));

      if (!jump(2)) { return false; }

      // visible
      if (!readBuffer(placeHolder, 1)) { return false; }
      staff->setVisible(placeHolder.toBool());

      if (!jump(12)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      staff->setYOffset(placeHolder.toInt());

      int jumpAmount = _ove->isVersion4() ? 26 : 18;
      if (!jump(jumpAmount)) { return false; }

      // group type
      if (!readBuffer(placeHolder, 1)) { return false; }
      GroupType groupType = GroupType::None;
      if (placeHolder.toUnsignedInt() == 1)
            groupType = GroupType::Braces;
      else if (placeHolder.toUnsignedInt() == 2)
            groupType = GroupType::Brackets;
      staff->setGroupType(groupType);

      // group staff count
      if (!readBuffer(placeHolder, 1)) { return false; }
      staff->setGroupStaffCount(placeHolder.toUnsignedInt());

      _handle = nullptr;

      return true;
      }

//---------------------------------------------------------
//   BarsParse
//---------------------------------------------------------

BarsParse::BarsParse(OveSong* ove)
   : BasicParse(ove)
      {
      for (int i = 0; i < _measureChunks.size(); ++i)
            _measureChunks[i] = nullptr;
      _measureChunks.clear();
      for (int i = 0; i < _conductChunks.size(); ++i)
            _conductChunks[i] = nullptr;
      _conductChunks.clear();
      for (int i = 0; i < _bdatChunks.size(); ++i)
            _bdatChunks[i] = nullptr;
      _bdatChunks.clear();
      }

BarsParse::~BarsParse()
      {
      for (int i = 0; i < _measureChunks.size(); ++i)
            _measureChunks[i] = nullptr;
      _measureChunks.clear();
      for (int i = 0; i < _conductChunks.size(); ++i)
            _conductChunks[i] = nullptr;
      _conductChunks.clear();
      for (int i = 0; i < _bdatChunks.size(); ++i)
            _bdatChunks[i] = nullptr;
      _bdatChunks.clear();
      }

//---------------------------------------------------------
//   BarsParse::parse
//---------------------------------------------------------

bool BarsParse::parse()
      {
      int i;
      int trackMeasureCount = _ove->trackBarCount();
      int trackCount = _ove->trackCount();
      int measureDataCount = trackCount * _measureChunks.size();
      QList<Measure*> measures;
      QList<MeasureData*> measureDatas;

      if (_measureChunks.isEmpty()
         || _measureChunks.size() != _conductChunks.size()
         || _bdatChunks.size() != measureDataCount)
            return false;

      // add to ove
      for (i = 0; i < _measureChunks.size(); ++i) {
            Measure* measure = new Measure(i);

            measures.push_back(measure);
            _ove->addMeasure(measure);
            }

      for (i = 0; i < measureDataCount; ++i) {
            MeasureData* oveMeasureData = new MeasureData();

            measureDatas.push_back(oveMeasureData);
            _ove->addMeasureData(oveMeasureData);
            }

      for (i = 0; i < _measureChunks.size(); ++i) {
            Measure* measure = measures[i];

            // MEAS
            if (!parseMeas(measure, _measureChunks[i])) {
                  QString ss = QString("failed in parse MEAS %1\n").arg(i);
                  messageOut(ss);

                  return false;
                  }
            }

      for (i = 0; i < _conductChunks.size(); ++i) {
            // COND
            if (!parseCond(measures[i], measureDatas[i], _conductChunks[i])) {
                  QString ss = QString("failed in parse COND %1\n").arg(i);
                  messageOut(ss);

                  return false;
                  }
            }

      for (i = 0; i < _bdatChunks.size(); ++i) {
            int measId = i % trackMeasureCount;

            // BDAT
            if (!parseBdat(measures[measId], measureDatas[i], _bdatChunks[i])) {
                  QString ss = QString("failed in parse BDAT %1\n").arg(i);
                  messageOut(ss);

                  return false;
                  }

            if (_notify) {
                  int measureID = i % trackMeasureCount;
                  int trackID = i / trackMeasureCount;

                  // msg.msg_ = OVE_IMPORT_POS;
                  // msg.param1_ = (measureID<<16) + trackMeasureCount;
                  // msg.param2_ = (trackID<<16) + trackCount;

                  _notify->loadPosition(measureID, trackMeasureCount, trackID, trackCount);
                  }
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseMeas
//---------------------------------------------------------

bool BarsParse::parseMeas(Measure* measure, SizeChunk* chunk)
      {
      Block placeHolder;

      StreamHandle measureHandle(chunk->dataBlock()->data(), chunk->sizeBlock()->toSize());

      _handle = &measureHandle;

      if (!jump(2)) { return false; }

      // multi-measure rest
      if (!readBuffer(placeHolder, 1)) { return false; }
      measure->setIsMultiMeasureRest(placeHolder.toBool());

      // pickup
      if (!readBuffer(placeHolder, 1)) { return false; }
      measure->setIsPickup(placeHolder.toBool());

      if (!jump(4)) { return false; }

      // left barline
      if (!readBuffer(placeHolder, 1)) { return false; }
      measure->setLeftBarlineType(BarLineType(placeHolder.toUnsignedInt()));

      // right barline
      if (!readBuffer(placeHolder, 1)) { return false; }
      measure->setRightBarlineType(BarLineType(placeHolder.toUnsignedInt()));

      // tempo
      if (!readBuffer(placeHolder, 2)) { return false; }
      qreal tempo = qreal(placeHolder.toUnsignedInt());
      if (_ove->isVersion4())
            tempo /= 100.0;
      measure->setTypeTempo(tempo);

      // bar length(tick)
      if (!readBuffer(placeHolder, 2)) { return false; }
      measure->setLength(placeHolder.toUnsignedInt());

      if (!jump(6)) { return false; }

      // bar number offset
      if (!parseOffsetElement(measure->barNumber())) { return false; }

      if (!jump(2)) { return false; }

      // multi-measure rest count
      if (!readBuffer(placeHolder, 2)) { return false; }
      measure->setMultiMeasureRestCount(placeHolder.toUnsignedInt());

      _handle = nullptr;

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseCond
//---------------------------------------------------------

bool BarsParse::parseCond(Measure* measure, MeasureData* measureData, SizeChunk* chunk)
      {
      Block placeHolder;

      StreamHandle handle(chunk->dataBlock()->data(), chunk->sizeBlock()->toSize());

      _handle = &handle;

      // item count
      if (!readBuffer(placeHolder, 2)) { return false; }
      unsigned cnt = placeHolder.toUnsignedInt();

      if (!parseTimeSignature(measure, 36)) { return false; }

      for (unsigned i = 0; i < cnt; ++i) {
            if (!readBuffer(placeHolder, 2)) { return false; }
            unsigned twoByte = placeHolder.toUnsignedInt();
            unsigned oldBlockSize = twoByte - 11;
            unsigned newBlockSize = twoByte - 7;

            // type id
            if (!readBuffer(placeHolder, 1)) { return false; }
            unsigned thisByte = placeHolder.toUnsignedInt();
            CondType type;

            if (!condElementType(thisByte, type)) { return false; }

            switch (type) {
                  case CondType::Bar_Number:
                        if (!parseBarNumber(measure, twoByte - 1)) { return false; }
                        break;
                  case CondType::Repeat:
                        if (!parseRepeatSymbol(measureData, oldBlockSize)) { return false; }
                        break;
                  case CondType::Numeric_Ending:
                        if (!parseNumericEnding(measureData, oldBlockSize)) { return false; }
                        break;
                  case CondType::Decorator:
                        if (!parseDecorator(measureData, newBlockSize)) { return false; }
                        break;
                  case CondType::Tempo:
                        if (!parseTempo(measureData, newBlockSize)) { return false; }
                        break;
                  case CondType::Text:
                        if (!parseText(measureData, newBlockSize)) { return false; }
                        break;
                  case CondType::Expression:
                        if (!parseExpression(measureData, newBlockSize)) { return false; }
                        break;
                  case CondType::_timeParameters:
                        if (!parseTimeSignatureParameters(measure, newBlockSize)) { return false; }
                        break;
                  case CondType::Barline_Parameters:
                        if (!parseBarlineParameters(measure, newBlockSize)) { return false; }
                        break;
                  default:
                        if (!jump(newBlockSize)) { return false; }
                        break;
                  }
            }

      _handle = nullptr;

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseTimeSignature
//---------------------------------------------------------

bool BarsParse::parseTimeSignature(Measure* measure, int /*length*/)
      {
      Block placeHolder;

      TimeSignature* timeSignature = measure->timeSig();

      // numerator
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setNumerator(placeHolder.toUnsignedInt());

      // denominator
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setDenominator(placeHolder.toUnsignedInt());

      if (!jump(2)) { return false; }

      // beat length
      if (!readBuffer(placeHolder, 2)) { return false; }
      timeSignature->setBeatLength(placeHolder.toUnsignedInt());

      // bar length
      if (!readBuffer(placeHolder, 2)) { return false; }
      timeSignature->setBarLength(placeHolder.toUnsignedInt());

      if (!jump(4)) { return false; }

      // is symbol
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setIsSymbol(placeHolder.toBool());

      if (!jump(1)) { return false; }

      // replace font
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setReplaceFont(placeHolder.toBool());

      // color
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setColor(placeHolder.toUnsignedInt());

      // show
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setShow(placeHolder.toBool());

      // show beat group
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setShowBeatGroup(placeHolder.toBool());

      if (!jump(6)) { return false; }

      // numerator 1, 2, 3
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setGroupNumerator1(placeHolder.toUnsignedInt());
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setGroupNumerator2(placeHolder.toUnsignedInt());
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setGroupNumerator3(placeHolder.toUnsignedInt());

      // denominator
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setGroupDenominator1(placeHolder.toUnsignedInt());
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setGroupDenominator2(placeHolder.toUnsignedInt());
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setGroupDenominator3(placeHolder.toUnsignedInt());

      // beam group 1~4
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setBeamGroup1(placeHolder.toUnsignedInt());
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setBeamGroup2(placeHolder.toUnsignedInt());
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setBeamGroup3(placeHolder.toUnsignedInt());
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->setBeamGroup4(placeHolder.toUnsignedInt());

      // beam 16th
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->set16thBeamCount(placeHolder.toUnsignedInt());

      // beam 32th
      if (!readBuffer(placeHolder, 1)) { return false; }
      timeSignature->set32thBeamCount(placeHolder.toUnsignedInt());

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseTimeSignatureParameters
//---------------------------------------------------------

bool BarsParse::parseTimeSignatureParameters(Measure* measure, int length)
      {
      Block placeHolder;
      TimeSignature* ts = measure->timeSig();

      int cursor = _ove->isVersion4() ? 10 : 8;
      if (!jump(cursor)) { return false; }

      // numerator
      if (!readBuffer(placeHolder, 1)) { return false; }
      unsigned numerator = placeHolder.toUnsignedInt();

      cursor = _ove->isVersion4() ? 11 : 9;
      if ((length - cursor) % 8 != 0 || (length - cursor) / 8 != int(numerator))
            return false;

      for (unsigned i = 0; i < numerator; ++i) {
            // beat start unit
            if (!readBuffer(placeHolder, 2)) { return false; }
            int beatStart = placeHolder.toUnsignedInt();

            // beat length unit
            if (!readBuffer(placeHolder, 2)) { return false; }
            int beatLength = placeHolder.toUnsignedInt();

            if (!jump(2)) { return false; }

            // beat start tick
            if (!readBuffer(placeHolder, 2)) { return false; }
            int beatStartTick = placeHolder.toUnsignedInt();

            ts->addBeat(beatStart, beatLength, beatStartTick);
            }

      ts->endAddBeat();

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseBarlineParameters
//---------------------------------------------------------

bool BarsParse::parseBarlineParameters(Measure* measure, int /*length*/)
      {
      Block placeHolder;

      int cursor = _ove->isVersion4() ? 12 : 10;
      if (!jump(cursor)) { return false; }

      // repeat count
      if (!readBuffer(placeHolder, 1)) { return false; }
      int repeatCount = placeHolder.toUnsignedInt();

      measure->setBackwardRepeatCount(repeatCount);

      if (!jump(6)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseNumericEnding
//---------------------------------------------------------

bool BarsParse::parseNumericEnding(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      NumericEnding* numeric = new NumericEnding();
      measureData->addCrossMeasureElement(numeric, true);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(numeric)) { return false; }

      if (!jump(6)) { return false; }

      // measure count
      if (!readBuffer(placeHolder, 2)) { return false; }
      //int offsetMeasure = placeHolder.toUnsignedInt() - 1;
      int offsetMeasure = placeHolder.toUnsignedInt();
      numeric->stop()->setMeasure(offsetMeasure);

      if (!jump(2)) { return false; }

      // left x offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      numeric->leftShoulder()->setXOffset(placeHolder.toInt());

      // height
      if (!readBuffer(placeHolder, 2)) { return false; }
      numeric->setHeight(placeHolder.toUnsignedInt());

      // left x offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      numeric->rightShoulder()->setXOffset(placeHolder.toInt());

      if (!jump(2)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      numeric->leftShoulder()->setYOffset(placeHolder.toInt());
      numeric->rightShoulder()->setYOffset(placeHolder.toInt());

      // number offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      numeric->numericHandle()->setXOffset(placeHolder.toInt());
      if (!readBuffer(placeHolder, 2)) { return false; }
      numeric->numericHandle()->setYOffset(placeHolder.toInt());

      if (!jump(6)) { return false; }

      // text size
      if (!readBuffer(placeHolder, 1)) { return false; }
      unsigned size = placeHolder.toUnsignedInt();

      // text : size maybe a huge value
      if (!readBuffer(placeHolder, size)) { return false; }
      numeric->setText(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));

      // fix for wedding march.ove
      if (size % 2 == 0)
            if (!jump(1)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseTempo
//---------------------------------------------------------

bool BarsParse::parseTempo(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;
      unsigned thisByte;

      Tempo* tempo = new Tempo();
      measureData->addMusicData(tempo);
      if (!jump(3))
            return false;
      // common
      if (!parseCommonBlock(tempo))
            return false;
      if (!readBuffer(placeHolder, 1))
            return false;
      thisByte = placeHolder.toUnsignedInt();
      // show tempo
      tempo->setShowMark((highNibble(thisByte) & 0x4) == 0x4);
      // show before text
      tempo->setShowBeforeText((highNibble(thisByte) & 0x8) == 0x8);
      // show parenthesis
      tempo->setShowParentheses((highNibble(thisByte) & 0x1) == 0x1);
      // left note type
      tempo->setLeftNoteType(NoteType(lowNibble(thisByte)));
      // left note dot
      tempo->setLeftNoteDot((highNibble(thisByte) & 0x2) == 0x2);
      if (!jump(1))  // dimension of the note symbol
            return false;
      if (_ove->isVersion4()) {
            if (!jump(2))
                  return false;
            // tempo
            if (!readBuffer(placeHolder, 2))
                  return false;
            tempo->setTypeTempo((qreal(placeHolder.toUnsignedInt())) / 100.0);
            }
      else {
            // tempo
            if (!readBuffer(placeHolder, 2))
                  return false;
            tempo->setTypeTempo(qreal(placeHolder.toUnsignedInt()));
            if (!jump(2))
                  return false;
            }
      // offset
      if (!parseOffsetElement(tempo))
            return false;
      if (!jump(16))
            return false;
      // 31 bytes left text
      if (!readBuffer(placeHolder, 31))
            return false;
      tempo->setLeftText(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if (!readBuffer(placeHolder, 1))
            return false;
      thisByte = placeHolder.toUnsignedInt();
      // swing eighth
      tempo->setSwingEighth((highNibble(thisByte) & 0x4) == 0x4);
      // right note dot
      tempo->setRightNoteDot((highNibble(thisByte) & 0x1) == 0x1);
      // compatibility with v3 files ?
      tempo->setRightSideType(static_cast<int>(highNibble(thisByte) & 0x2));
      // right note type
      tempo->setRightNoteType(NoteType(lowNibble(thisByte)));
      // right text
      if (_ove->isVersion4()) {
            if (!readBuffer(placeHolder, 31))
                  return false;
            tempo->setRightText(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));
            if (!readBuffer(placeHolder, 1))
                  return false;
            // 00 -> float; 01 -> notetype; 02 -> text; 03 -> integer(floor)
            tempo->setRightSideType(placeHolder.toInt());
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseBarNumber
//---------------------------------------------------------

bool BarsParse::parseBarNumber(Measure* measure, int /*length*/)
      {
      Block placeHolder;

      BarNumber* barNumber = measure->barNumber();

      if (!jump(2)) { return false; }

      // show on paragraph start
      if (!readBuffer(placeHolder, 1)) { return false; }
      barNumber->setShowOnParagraphStart(lowNibble(placeHolder.toUnsignedInt()) == 8);

      unsigned blankSize = _ove->isVersion4() ? 9 : 7;
      if (!jump(blankSize)) { return false; }

      // text align
      if (!readBuffer(placeHolder, 1)) { return false; }
      barNumber->setAlign(placeHolder.toUnsignedInt());

      if (!jump(4)) { return false; }

      // show flag
      if (!readBuffer(placeHolder, 1)) { return false; }
      barNumber->setShowFlag(placeHolder.toUnsignedInt());

      if (!jump(10)) { return false; }

      // bar range
      if (!readBuffer(placeHolder, 1)) { return false; }
      barNumber->setShowEveryBarCount(placeHolder.toUnsignedInt());

      // prefix
      if (!readBuffer(placeHolder, 2)) { return false; }
      barNumber->setPrefix(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if (!jump(18)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseText
//---------------------------------------------------------

bool BarsParse::parseText(MeasureData* measureData, int length)
      {
      Block placeHolder;

      Text* text = new Text();
      measureData->addMusicData(text);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(text)) { return false; }

      // type
      if (!readBuffer(placeHolder, 1)) { return false; }
      unsigned thisByte = placeHolder.toUnsignedInt();
      bool includeLineBreak = ((highNibble(thisByte)&0x2) != 0x2);
      unsigned id = lowNibble(thisByte);
      Text::TextType textType = Text::TextType::Rehearsal;

      if (id == 0)
            textType = Text::TextType::MeasureText;
      else if (id == 1)
            textType = Text::TextType::SystemText;
      else /*if (id == 2)*/
            textType = Text::TextType::Rehearsal;

      text->setTextType(textType);

      if (!jump(1)) { return false; }

      // x offset
      if (!readBuffer(placeHolder, 4)) { return false; }
      text->setXOffset(placeHolder.toInt());

      // y offset
      if (!readBuffer(placeHolder, 4)) { return false; }
      text->setYOffset(placeHolder.toInt());

      // width
      if (!readBuffer(placeHolder, 4)) { return false; }
      text->setWidth(placeHolder.toUnsignedInt());

      // height
      if (!readBuffer(placeHolder, 4)) { return false; }
      text->setHeight(placeHolder.toUnsignedInt());

      if (!jump(7)) { return false; }

      // horizontal margin
      if (!readBuffer(placeHolder, 1)) { return false; }
      text->setHorizontalMargin(placeHolder.toUnsignedInt());

      if (!jump(1)) { return false; }

      // vertical margin
      if (!readBuffer(placeHolder, 1)) { return false; }
      text->setVerticalMargin(placeHolder.toUnsignedInt());

      if (!jump(1)) { return false; }

      // line thick
      if (!readBuffer(placeHolder, 1)) { return false; }
      text->setLineThick(placeHolder.toUnsignedInt());

      if (!jump(2)) { return false; }

      // text size
      if (!readBuffer(placeHolder, 2)) { return false; }
      unsigned size = placeHolder.toUnsignedInt();

      // text string, maybe huge
      if (!readBuffer(placeHolder, size)) { return false; }
      text->setText(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if (!includeLineBreak)
            if (!jump(6)) { return false; }
      else {
            unsigned cursor = _ove->isVersion4() ? 43 : 41;
            cursor += size;

            // multi lines of text
            for (unsigned i = 0; i < 2; ++i) {
                  if (int(cursor) < length) {
                        // line parameters count
                        if (!readBuffer(placeHolder, 2)) { return false; }
                        unsigned lineCount = placeHolder.toUnsignedInt();

                        if (i == 0 && int(cursor + 2 + 8 * lineCount) > length)
                              return false;

                        if (i == 1 && int(cursor + 2 + 8 * lineCount) != length)
                              return false;

                        if (!jump(8 * lineCount)) { return false; }

                        cursor += 2 + 8 * lineCount;
                        }
                  }
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseRepeatSymbol
//---------------------------------------------------------

bool BarsParse::parseRepeatSymbol(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      RepeatSymbol* repeat = new RepeatSymbol();
      measureData->addMusicData(repeat);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(repeat)) { return false; }

      // RepeatType
      if (!readBuffer(placeHolder, 1)) { return false; }
      repeat->setRepeatType(RepeatType(placeHolder.toUnsignedInt()));

      if (!jump(13)) { return false; }

      // offset
      if (!parseOffsetElement(repeat)) { return false; }

      if (!jump(15)) { return false; }

      // size
      if (!readBuffer(placeHolder, 2)) { return false; }
      unsigned size = placeHolder.toUnsignedInt();

      // text, maybe huge
      if (!readBuffer(placeHolder, size)) { return false; }
      repeat->setText(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));

      // last 0
      if (size % 2 == 0)
            if (!jump(1)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseBdat
//---------------------------------------------------------

bool BarsParse::parseBdat(Measure* /*measure*/, MeasureData* measureData, SizeChunk* chunk)
      {
      Block placeHolder;
      StreamHandle handle(chunk->dataBlock()->data(), chunk->sizeBlock()->toSize());

      _handle = &handle;

      // parse here
      if (!readBuffer(placeHolder, 2)) { return false; }
      unsigned cnt = placeHolder.toUnsignedInt();

      for (unsigned i = 0; i < cnt; ++i) {
            // 0x0028 or 0x0016 or 0x002C
            if (!readBuffer(placeHolder, 2)) { return false; }
            unsigned count = placeHolder.toUnsignedInt() - 7;

            // type id
            if (!readBuffer(placeHolder, 1)) { return false; }
            unsigned thisByte = placeHolder.toUnsignedInt();
            BdatType type;

            if (!bdatElementType(thisByte, type)) { return false; }

            switch (type) {
                  case BdatType::Raw_Note:
                  case BdatType::Rest:
                  case BdatType::Note:
                        if (!parseNoteRest(measureData, count, type)) { return false; }
                        break;
                  case BdatType::Beam:
                        if (!parseBeam(measureData, count)) { return false; }
                        break;
                  case BdatType::Harmony:
                        if (!parseHarmony(measureData, count)) { return false; }
                        break;
                  case BdatType::Clef:
                        if (!parseClef(measureData, count)) { return false; }
                        break;
                  case BdatType::Dynamic:
                        if (!parseDynamic(measureData, count)) { return false; }
                        break;
                  case BdatType::Wedge:
                        if (!parseWedge(measureData, count)) { return false; }
                        break;
                  case BdatType::Glissando:
                        if (!parseGlissando(measureData, count)) { return false; }
                        break;
                  case BdatType::Decorator:
                        if (!parseDecorator(measureData, count)) { return false; }
                        break;
                  case BdatType::Key:
                        if (!parseKey(measureData, count)) { return false; }
                        break;
                  case BdatType::Lyric:
                        if (!parseLyric(measureData, count)) { return false; }
                        break;
                  case BdatType::Octave_Shift:
                        if (!parseOctaveShift(measureData, count)) { return false; }
                        break;
                  case BdatType::Slur:
                        if (!parseSlur(measureData, count)) { return false; }
                        break;
                  case BdatType::Text:
                        if (!parseText(measureData, count)) { return false; }
                        break;
                  case BdatType::Tie:
                        if (!parseTie(measureData, count)) { return false; }
                        break;
                  case BdatType::Tuplet:
                        if (!parseTuplet(measureData, count)) { return false; }
                        break;
                  case BdatType::Guitar_Bend:
                  case BdatType::Guitar_Barre:
                        if (!parseSizeBlock(count)) { return false; }
                        break;
                  case BdatType::Pedal:
                        if (!parsePedal(measureData, count)) { return false; }
                        break;
                  case BdatType::Bracket:
                        if (!parseBracket(measureData, count)) { return false; }
                        break;
                  case BdatType::Expression:
                        if (!parseExpression(measureData, count)) { return false; }
                        break;
                  case BdatType::Harp_Pedal:
                        if (!parseHarpPedal(measureData, count)) { return false; }
                        break;
                  case BdatType::Multi_Measure_Rest:
                        if (!parseMultiMeasureRest(measureData, count)) { return false; }
                        break;
                  case BdatType::Harmony_GuitarFrame:
                        if (!parseHarmonyGuitarFrame(measureData, count)) { return false; }
                        break;
                  case BdatType::Graphics_40:
                  case BdatType::Graphics_RoundRect:
                  case BdatType::Graphics_Rect:
                  case BdatType::Graphics_Round:
                  case BdatType::Graphics_Line:
                  case BdatType::Graphics_Curve:
                  case BdatType::Graphics_WedgeSymbol:
                        if (!parseSizeBlock(count)) { return false; }
                        break;
                  case BdatType::Midi_Controller:
                        if (!parseMidiController(measureData, count)) { return false; }
                        break;
                  case BdatType::Midi_Program_Change:
                        if (!parseMidiProgramChange(measureData, count)) { return false; }
                        break;
                  case BdatType::Midi_Channel_Pressure:
                        if (!parseMidiChannelPressure(measureData, count)) { return false; }
                        break;
                  case BdatType::Midi_Pitch_Wheel:
                        if (!parseMidiPitchWheel(measureData, count)) { return false; }
                        break;
                  default:
                        if (!jump(count)) { return false; }
                        break;
                  }

            // if i == count-1 then is bar end place holder
            }

      _handle = nullptr;

      return true;
      }

//---------------------------------------------------------
//   getInt
//---------------------------------------------------------

static int getInt(int byte, int bits)
      {
      int num = 0;

      if (bits > 0) {
            int factor = int(pow(2.0, bits - 1));
            num = (byte % (factor * 2));

            if ((byte & factor) == factor)
                  num -= factor * 2;
            }

      return num;
      }

//---------------------------------------------------------
//   parseNoteRest
//---------------------------------------------------------

bool BarsParse::parseNoteRest(MeasureData* measureData, int length, BdatType type)
      {
      NoteContainer* container = new NoteContainer();
      Block placeHolder;
      unsigned thisByte;

      measureData->addNoteContainer(container);
      measureData->addMusicData(container);

      // note|rest & grace
      container->setIsRest(type == BdatType::Rest);
      container->setIsRaw(type == BdatType::Raw_Note);

      if (!readBuffer(placeHolder, 2)) { return false; }
      thisByte = placeHolder.toUnsignedInt();
      container->setIsGrace(thisByte == 0x3C00);
      container->setIsCue(thisByte == 0x4B40 || thisByte == 0x3240);

      // show / hide
      if (!readBuffer(placeHolder, 1)) { return false; }
      thisByte = placeHolder.toUnsignedInt();
      container->setShow(lowNibble(thisByte) != 0x8);

      // voice
      container->setVoice(lowNibble(thisByte) & 0x7);

      // common
      if (!parseCommonBlock(container)) { return false; }

      // tuplet
      if (!readBuffer(placeHolder, 1)) { return false; }
      container->setTuplet(placeHolder.toUnsignedInt());

      // space
      if (!readBuffer(placeHolder, 1)) { return false; }
      container->setSpace(placeHolder.toUnsignedInt());

      // in beam
      if (!readBuffer(placeHolder, 1)) { return false; }
      thisByte = placeHolder.toUnsignedInt();
      bool inBeam = (highNibble(thisByte) & 0x1) == 0x1;
      container->setInBeam(inBeam);

      // grace NoteType
      container->setGraceNoteType(NoteType(highNibble(thisByte)));

      // dot
      container->setDot(lowNibble(thisByte)&0x03);

      // NoteType
      if (!readBuffer(placeHolder, 1)) { return false; }
      thisByte = placeHolder.toUnsignedInt();
      container->setNoteType((NoteType)lowNibble(thisByte));

      int cursor = 0;

      if (type == BdatType::Rest) {
            Note* restPtr = new Note();
            container->addNoteRest(restPtr);
            restPtr->setIsRest(true);

            // line
            if (!readBuffer(placeHolder, 1)) { return false; }
            restPtr->setLine(placeHolder.toInt());

            if (!jump(1)) { return false; }

            cursor = _ove->isVersion4() ? 16 : 14;
            }
      else // type == Bdat_Note || type == Bdat_Raw_Note
            {
            // stem up 0x80, stem down 0x00
            if (!readBuffer(placeHolder, 1)) { return false; }
            thisByte = placeHolder.toUnsignedInt();
            container->setUp((highNibble(thisByte) & 0x8) == 0x8);

            // stem length
            int stemOffset = thisByte%0x80;
            container->setStemLength(getInt(stemOffset, 7) + 7/*3.5 line span*/);

            // show stem 0x00, hide stem 0x40
            if (!readBuffer(placeHolder, 1)) { return false; }
            bool hideStem = (highNibble(thisByte) == 0x4);
            container->setShowStem(!hideStem);

            if (!jump(1)) { return false; }

            // note count
            if (!readBuffer(placeHolder, 1)) { return false; }
            unsigned noteCount = placeHolder.toUnsignedInt();
            unsigned i;

            // each note 16 bytes
            for (i = 0; i < noteCount; ++i) {
                  Note* notePtr = new Note();
                  notePtr->setIsRest(false);

                  container->addNoteRest(notePtr);

                  // note show / hide
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  notePtr->setShow((thisByte & 0x80) != 0x80);

                  // notehead type
                  notePtr->setHeadType(NoteHeadType(thisByte & 0x7f));

                  // tie pos
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  notePtr->setTiePos(TiePos(highNibble(thisByte)));

                  // offset staff, in {-1, 0, 1}
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  thisByte = lowNibble(placeHolder.toUnsignedInt());
                  int offsetStaff = 0;
                  if (thisByte == 1) { offsetStaff = 1; }
                  if (thisByte == 7) { offsetStaff = -1; }
                  notePtr->setOffsetStaff(offsetStaff);

                  // accidental
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  notePtr->setAccidental(AccidentalType(lowNibble(thisByte)));
                  // accidental 0: influenced by key, 4: influenced by previous accidental in measure
                  // bool notShow = (highNibble(thisByte) == 0) || (highNibble(thisByte) == 4);
                  bool notShow = !(highNibble(thisByte) & 0x1);
                  notePtr->setShowAccidental(!notShow);

                  if (!jump(1)) { return false; }

                  // line
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  notePtr->setLine(placeHolder.toInt());

                  if (!jump(1)) { return false; }

                  // note
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  unsigned note = placeHolder.toUnsignedInt();
                  notePtr->setNote(note);

                  // note on velocity
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  unsigned onVelocity = placeHolder.toUnsignedInt();
                  notePtr->setOnVelocity(onVelocity);

                  // note off velocity
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  unsigned offVelocity = placeHolder.toUnsignedInt();
                  notePtr->setOffVelocity(offVelocity);

                  if (!jump(2)) { return false; }

                  // length (tick)
                  if (!readBuffer(placeHolder, 2)) { return false; }
                  container->setLength(placeHolder.toUnsignedInt());

                  // offset tick
                  if (!readBuffer(placeHolder, 2)) { return false; }
                  notePtr->setOffsetTick(placeHolder.toInt());
                  }

            cursor = _ove->isVersion4() ? 18 : 16;
            cursor += noteCount * 16 /* note size */;
            }

      // articulation
      while (cursor < length + 1 /* 0x70 || 0x80 || 0x90 */) {
            Articulation* art = new Articulation();
            container->addArticulation(art);

            // block size
            if (!readBuffer(placeHolder, 2)) { return false; }
            int blockSize = placeHolder.toUnsignedInt();

            // articulation type
            if (!readBuffer(placeHolder, 1)) { return false; }
            art->setArticulationType(ArticulationType(placeHolder.toUnsignedInt()));

            // placement
            if (!readBuffer(placeHolder, 1)) { return false; }
            art->setPlacementAbove(placeHolder.toUnsignedInt() != 0x00); //0x00:below, 0x30:above

            // offset
            if (!parseOffsetElement(art)) { return false; }

            if (!_ove->isVersion4()) {
                  if (blockSize - 8 > 0)
                        if (!jump(blockSize-8)) { return false; }
                  } 
            else {
                  // setting
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  const bool changeSoundEffect = ((thisByte & 0x1) == 0x1);
                  const bool changeLength = ((thisByte & 0x2) == 0x2);
                  const bool changeVelocity = ((thisByte & 0x4) == 0x4);
                  //const bool changeExtraLength = ((thisByte & 0x20) == 0x20);

                  if (!jump(8)) { return false; }

                  // velocity type
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  if (changeVelocity)
                        art->setVelocityType(Articulation::VelocityType(thisByte));

                  if (!jump(14)) { return false; }

                  // sound effect
                  if (!readBuffer(placeHolder, 2)) { return false; }
                  int from = placeHolder.toInt();
                  if (!readBuffer(placeHolder, 2)) { return false; }
                  int to = placeHolder.toInt();
                  if (changeSoundEffect)
                        art->setSoundEffect(from, to);

                  if (!jump(1)) { return false; }

                  // length percentage
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  if (changeLength)
                        art->setLengthPercentage(placeHolder.toUnsignedInt());

                  // velocity
                  if (!readBuffer(placeHolder, 2)) { return false; }
                  if (changeVelocity)
                        art->setVelocity(placeHolder.toInt());

                  if (art->isTrill()) {
                        if (!jump(8)) { return false; }

                        // trill note length
                        if (!readBuffer(placeHolder, 1)) { return false; }
                        art->setTrillNoteLength(placeHolder.toUnsignedInt());

                        // trill rate
                        if (!readBuffer(placeHolder, 1)) { return false; }
                        thisByte = placeHolder.toUnsignedInt();
                        NoteType trillNoteType = NoteType::Note_Sixteen;
                        switch (highNibble(thisByte)) {
                              case 0:
                                    trillNoteType = NoteType::Note_None;
                                    break;
                              case 1:
                                    trillNoteType = NoteType::Note_Sixteen;
                                    break;
                              case 2:
                                    trillNoteType = NoteType::Note_32;
                                    break;
                              case 3:
                                    trillNoteType = NoteType::Note_64;
                                    break;
                              case 4:
                                    trillNoteType = NoteType::Note_128;
                                    break;
                              default:
                                    break;
                              }
                        art->setTrillRate(trillNoteType);

                        // accelerate type
                        art->setAccelerateType(Articulation::AccelerateType(thisByte & 0xf));

                        if (!jump(1)) { return false; }

                        // auxiliary first
                        if (!readBuffer(placeHolder, 1)) { return false; }
                        art->setAuxiliaryFirst(placeHolder.toBool());

                        if (!jump(1)) { return false; }

                        // trill interval
                        if (!readBuffer(placeHolder, 1)) { return false; }
                        art->setTrillInterval(Articulation::TrillInterval(placeHolder.toUnsignedInt()));
                        } 
                  else {
                        if (blockSize > 40)
                              if (!jump(blockSize - 40)) { return false; }
                        }
                  }

            cursor += blockSize;
            }

      return true;
      }

//---------------------------------------------------------
//   tupletToSpace
//---------------------------------------------------------

static int tupletToSpace(int tuplet)
      {
      int a = 1;

      while (a * 2 < tuplet)
            a *= 2;

      return a;
      }

//---------------------------------------------------------
//   BarsParse::parseBeam
//---------------------------------------------------------

bool BarsParse::parseBeam(MeasureData* measureData, int length)
      {
      int i;
      Block placeHolder;

      Beam* beam = new Beam();
      measureData->addCrossMeasureElement(beam, true);

      // maybe create tuplet, for < quarter & tool 3(
      bool createTuplet = false;
      int maxEndUnit = 0;
      Tuplet* tuplet = new Tuplet();

      // is grace
      if (!readBuffer(placeHolder, 1)) { return false; }
      beam->setIsGrace(placeHolder.toBool());

      if (!jump(1)) { return false; }

      // voice
      if (!readBuffer(placeHolder, 1)) { return false; }
      beam->setVoice(lowNibble(placeHolder.toUnsignedInt())&0x7);

      // common
      if (!parseCommonBlock(beam)) { return false; }

      if (!jump(2)) { return false; }

      // beam count
      if (!readBuffer(placeHolder, 1)) { return false; }
      int beamCount = placeHolder.toUnsignedInt();

      if (!jump(1)) { return false; }

      // left line
      if (!readBuffer(placeHolder, 1)) { return false; }
      beam->leftLine()->setLine(placeHolder.toInt());

      // right line
      if (!readBuffer(placeHolder, 1)) { return false; }
      beam->rightLine()->setLine(placeHolder.toInt());

      if (_ove->isVersion4())
            if (!jump(8)) { return false; }

      int currentCursor = _ove->isVersion4() ? 23 : 13;
      int count = (length - currentCursor)/16;

      if (count != beamCount) { return false; }

      for (i = 0; i < count; ++i) {
            if (!jump(1)) { return false; }

            // tuplet
            if (!readBuffer(placeHolder, 1)) { return false; }
            int tupletCount = placeHolder.toUnsignedInt();
            if (tupletCount > 0) {
                  createTuplet = true;
                  tuplet->setTuplet(tupletCount);
                  tuplet->setSpace(tupletToSpace(tupletCount));
                  }

            // start / stop measure
            // line i start end position
            MeasurePos startMp;
            MeasurePos stopMp;

            if (!readBuffer(placeHolder, 1)) { return false; }
            startMp.setMeasure(placeHolder.toUnsignedInt());
            if (!readBuffer(placeHolder, 1)) { return false; }
            stopMp.setMeasure(placeHolder.toUnsignedInt());

            if (!readBuffer(placeHolder, 2)) { return false; }
            startMp.setOffset(placeHolder.toInt());
            if (!readBuffer(placeHolder, 2)) { return false; }
            stopMp.setOffset(placeHolder.toInt());

            beam->addLine(startMp, stopMp);

            if (stopMp.offset() > maxEndUnit)
                  maxEndUnit = stopMp.offset();

            if (i == 0) {
                  if (!jump(4)) { return false; }

                  // left offset up+4, down-4
                  if (!readBuffer(placeHolder, 2)) { return false; }
                  beam->leftShoulder()->setYOffset(placeHolder.toInt());

                  // right offset up+4, down-4
                  if (!readBuffer(placeHolder, 2)) { return false; }
                  beam->rightShoulder()->setYOffset(placeHolder.toInt());
                  }
            else {
                  if (!jump(8)) { return false; }
                  }
            }

      const QList<QPair<MeasurePos, MeasurePos> > lines = beam->lines();
      MeasurePos offsetMp;

      for (i = 0; i < lines.size(); ++i) {
            if (lines[i].second > offsetMp)
                  offsetMp = lines[i].second;
            }

      beam->stop()->setMeasure(offsetMp.measure());
      beam->stop()->setOffset(offsetMp.offset());

      // a case that Tuplet block don't exist, and hide inside beam
      if (createTuplet) {
            tuplet->copyCommonBlock(*beam);
            tuplet->leftLine()->setLine(beam->leftLine()->line());
            tuplet->rightLine()->setLine(beam->rightLine()->line());
            tuplet->stop()->setMeasure(beam->stop()->measure());
            tuplet->stop()->setOffset(maxEndUnit);

            measureData->addCrossMeasureElement(tuplet, true);
            }
      else {
            delete tuplet;
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseTie
//---------------------------------------------------------

bool BarsParse::parseTie(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      Tie* tie = new Tie();
      measureData->addCrossMeasureElement(tie, true);

      if (!jump(3)) { return false; }

      // start common
      if (!parseCommonBlock(tie)) { return false; }

      if (!jump(1)) { return false; }

      // note
      if (!readBuffer(placeHolder, 1)) { return false; }
      tie->setNote(placeHolder.toUnsignedInt());

      // pair lines
      if (!parsePairLinesBlock(tie)) { return false; }

      // offset common
      if (!parseOffsetCommonBlock(tie)) { return false; }

      // left shoulder offset
      if (!parseOffsetElement(tie->leftShoulder())) { return false; }

      // right shoulder offset
      if (!parseOffsetElement(tie->rightShoulder())) { return false; }

      // height
      if (!readBuffer(placeHolder, 2)) { return false; }
      tie->setHeight(placeHolder.toUnsignedInt());

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseTuplet
//---------------------------------------------------------

bool BarsParse::parseTuplet(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      Tuplet* tuplet = new Tuplet();
      measureData->addCrossMeasureElement(tuplet, true);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(tuplet)) { return false; }

      if (!jump(2)) { return false; }

      // pair lines
      if (!parsePairLinesBlock(tuplet)) { return false; }

      // offset common
      if (!parseOffsetCommonBlock(tuplet)) { return false; }

      // left shoulder offset
      if (!parseOffsetElement(tuplet->leftShoulder())) { return false; }

      // right shoulder offset
      if (!parseOffsetElement(tuplet->rightShoulder())) { return false; }

      if (!jump(2)) { return false; }

      // height
      if (!readBuffer(placeHolder, 2)) { return false; }
      tuplet->setHeight(placeHolder.toUnsignedInt());

      // tuplet
      if (!readBuffer(placeHolder, 1)) { return false; }
      tuplet->setTuplet(placeHolder.toUnsignedInt());

      // space
      if (!readBuffer(placeHolder, 1)) { return false; }
      tuplet->setSpace(placeHolder.toUnsignedInt());

      // mark offset
      if (!parseOffsetElement(tuplet->markHandle())) { return false; }

      return true;
      }

//---------------------------------------------------------
//   binaryToHarmonyType
//---------------------------------------------------------

static QString binaryToHarmonyType(int bin)
      {
      QString type = "";
      switch (bin) {
            case 0x0005: type = "add9(no3)";       break;
            case 0x0009: type = "min(no5)";        break;
            case 0x0011: type = "(no5)";           break;
            case 0x0021: type = "sus(no5)";        break;
            case 0x0025: type = "24";              break;
            case 0x0029: type = "min4(no5)";       break;
            case 0x0049: type = "dim";             break;
            case 0x0051: type = "(b5)";            break;
            case 0x0055: type = "2#4(no5)";        break;
            case 0x0081: type = "(no3)";           break;
            case 0x0085: type = "2";               break;
            case 0x0089: type = "min";             break;
            case 0x008D: type = "min(add9)";       break;
            case 0x0091: type = "";                break;
            case 0x0093: type = "addb9";           break;
            case 0x0095: type = "add9";            break;
            case 0x00A1: type = "sus4";            break;
            case 0x00A5: type = "sus(add9)";       break;
            case 0x00A9: type = "min4";            break;
            case 0x00D5: type = "2#4";             break;
            case 0x0111: type = "aug";             break;
            case 0x0115: type = "aug(add9)";       break;
            case 0x0151: type = "(b5b6)";          break;
            case 0x0155: type = "+add9#11";        break;
            case 0x0189: type = "minb6";           break;
            case 0x018D: type = "min2b6";          break;
            case 0x0191: type = "(b6)";            break;
            case 0x0199: type = "(add9)b6";        break;
            case 0x0205: type = "26";              break;
            case 0x020D: type = "min69";           break;
            case 0x0211: type = "6";               break;
            case 0x0215: type = "69";              break;
            case 0x022D: type = "min69 11";        break;
            case 0x0249: type = "dim7";            break;
            case 0x0251: type = "6#11";            break;
            case 0x0255: type = "13#11";           break;
            case 0x0281: type = "6(no3)";          break;
            case 0x0285: type = "69(no3)";         break;
            case 0x0289: type = "min6";            break;
            case 0x028D: type = "min69";           break;
            case 0x0291: type = "6";               break;
            case 0x0293: type = "6b9";             break;
            case 0x0295: type = "69";              break;
            case 0x02AD: type = "min69 11";        break;
            case 0x02C5: type = "69#11(no3)";      break;
            case 0x02D5: type = "69#11";           break;
            case 0x040D: type = "min9(no5)";       break;
            case 0x0411: type = "7(no5)";          break;
            case 0x0413: type = "7b9";             break;
            case 0x0415: type = "9";               break;
            case 0x0419: type = "7#9";             break;
            case 0x041B: type = "7b9#9";           break;
            case 0x0421: type = "sus7";            break;
            case 0x0429: type = "min11";           break;
            case 0x042D: type = "min11";           break;
            case 0x0445: type = "9b5(no3)";        break;
            case 0x0449: type = "min7b5";          break;
            case 0x044D: type = "min9b5";          break;
            case 0x0451: type = "7b5";             break;
            case 0x0453: type = "7b9b5";           break;
            case 0x0455: type = "9b5";             break;
            case 0x045B: type = "7b5b9#9";         break;
            case 0x0461: type = "sus7b5";          break;
            case 0x0465: type = "sus9b5";          break;
            case 0x0469: type = "min11b5";         break;
            case 0x046D: type = "min11b5";         break;
            case 0x0481: type = "7(no3)";          break;
            case 0x0489: type = "min7";            break;
            case 0x048D: type = "min9";            break;
            case 0x0491: type = "7";               break;
            case 0x0493: type = "7b9";             break;
            case 0x0495: type = "9";               break;
            case 0x0499: type = "7#9";             break;
            case 0x049B: type = "7b9#9";           break;
            case 0x04A1: type = "sus7";            break;
            case 0x04A5: type = "sus9";            break;
            case 0x04A9: type = "min11";           break;
            case 0x04AD: type = "min11";           break;
            case 0x04B5: type = "11";              break;
            case 0x04D5: type = "9#11";            break;
            case 0x0509: type = "min7#5";          break;
            case 0x0511: type = "aug7";            break;
            case 0x0513: type = "7#5b9";           break;
            case 0x0515: type = "aug9";            break;
            case 0x0519: type = "7#5#9";           break;
            case 0x0529: type = "min11b13";        break;
            case 0x0533: type = "11b9#5";          break;
            case 0x0551: type = "aug7#11";         break;
            case 0x0553: type = "7b5b9b13";        break;
            case 0x0555: type = "aug9#11";         break;
            case 0x0559: type = "aug7#9#11";       break;
            case 0x0609: type = "min13";           break;
            case 0x0611: type = "13";              break;
            case 0x0613: type = "13b9";            break;
            case 0x0615: type = "13";              break;
            case 0x0619: type = "13#9";            break;
            case 0x061B: type = "13b9#9";          break;
            case 0x0621: type = "sus13";           break;
            case 0x062D: type = "min13(11)";       break;
            case 0x0633: type = "13b9add4";        break;
            case 0x0635: type = "13";              break;
            case 0x0645: type = "13#11(no3)";      break;
            case 0x0651: type = "13b5";            break;
            case 0x0653: type = "13b9#11";         break;
            case 0x0655: type = "13#11";           break;
            case 0x0659: type = "13#9b5";          break;
            case 0x065B: type = "13b5b9#9";        break;
            case 0x0685: type = "13(no3)";         break;
            case 0x068D: type = "min13";           break;
            case 0x0691: type = "13";              break;
            case 0x0693: type = "13b9";            break;
            case 0x0695: type = "13";              break;
            case 0x0699: type = "13#9";            break;
            case 0x06A5: type = "sus13";           break;
            case 0x06AD: type = "min13(11)";       break;
            case 0x06B5: type = "13";              break;
            case 0x06D5: type = "13#11";           break;
            case 0x0813: type = "maj7b9";          break;
            case 0x0851: type = "maj7#11";         break;
            case 0x0855: type = "maj9#11";         break;
            case 0x0881: type = "maj7(no3)";       break;
            case 0x0889: type = "min(\u266e7)";    break; // "min(<sym>accidentalNatural</sym>7)"
            case 0x088D: type = "min9(\u266e7)";   break; // "min9(<sym>accidentalNatural</sym>7)"
            case 0x0891: type = "maj7";            break;
            case 0x0895: type = "maj9";            break;
            case 0x08C9: type = "dim7(add maj 7)"; break;
            case 0x08D1: type = "maj7#11";         break;
            case 0x08D5: type = "maj9#11";         break;
            case 0x0911: type = "maj7#5";          break;
            case 0x0991: type = "maj7#5";          break;
            case 0x0995: type = "maj9#5";          break;
            case 0x0A0D: type = "min69(\u266e7)";  break; // "min69(<sym>accidentalNatural</sym>7)"
            case 0x0A11: type = "maj13";           break;
            case 0x0A15: type = "maj13";           break;
            case 0x0A51: type = "maj13#11";        break;
            case 0x0A55: type = "maj13#11";        break;
            case 0x0A85: type = "maj13(no3)";      break;
            case 0x0A89: type = "min13(\u266e7)";  break; // "min13(<sym>accidentalNatural</sym>7)"
            case 0x0A8D: type = "min69(\u266e7)";  break; // "min69(<sym>accidentalNatural</sym>7)"
            case 0x0A91: type = "maj13";           break;
            case 0x0A95: type = "maj13";           break;
            case 0x0AAD: type = "min13(\u266e7)";  break;   // "min13(<sym>accidentalNatural</sym>7)"
            case 0x0AD5: type = "maj13#11";        break;
            case 0x0B45: type = "maj13#5#11(no4)"; break;
            default:
                  qDebug("Unrecognized harmony type: %04X",bin);
                  type = "";
                  break;
            }
      return type;
      }

//---------------------------------------------------------
//   BarsParse::parseHarmony
//---------------------------------------------------------

bool BarsParse::parseHarmony(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      Harmony* harmony = new Harmony();
      measureData->addMusicData(harmony);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(harmony)) { return false; }

      // bass on bottom
      if (!readBuffer(placeHolder, 1)) { return false; }
      harmony->setBassOnBottom((highNibble(placeHolder.toUnsignedInt()) & 0x4));

      // root alteration
      switch (placeHolder.toUnsignedInt() & 0x18) {
            case 0:
                  harmony->setAlterRoot(0); // natural
                  break;
            case 16:
                  harmony->setAlterRoot(-1); // flat
                  break;
            case 8:
                  harmony->setAlterRoot(1); // sharp
                  break;
            default:
                  harmony->setAlterRoot(0);
                  break;
            }

      // bass alteration
      switch (placeHolder.toUnsignedInt() & 0x3) {
            case 0:
                  harmony->setAlterBass(0); // natural
                  break;
            case 2:
                  harmony->setAlterBass(-1); // flat
                  break;
            case 1:
                  harmony->setAlterBass(1); // sharp
                  break;
            default:
                  harmony->setAlterBass(0);
                  break;
            }

      // show bass
      bool useBass = placeHolder.toUnsignedInt() & 0x80;

      if (!jump(1)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      harmony->setYOffset(placeHolder.toInt());

      // harmony type
      if (!readBuffer(placeHolder, 2)) { return false; }
      harmony->setHarmonyType(binaryToHarmonyType(placeHolder.toUnsignedInt()));

      // root
      if (!readBuffer(placeHolder, 1)) { return false; }
      harmony->setRoot(placeHolder.toInt());

      // bass
      if (!readBuffer(placeHolder, 1)) { return false; }
      if (useBass)
            harmony->setBass(placeHolder.toInt());

      // angle
      if (!readBuffer(placeHolder, 2)) { return false; }
      harmony->setAngle(placeHolder.toInt());

      if (_ove->isVersion4()) {
            // length (tick)
            if (!readBuffer(placeHolder, 2)) { return false; }
            harmony->setLength(placeHolder.toUnsignedInt());

            if (!jump(4)) { return false; }
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseClef
//---------------------------------------------------------

bool BarsParse::parseClef(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      Clef* clef = new Clef();
      measureData->addMusicData(clef);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(clef)) { return false; }

      // clef type
      if (!readBuffer(placeHolder, 1)) { return false; }
      clef->setClefType((ClefType)placeHolder.toUnsignedInt());

      // line
      if (!readBuffer(placeHolder, 1)) { return false; }
      clef->setLine(placeHolder.toInt());

      if (!jump(2)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseLyric
//---------------------------------------------------------

bool BarsParse::parseLyric(MeasureData* measureData, int length)
      {
      Block placeHolder;

      Lyric* lyric = new Lyric();
      measureData->addMusicData(lyric);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(lyric)) { return false; }

      if (!jump(2)) { return false; }

      // offset
      if (!parseOffsetElement(lyric)) { return false; }

      if (!jump(7)) { return false; }

      // verse
      if (!readBuffer(placeHolder, 1)) { return false; }
      lyric->setVerse(placeHolder.toUnsignedInt());

      if (_ove->isVersion4()) {
            if (!jump(6)) { return false; }

            // lyric
            if (length > 29) {
                  if (!readBuffer(placeHolder, length-29)) { return false; }
                  lyric->setLyric(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));
                  }
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseSlur
//---------------------------------------------------------

bool BarsParse::parseSlur(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      Slur* slur = new Slur();
      measureData->addCrossMeasureElement(slur, true);

      if (!jump(2)) { return false; }

      // voice
      if (!readBuffer(placeHolder, 1)) { return false; }
      slur->setVoice(lowNibble(placeHolder.toUnsignedInt()) & 0x7);

      // common
      if (!parseCommonBlock(slur)) { return false; }

      // show on top
      if (!readBuffer(placeHolder, 1)) { return false; }
      slur->setShowOnTop(highNibble(placeHolder.toUnsignedInt()) == 0x8);

      if (!jump(1)) { return false; }

      // pair lines
      if (!parsePairLinesBlock(slur)) { return false; }

      // offset common
      if (!parseOffsetCommonBlock(slur)) { return false; }

      // handle 1
      if (!parseOffsetElement(slur->leftShoulder())) { return false; }

      // handle 4
      if (!parseOffsetElement(slur->rightShoulder())) { return false; }

      // handle 2
      if (!parseOffsetElement(slur->handle2())) { return false; }

      // handle 3
      if (!parseOffsetElement(slur->handle3())) { return false; }

      if (_ove->isVersion4()) {
            if (!jump(3)) { return false; }

            // note time percent
            if (!readBuffer(placeHolder, 1)) { return false; }
            slur->setNoteTimePercent(placeHolder.toUnsignedInt());

            if (!jump(36)) { return false; }
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseGlissando
//---------------------------------------------------------

bool BarsParse::parseGlissando(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      Glissando* glissando = new Glissando();
      measureData->addCrossMeasureElement(glissando, true);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(glissando)) { return false; }

      // straight or wavy
      if (!readBuffer(placeHolder, 1)) { return false; }
      unsigned thisByte = placeHolder.toUnsignedInt();
      glissando->setStraightWavy(highNibble(thisByte) == 4);

      if (!jump(1)) { return false; }

      // pair lines
      if (!parsePairLinesBlock(glissando)) { return false; }

      // offset common
      if (!parseOffsetCommonBlock(glissando)) { return false; }

      // left shoulder
      if (!parseOffsetElement(glissando->leftShoulder())) { return false; }

      // right shoulder
      if (!parseOffsetElement(glissando->rightShoulder())) { return false; }

      if (_ove->isVersion4()) {
            if (!jump(1)) { return false; }

            // line thick
            if (!readBuffer(placeHolder, 1)) { return false; }
            glissando->setLineThick(placeHolder.toUnsignedInt());

            if (!jump(12)) { return false; }

            // text 32 bytes
            if (!readBuffer(placeHolder, 32)) { return false; }
            glissando->setText(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));

            if (!jump(6)) { return false; }
            }

      return true;
      }

//---------------------------------------------------------
//   getDecoratorType
//---------------------------------------------------------

static bool getDecoratorType(unsigned thisByte, bool& measureRepeat, Decorator::DecoratorType& decoratorType,
   bool& singleRepeat, ArticulationType& artType)
      {
      measureRepeat = false;
      decoratorType = Decorator::DecoratorType::Articulation;
      singleRepeat = true;
      artType = ArticulationType::None;

      switch (thisByte) {
            case 0x00:
                  decoratorType = Decorator::DecoratorType::Dotted_Barline;
                  break;
            case 0x30:
                  artType = ArticulationType::Open_String;
                  break;
            case 0x31:
                  artType = ArticulationType::Finger_1;
                  break;
            case 0x32:
                  artType = ArticulationType::Finger_2;
                  break;
            case 0x33:
                  artType = ArticulationType::Finger_3;
                  break;
            case 0x34:
                  artType = ArticulationType::Finger_4;
                  break;
            case 0x35:
                  artType = ArticulationType::Finger_5;
                  break;
            case 0x6B:
                  artType = ArticulationType::Flat_Accidental_For_Trill;
                  break;
            case 0x6C:
                  artType = ArticulationType::Sharp_Accidental_For_Trill;
                  break;
            case 0x6D:
                  artType = ArticulationType::Natural_Accidental_For_Trill;
                  break;
            case 0x8D:
                  measureRepeat = true;
                  singleRepeat = true;
                  break;
            case 0x8E:
                  measureRepeat = true;
                  singleRepeat = false;
                  break;
            case 0xA0:
                  artType = ArticulationType::Minor_Trill;
                  break;
            case 0xA1:
                  artType = ArticulationType::Major_Trill;
                  break;
            case 0xA2:
                  artType = ArticulationType::Trill_Section;
                  break;
            case 0xA6:
                  artType = ArticulationType::Turn;
                  break;
            case 0xA8:
                  artType = ArticulationType::Tremolo_Eighth;
                  break;
            case 0xA9:
                  artType = ArticulationType::Tremolo_Sixteenth;
                  break;
            case 0xAA:
                  artType = ArticulationType::Tremolo_Thirty_Second;
                  break;
            case 0xAB:
                  artType = ArticulationType::Tremolo_Sixty_Fourth;
                  break;
            case 0xB2:
                  artType = ArticulationType::Fermata;
                  break;
            case 0xB3:
                  artType = ArticulationType::Fermata_Inverted;
                  break;
            case 0xB9:
                  artType = ArticulationType::Pause;
                  break;
            case 0xBA:
                  artType = ArticulationType::Grand_Pause;
                  break;
            case 0xC0:
                  artType = ArticulationType::Marcato;
                  break;
            case 0xC1:
                  artType = ArticulationType::Marcato_Dot;
                  break;
            case 0xC2:
                  artType = ArticulationType::SForzando;
                  break;
            case 0xC3:
                  artType = ArticulationType::SForzando_Dot;
                  break;
            case 0xC4:
                  artType = ArticulationType::SForzando_Inverted;
                  break;
            case 0xC5:
                  artType = ArticulationType::SForzando_Dot_Inverted;
                  break;
            case 0xC6:
                  artType = ArticulationType::Staccatissimo;
                  break;
            case 0xC7:
                  artType = ArticulationType::Staccato;
                  break;
            case 0xC8:
                  artType = ArticulationType::Tenuto;
                  break;
            case 0xC9:
                  artType = ArticulationType::Natural_Harmonic;
                  break;
            case 0xCA:
                  artType = ArticulationType::Artificial_Harmonic;
                  break;
            case 0xCB:
                  artType = ArticulationType::Plus_Sign;
                  break;
            case 0xCC:
                  artType = ArticulationType::Up_Bow;
                  break;
            case 0xCD:
                  artType = ArticulationType::Down_Bow;
                  break;
            case 0xCE:
                  artType = ArticulationType::Up_Bow_Inverted;
                  break;
            case 0xCF:
                  artType = ArticulationType::Down_Bow_Inverted;
                  break;
            case 0xD0:
                  artType = ArticulationType::Pedal_Down;
                  break;
            case 0xD1:
                  artType = ArticulationType::Pedal_Up;
                  break;
            case 0xD6:
                  artType = ArticulationType::Heavy_Attack;
                  break;
            case 0xD7:
                  artType = ArticulationType::Heavier_Attack;
                  break;
            default:
                  return false;
                  break;
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseDecorator
//---------------------------------------------------------

bool BarsParse::parseDecorator(MeasureData* measureData, int length)
      {
      Block placeHolder;
      MusicData* musicData = new MusicData();

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(musicData)) { return false; }

      if (!jump(2)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      musicData->setYOffset(placeHolder.toInt());

      if (!jump(2)) { return false; }

      // measure repeat | piano pedal | dotted barline | articulation
      if (!readBuffer(placeHolder, 1)) { return false; }
      unsigned thisByte = placeHolder.toUnsignedInt();

      Decorator::DecoratorType _decoratorType;
      bool isMeasureRepeat;
      bool isSingleRepeat = true;
      ArticulationType artType = ArticulationType::None;

      getDecoratorType(thisByte, isMeasureRepeat, _decoratorType, isSingleRepeat, artType);

      if (isMeasureRepeat) {
            MeasureRepeat* measureRepeat = new MeasureRepeat();
            measureData->addCrossMeasureElement(measureRepeat, true);

            measureRepeat->copyCommonBlock(*musicData);
            measureRepeat->setYOffset(musicData->yOffset());

            measureRepeat->setSingleRepeat(isSingleRepeat);
            }
      else {
            Decorator* decorator = new Decorator();
            measureData->addMusicData(decorator);

            decorator->copyCommonBlock(*musicData);
            decorator->setYOffset(musicData->yOffset());

            decorator->setDecoratorType(_decoratorType);
            decorator->setArticulationType(artType);
            }

      int cursor = _ove->isVersion4() ? 16 : 14;
      if (!jump(length-cursor)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseWedge
//---------------------------------------------------------

bool BarsParse::parseWedge(MeasureData* measureData, int length)
      {
      Block placeHolder;
      Wedge* wedge = new Wedge();

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(wedge)) { return false; }

      // wedge type
      if (!readBuffer(placeHolder, 1)) { return false; }
      WedgeType wedgeType = WedgeType::Crescendo_Line;
      bool wedgeOrExpression = true;
      unsigned highHalfByte = highNibble(placeHolder.toUnsignedInt());
      unsigned lowHalfByte = lowNibble(placeHolder.toUnsignedInt());

      switch (highHalfByte) {
            case 0x0:
                  wedgeType = WedgeType::Crescendo_Line;
                  wedgeOrExpression = true;
                  break;
            case 0x4:
                  wedgeType = WedgeType::Decrescendo_Line;
                  wedgeOrExpression = true;
                  break;
            case 0x6:
                  wedgeType = WedgeType::Decrescendo;
                  wedgeOrExpression = false;
                  break;
            case 0x2:
                  wedgeType = WedgeType::Crescendo;
                  wedgeOrExpression = false;
                  break;
            default:
                  break;
            }

      // 0xb | 0x8(ove3) , else 3, 0(ove3)
      if ((lowHalfByte & 0x8) == 0x8) {
            wedgeType = WedgeType::Double_Line;
            wedgeOrExpression = true;
            }

      if (!jump(1)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      wedge->setYOffset(placeHolder.toInt());

      // wedge
      if (wedgeOrExpression) {
            measureData->addCrossMeasureElement(wedge, true);
            wedge->setWedgeType(wedgeType);

            if (!jump(2)) { return false; }

            // height
            if (!readBuffer(placeHolder, 2)) { return false; }
            wedge->setHeight(placeHolder.toUnsignedInt());

            // offset common
            if (!parseOffsetCommonBlock(wedge)) { return false; }

            int cursor = _ove->isVersion4() ? 21 : 19;
            if (!jump(length-cursor)) { return false; }
            }
      // expression : cresc, decresc
      else {
            Expression* express = new Expression();
            measureData->addMusicData(express);

            express->copyCommonBlock(*wedge);
            express->setYOffset(wedge->yOffset());

            if (!jump(4)) { return false; }

            // offset common
            if (!parseOffsetCommonBlock(express)) { return false; }

            if (_ove->isVersion4()) {
                  if (!jump(18)) { return false; }

                  // words
                  if (length > 39) {
                        if (!readBuffer(placeHolder, length-39)) { return false; }
                        express->setText(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));
                        }
                  }
            else {
                  QString str = wedgeType == WedgeType::Crescendo ? "cresc" : "decresc";
                  express->setText(str);

                  if (!jump(8)) { return false; }
                  }
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseDynamic
//---------------------------------------------------------

bool BarsParse::parseDynamic(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      Dynamic* dynamic = new Dynamic();
      measureData->addMusicData(dynamic);

      if (!jump(1)) { return false; }

      // play
      if (!readBuffer(placeHolder, 1)) { return false; }
      dynamic->setPlay(highNibble(placeHolder.toUnsignedInt()) != 0x4);

      if (!jump(1)) { return false; }

      // common
      if (!parseCommonBlock(dynamic)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      dynamic->setYOffset(placeHolder.toInt());

      // dynamic type
      if (!readBuffer(placeHolder, 1)) { return false; }
      dynamic->setDynamicType(DynamicType(lowNibble(placeHolder.toUnsignedInt())));

      // velocity
      if (!readBuffer(placeHolder, 1)) { return false; }
      dynamic->setVelocity(placeHolder.toUnsignedInt());

      int cursor = _ove->isVersion4() ? 4 : 2;

      if (!jump(cursor)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseKey
//---------------------------------------------------------

bool BarsParse::parseKey(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;
      Key* key = measureData->key();
      int cursor = _ove->isVersion4() ? 9 : 7;

      if (!jump(cursor)) { return false; }

      // key
      if (!readBuffer(placeHolder, 1)) { return false; }
      key->setKey(oveKeyToKey(placeHolder.toUnsignedInt()));

      // previous key
      if (!readBuffer(placeHolder, 1)) { return false; }
      key->setPreviousKey(oveKeyToKey(placeHolder.toUnsignedInt()));

      if (!jump(3)) { return false; }

      // symbol count
      if (!readBuffer(placeHolder, 1)) { return false; }
      key->setSymbolCount(placeHolder.toUnsignedInt());

      if (!jump(4)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parsePedal
//---------------------------------------------------------

bool BarsParse::parsePedal(MeasureData* measureData, int length)
      {
      Block placeHolder;

      Pedal* pedal = new Pedal();
      //measureData->addMusicData(pedal); //can't remember why
      measureData->addCrossMeasureElement(pedal, true);

      if (!jump(1)) { return false; }

      // play
      if (!readBuffer(placeHolder, 1)) { return false; }
      pedal->setPlay(highNibble(placeHolder.toUnsignedInt()) != 4);

      if (!jump(1)) { return false; }

      // common
      if (!parseCommonBlock(pedal)) { return false; }

      if (!jump(2)) { return false; }

      // pair lines
      if (!parsePairLinesBlock(pedal)) { return false; }

      // offset common
      if (!parseOffsetCommonBlock(pedal)) { return false; }

      // left shoulder
      if (!parseOffsetElement(pedal->leftShoulder())) { return false; }

      // right shoulder
      if (!parseOffsetElement(pedal->rightShoulder())) { return false; }

      int cursor = _ove->isVersion4() ? 0x45 : 0x23;
      int blankCount = _ove->isVersion4() ? 42 : 10;

      pedal->setHalf(length > cursor);

      if (!jump(blankCount)) { return false; }

      if (length > cursor) {
            if (!jump(2)) { return false; }

            // handle x offset
            if (!readBuffer(placeHolder, 2)) { return false; }
            pedal->pedalHandle()->setXOffset(placeHolder.toInt());

            if (!jump(6)) { return false; }
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseBracket
//---------------------------------------------------------

bool BarsParse::parseBracket(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      Bracket* bracket = new Bracket();
      measureData->addMusicData(bracket);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(bracket)) { return false; }

      if (!jump(2)) { return false; }

      // pair lines
      if (!parsePairLinesBlock(bracket)) { return false; }

      if (!jump(4)) { return false; }

      // left shoulder
      if (!parseOffsetElement(bracket->leftShoulder())) { return false; }

      // right shoulder
      if (!parseOffsetElement(bracket->rightShoulder())) { return false; }

      // kuohao type
      if (!readBuffer(placeHolder, 1)) { return false; }
      bracket->setBracketType(BracketType(placeHolder.toUnsignedInt()));

      // height
      if (!readBuffer(placeHolder, 1)) { return false; }
      bracket->setHeight(placeHolder.toUnsignedInt());

      int jumpAmount = _ove->isVersion4() ? 40 : 8;
      if (!jump(jumpAmount)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseExpression
//---------------------------------------------------------

bool BarsParse::parseExpression(MeasureData* measureData, int length)
      {
      Block placeHolder;

      Expression* expression = new Expression();
      measureData->addMusicData(expression);

      if (!jump(3)) { return false; }

      // common00
      if (!parseCommonBlock(expression)) { return false; }

      if (!jump(2)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      expression->setYOffset(placeHolder.toInt());

      // range bar offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      //int barOffset = placeHolder.toUnsignedInt();

      if (!jump(10)) { return false; }

      // tempo 1
      if (!readBuffer(placeHolder, 2)) { return false; }
      //qreal tempo1 = ((qreal)placeHolder.toUnsignedInt()) / 100.0;

      // tempo 2
      if (!readBuffer(placeHolder, 2)) { return false; }
      //qreal tempo2 = ((qreal)placeHolder.toUnsignedInt()) / 100.0;

      if (!jump(6)) { return false; }

      // text
      int cursor = _ove->isVersion4() ? 35 : 33;
      if (length > cursor) {
            if (!readBuffer(placeHolder, length-cursor)) { return false; }
            expression->setText(_ove->codecString(placeHolder.fixedSizeBufferToStrByteArray()));
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseHarpPedal
//---------------------------------------------------------

bool BarsParse::parseHarpPedal(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      HarpPedal* harpPedal = new HarpPedal();
      measureData->addMusicData(harpPedal);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(harpPedal)) { return false; }

      if (!jump(2)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      harpPedal->setYOffset(placeHolder.toInt());

      // show type
      if (!readBuffer(placeHolder, 1)) { return false; }
      harpPedal->setType(HarpPedalType(placeHolder.toUnsignedInt()));

      // show char flag
      if (!readBuffer(placeHolder, 1)) { return false; }
      harpPedal->setShowCharFlag(placeHolder.toUnsignedInt());

      if (!jump(8)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseMultiMeasureRest
//---------------------------------------------------------

bool BarsParse::parseMultiMeasureRest(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder(2);
      MultiMeasureRest* measureRest = new MultiMeasureRest();
      measureData->addMusicData(measureRest);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(measureRest)) { return false; }

      if (!jump(6)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseHarmonyGuitarFrame
//---------------------------------------------------------

bool BarsParse::parseHarmonyGuitarFrame(MeasureData* measureData, int length)
      {
      Block placeHolder;

      Harmony* harmony = new Harmony();
      measureData->addMusicData(harmony);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(harmony)) { return false; }

      // root
      if (!readBuffer(placeHolder, 1)) { return false; }
      harmony->setRoot(placeHolder.toUnsignedInt());

      // type
      if (!readBuffer(placeHolder, 1)) { return false; }
      //harmony->setHarmonyType((HarmonyType)placeHolder.toUnsignedInt()); // TODO

      // bass
      if (!readBuffer(placeHolder, 1)) { return false; }
      harmony->setBass(placeHolder.toUnsignedInt());

      int jumpAmount = _ove->isVersion4() ? length - 12 : length - 10;
      if (!jump(jumpAmount)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   extractOctave
//---------------------------------------------------------

static void extractOctave(unsigned Bits, OctaveShiftType& octaveShiftType,
   QList<OctaveShiftPosition>& positions)
      {
      octaveShiftType = OctaveShiftType::OS_8;
      positions.clear();

      switch (Bits) {
            case 0x0:
                  octaveShiftType = OctaveShiftType::OS_8;
                  positions.push_back(OctaveShiftPosition::Continue);
                  break;
            case 0x1:
                  octaveShiftType = OctaveShiftType::OS_Minus_8;
                  positions.push_back(OctaveShiftPosition::Continue);
                  break;
            case 0x2:
                  octaveShiftType = OctaveShiftType::OS_15;
                  positions.push_back(OctaveShiftPosition::Continue);
                  break;
            case 0x3:
                  octaveShiftType = OctaveShiftType::OS_Minus_15;
                  positions.push_back(OctaveShiftPosition::Continue);
                  break;
            case 0x4:
                  octaveShiftType = OctaveShiftType::OS_8;
                  positions.push_back(OctaveShiftPosition::Stop);
                  break;
            case 0x5:
                  octaveShiftType = OctaveShiftType::OS_Minus_8;
                  positions.push_back(OctaveShiftPosition::Stop);
                  break;
            case 0x6:
                  octaveShiftType = OctaveShiftType::OS_15;
                  positions.push_back(OctaveShiftPosition::Stop);
                  break;
            case 0x7:
                  octaveShiftType = OctaveShiftType::OS_Minus_15;
                  positions.push_back(OctaveShiftPosition::Stop);
                  break;
            case 0x8:
                  octaveShiftType = OctaveShiftType::OS_8;
                  positions.push_back(OctaveShiftPosition::Start);
                  break;
            case 0x9:
                  octaveShiftType = OctaveShiftType::OS_Minus_8;
                  positions.push_back(OctaveShiftPosition::Start);
                  break;
            case 0xA:
                  octaveShiftType = OctaveShiftType::OS_15;
                  positions.push_back(OctaveShiftPosition::Start);
                  break;
            case 0xB:
                  octaveShiftType = OctaveShiftType::OS_Minus_15;
                  positions.push_back(OctaveShiftPosition::Start);
                  break;
            case 0xC:
                  octaveShiftType = OctaveShiftType::OS_8;
                  positions.push_back(OctaveShiftPosition::Start);
                  positions.push_back(OctaveShiftPosition::Stop);
                  break;
            case 0xD:
                  octaveShiftType = OctaveShiftType::OS_Minus_8;
                  positions.push_back(OctaveShiftPosition::Start);
                  positions.push_back(OctaveShiftPosition::Stop);
                  break;
            case 0xE:
                  octaveShiftType = OctaveShiftType::OS_15;
                  positions.push_back(OctaveShiftPosition::Start);
                  positions.push_back(OctaveShiftPosition::Stop);
                  break;
            case 0xF:
                  octaveShiftType = OctaveShiftType::OS_Minus_15;
                  positions.push_back(OctaveShiftPosition::Start);
                  positions.push_back(OctaveShiftPosition::Stop);
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   BarsParse::parseOctaveShift
//---------------------------------------------------------

bool BarsParse::parseOctaveShift(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;

      OctaveShift* octave = new OctaveShift();
      measureData->addCrossMeasureElement(octave, true);

      if (!jump(3)) { return false; }

      // common
      if (!parseCommonBlock(octave)) { return false; }

      // octave
      if (!readBuffer(placeHolder, 1)) { return false; }
      unsigned type = lowNibble(placeHolder.toUnsignedInt());
      OctaveShiftType octaveShiftType = OctaveShiftType::OS_8;
      QList<OctaveShiftPosition> positions;
      extractOctave(type, octaveShiftType, positions);

      octave->setOctaveShiftType(octaveShiftType);

      if (!jump(1)) { return false; }

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      octave->setYOffset(placeHolder.toInt());

      if (!jump(4)) { return false; }

      // length
      if (!readBuffer(placeHolder, 2)) { return false; }
      octave->setLength(placeHolder.toUnsignedInt());

      // end tick
      if (!readBuffer(placeHolder, 2)) { return false; }
      octave->setEndTick(placeHolder.toUnsignedInt());

      // start & stop maybe appear in same measure
      for (int i = 0; i < positions.size(); ++i) {
            OctaveShiftPosition position = positions[i];
            OctaveShiftEndPoint* octavePoint = new OctaveShiftEndPoint();
            measureData->addMusicData(octavePoint);

            octavePoint->copyCommonBlock(*octave);
            octavePoint->setOctaveShiftType(octaveShiftType);
            octavePoint->setOctaveShiftPosition(position);
            octavePoint->setEndTick(octave->endTick());

            // stop
            if (i == 0 && position == OctaveShiftPosition::Stop)
                  octavePoint->start()->setOffset(octave->start()->offset() + octave->length());

            // end point
            if (i > 0) {
                  octavePoint->start()->setOffset(octave->start()->offset() + octave->length());
                  octavePoint->setTick(octave->endTick());
                  }
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseMidiController
//---------------------------------------------------------

bool BarsParse::parseMidiController(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;
      MidiController* controller = new MidiController();
      measureData->addMidiData(controller);

      parseMidiCommon(controller);

      // value [0, 128)
      if (!readBuffer(placeHolder, 1)) { return false; }
      controller->setValue(placeHolder.toUnsignedInt());

      // controller number
      if (!readBuffer(placeHolder, 1)) { return false; }
      controller->setController(placeHolder.toUnsignedInt());

      if (_ove->isVersion4())
            if (!jump(2)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseMidiProgramChange
//---------------------------------------------------------

bool BarsParse::parseMidiProgramChange(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;
      MidiProgramChange* program = new MidiProgramChange();
      measureData->addMidiData(program);

      parseMidiCommon(program);

      if (!jump(1)) { return false; }

      // patch
      if (!readBuffer(placeHolder, 1)) { return false; }
      program->setPatch(placeHolder.toUnsignedInt());

      if (_ove->isVersion4())
            if (!jump(2)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseMidiChannelPressure
//---------------------------------------------------------

bool BarsParse::parseMidiChannelPressure(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;
      MidiChannelPressure* pressure = new MidiChannelPressure();
      measureData->addMidiData(pressure);

      parseMidiCommon(pressure);

      if (!jump(1)) { return false; }

      // pressure
      if (!readBuffer(placeHolder, 1)) { return false; }
      pressure->setPressure(placeHolder.toUnsignedInt());

      if (_ove->isVersion4())
            if (!jump(2)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseMidiPitchWheel
//---------------------------------------------------------

bool BarsParse::parseMidiPitchWheel(MeasureData* measureData, int /*length*/)
      {
      Block placeHolder;
      MidiPitchWheel* wheel = new MidiPitchWheel();
      measureData->addMidiData(wheel);

      parseMidiCommon(wheel);

      // pitch wheel
      if (!readBuffer(placeHolder, 2)) { return false; }
      int value = placeHolder.toUnsignedInt();
      wheel->setValue(value);

      if (_ove->isVersion4())
            if (!jump(2)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseSizeBlock
//---------------------------------------------------------

bool BarsParse::parseSizeBlock(int length)
      {
      if (!jump(length)) { return false; }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseMidiCommon
//---------------------------------------------------------

bool BarsParse::parseMidiCommon(MidiData* md)
      {
      Block placeHolder;

      if (!jump(3)) { return false; }

      // start position
      if (!readBuffer(placeHolder, 2)) { return false; }
      md->setTick(placeHolder.toUnsignedInt());

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseCommonBlock
//---------------------------------------------------------

bool BarsParse::parseCommonBlock(MusicData* md)
      {
      Block placeHolder;

      // start tick
      if (!readBuffer(placeHolder, 2)) { return false; }
      md->setTick(placeHolder.toInt());

      // start unit
      if (!readBuffer(placeHolder, 2)) { return false; }
      md->start()->setOffset(placeHolder.toInt());

      if (_ove->isVersion4()) {
            // color
            if (!readBuffer(placeHolder, 1)) { return false; }
            md->setColor(placeHolder.toUnsignedInt());

            if (!jump(1)) { return false; }
            }

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseOffsetCommonBlock
//---------------------------------------------------------

bool BarsParse::parseOffsetCommonBlock(MusicData* md)
      {
      Block placeHolder;

      // offset measure
      if (!readBuffer(placeHolder, 2)) { return false; }
      md->stop()->setMeasure(placeHolder.toUnsignedInt());

      // end unit
      if (!readBuffer(placeHolder, 2)) { return false; }
      md->stop()->setOffset(placeHolder.toInt());

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parsePairLinesBlock
//---------------------------------------------------------

bool BarsParse::parsePairLinesBlock(PairEnds* ptr)
      {
      Block placeHolder;

      // left line
      if (!readBuffer(placeHolder, 2)) { return false; }
      ptr->leftLine()->setLine(placeHolder.toInt());

      // right line
      if (!readBuffer(placeHolder, 2)) { return false; }
      ptr->rightLine()->setLine(placeHolder.toInt());

      return true;
      }

//---------------------------------------------------------
//   BarsParse::parseOffsetElement
//---------------------------------------------------------

bool BarsParse::parseOffsetElement(OffsetElement* ptr)
      {
      Block placeHolder;

      // x offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      ptr->setXOffset(placeHolder.toInt());

      // y offset
      if (!readBuffer(placeHolder, 2)) { return false; }
      ptr->setYOffset(placeHolder.toInt());

      return true;
      }

//---------------------------------------------------------
//   BarsParse::condElementType
//---------------------------------------------------------

bool BarsParse::condElementType(unsigned byteData, CondType& type)
      {
      if (byteData == 0x09)
            type = CondType::_timeParameters;
      else if (byteData == 0x0A)
            type = CondType::Bar_Number;
      else if (byteData == 0x16)
            type = CondType::Decorator;
      else if (byteData == 0x1C)
            type = CondType::Tempo;
      else if (byteData == 0x1D)
            type = CondType::Text;
      else if (byteData == 0x25)
            type = CondType::Expression;
      else if (byteData == 0x30)
            type = CondType::Barline_Parameters;
      else if (byteData == 0x31)
            type = CondType::Repeat;
      else if (byteData == 0x32)
            type = CondType::Numeric_Ending;
      else
            return false;

      return true;
      }

//---------------------------------------------------------
//   BarsParse::bdatElementType
//---------------------------------------------------------

bool BarsParse::bdatElementType(unsigned byteData, BdatType& type)
      {
      if (byteData == 0x70)
            type = BdatType::Raw_Note;
      else if (byteData == 0x80)
            type = BdatType::Rest;
      else if (byteData == 0x90)
            type = BdatType::Note;
      else if (byteData == 0x10)
            type = BdatType::Beam;
      else if (byteData == 0x11)
            type = BdatType::Harmony;
      else if (byteData == 0x12)
            type = BdatType::Clef;
      else if (byteData == 0x13)
            type = BdatType::Wedge;
      else if (byteData == 0x14)
            type = BdatType::Dynamic;
      else if (byteData == 0x15)
            type = BdatType::Glissando;
      else if (byteData == 0x16)
            type = BdatType::Decorator;
      else if (byteData == 0x17)
            type = BdatType::Key;
      else if (byteData == 0x18)
            type = BdatType::Lyric;
      else if (byteData == 0x19)
            type = BdatType::Octave_Shift;
      else if (byteData == 0x1B)
            type = BdatType::Slur;
      else if (byteData == 0x1D)
            type = BdatType::Text;
      else if (byteData == 0x1E)
            type = BdatType::Tie;
      else if (byteData == 0x1F)
            type = BdatType::Tuplet;
      else if (byteData == 0x21)
            type = BdatType::Guitar_Bend;
      else if (byteData == 0x22)
            type = BdatType::Guitar_Barre;
      else if (byteData == 0x23)
            type = BdatType::Pedal;
      else if (byteData == 0x24)
            type = BdatType::Bracket;
      else if (byteData == 0x25)
            type = BdatType::Expression;
      else if (byteData == 0x26)
            type = BdatType::Harp_Pedal;
      else if (byteData == 0x27)
            type = BdatType::Multi_Measure_Rest;
      else if (byteData == 0x28)
            type = BdatType::Harmony_GuitarFrame;
      else if (byteData == 0x40)
            type = BdatType::Graphics_40;
      else if (byteData == 0x41)
            type = BdatType::Graphics_RoundRect;
      else if (byteData == 0x42)
            type = BdatType::Graphics_Rect;
      else if (byteData == 0x43)
            type = BdatType::Graphics_Round;
      else if (byteData == 0x44)
            type = BdatType::Graphics_Line;
      else if (byteData == 0x45)
            type = BdatType::Graphics_Curve;
      else if (byteData == 0x46)
            type = BdatType::Graphics_WedgeSymbol;
      else if (byteData == 0xAB)
            type = BdatType::Midi_Controller;
      else if (byteData == 0xAC)
            type = BdatType::Midi_Program_Change;
      else if (byteData == 0xAD)
            type = BdatType::Midi_Channel_Pressure;
      else if (byteData == 0xAE)
            type = BdatType::Midi_Pitch_Wheel;
      else if (byteData == 0xFF)
            type = BdatType::Bar_End;
      else
            return false;

      return true;
      }

//---------------------------------------------------------
//   LyricChunkParse
//---------------------------------------------------------

LyricChunkParse::LyricChunkParse(OveSong* ove)
   : BasicParse(ove)
      {
      _chunk = nullptr;
      }

LyricChunkParse::~LyricChunkParse()
      {
      _chunk = nullptr;
      }

//---------------------------------------------------------
//   LyricChunkParse::parse
//    Only ove3 has this chunk
//---------------------------------------------------------

bool LyricChunkParse::parse()
      {
      unsigned i;
      Block* dataBlock = _chunk->dataBlock();
      unsigned blockSize = _chunk->sizeBlock()->toSize();
      StreamHandle handle(dataBlock->data(), blockSize);
      Block placeHolder;

      _handle = &handle;

      if (!jump(4)) { return false; }

      // Lyric count
      if (!readBuffer(placeHolder, 2)) { return false; }
      unsigned count = placeHolder.toUnsignedInt();

      for (i = 0; i < count; ++i) {
            LyricInfo info;

            if (!readBuffer(placeHolder, 2)) { return false; }
            //unsigned size = placeHolder.toUnsignedInt();

            // 0x0D00
            if (!jump(2)) { return false; }

            // voice
            if (!readBuffer(placeHolder, 1)) { return false; }
            info._voice = placeHolder.toUnsignedInt();

            // verse
            if (!readBuffer(placeHolder, 1)) { return false; }
            info._verse = placeHolder.toUnsignedInt();

            // track
            if (!readBuffer(placeHolder, 1)) { return false; }
            info._track = placeHolder.toUnsignedInt();

            if (!jump(1)) { return false; }

            // measure
            if (!readBuffer(placeHolder, 2)) { return false; }
            info._measure = placeHolder.toUnsignedInt();

            // word count
            if (!readBuffer(placeHolder, 2)) { return false; }
            info._wordCount = placeHolder.toUnsignedInt();

            // lyric size
            if (!readBuffer(placeHolder, 2)) { return false; }
            info._lyricSize = placeHolder.toUnsignedInt();

            if (!jump(6)) { return false; }

            // name
            if (!readBuffer(placeHolder, 32)) { return false; }
            info._name = _ove->codecString(placeHolder.fixedSizeBufferToStrByteArray());

            if (info._lyricSize > 0) {
                  // lyric
                  if (info._lyricSize > 0) {
                        if (!readBuffer(placeHolder, info._lyricSize)) { return false; }
                        info._lyric = _ove->codecString(placeHolder.fixedSizeBufferToStrByteArray());
                        }

                  if (!jump(4)) { return false; }

                  // font
                  if (!readBuffer(placeHolder, 2)) { return false; }
                  info._font = placeHolder.toUnsignedInt();

                  if (!jump(1)) { return false; }

                  // font size
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  info._fontSize = placeHolder.toUnsignedInt();

                  // font style
                  if (!readBuffer(placeHolder, 1)) { return false; }
                  info._fontStyle = placeHolder.toUnsignedInt();

                  if (!jump(1)) { return false; }

                  for (int j = 0; j < info._wordCount; ++j)
                        if (!jump(8)) { return false; }
                  }

            processLyricInfo(info);
            }

      return true;
      }

#if 0
//---------------------------------------------------------
//   isSpace
//---------------------------------------------------------

static bool isSpace(char c)
      {
      return c == ' ' || c == '\n';
      }
#endif

//---------------------------------------------------------
//   LyricChunkParse::processLyricInfo
//---------------------------------------------------------

void LyricChunkParse::processLyricInfo(const LyricInfo& info)
      {
      int i;
      int j;
      int index = 0; // words

      int measureId = info._measure - 1;
      bool changeMeasure = true;
      MeasureData* measureData = 0;
      int trackMeasureCount = _ove->trackBarCount();
      QStringList words = info._lyric.split(" ", QString::SkipEmptyParts);

      while (index < words.size() && measureId + 1 < trackMeasureCount) {
            if (changeMeasure) {
                  ++measureId;
                  measureData = _ove->measureData(info._track, measureId);
                  changeMeasure = false;
                  }

            if (measureData == 0)
                  return;
            QList<NoteContainer*> containers = measureData->noteContainers();
            QList<MusicData*> lyrics = measureData->musicDatas(MusicDataType::Lyric);

            for (i = 0; i < containers.size() && index < words.size(); ++i) {
                  if (containers[i]->isRest())
                        continue;

                  for (j = 0; j < lyrics.size(); ++j) {
                        Lyric* lyric = static_cast<Lyric*>(lyrics[j]);

                        if (containers[i]->start()->offset() == lyric->start()->offset()
                           && int(containers[i]->voice()) == info._voice
                           && lyric->verse() == info._verse) {
                              if (index < words.size()) {
                                    QString l = words[index].trimmed();
                                    if (!l.isEmpty()) {
                                          lyric->setLyric(l);
                                          lyric->setVoice(info._voice);
                                          }
                                    }

                              ++index;
                              }
                        }
                  }

            changeMeasure = true;
            }
      }

//---------------------------------------------------------
//   TitleChunkParse
//---------------------------------------------------------

TitleChunkParse::TitleChunkParse(OveSong* ove)
   : BasicParse(ove)
      {
      _titleType     = 0x00000001;
      _annotateType  = 0x00010000;
      _writerType    = 0x00020002;
      _copyrightType = 0x00030001;
      _headerType    = 0x00040000;
      _footerType    = 0x00050002;

      _chunk = nullptr;
      }

TitleChunkParse::~TitleChunkParse()
      {
      _chunk = nullptr;
      }

//---------------------------------------------------------
//   byteArray
//---------------------------------------------------------

static QByteArray byteArray(const Block& block)
      {
      QByteArray array((char*)block.data(), block.size());
      int index0 = array.indexOf('\0');
      array = array.left(index0);

      return array;
      }

//---------------------------------------------------------
//   TitleChunkParse::parse
//---------------------------------------------------------

bool TitleChunkParse::parse()
      {
      Block* dataBlockP = _chunk->dataBlock();
      unsigned blockSize = _chunk->sizeBlock()->toSize();
      StreamHandle handle(dataBlockP->data(), blockSize);
      Block typeBlock;
      unsigned titleType;

      _handle = &handle;

      if (!readBuffer(typeBlock, 4)) { return false; }

      titleType = typeBlock.toUnsignedInt();

      if (titleType == _titleType || titleType == _annotateType
         || titleType == _writerType || titleType == _copyrightType) {
            Block offsetBlock;

            if (!readBuffer(offsetBlock, 4)) { return false; }

            const unsigned itemCount = 4;
            unsigned i;

            for (i = 0; i < itemCount; ++i) {
                  if (i > 0) {
                        // 0x 00 AB 00 0C 00 00
                        if (!jump(6)) { return false; }
                        }

                  Block countBlock;
                  if (!readBuffer(countBlock, 2)) { return false; }
                  unsigned titleSize = countBlock.toUnsignedInt();

                  Block dataBlock;
                  if (!readBuffer(dataBlock, titleSize)) { return false; }

                  QByteArray array = byteArray(dataBlock);
                  if (!array.isEmpty())
                        addToOve(_ove->codecString(array), titleType);
                  }

            return true;
            }

      if (titleType == _headerType || titleType == _footerType) {
            if (!jump(10)) { return false; }

            Block countBlock;
            if (!readBuffer(countBlock, 2)) { return false; }
            unsigned titleSize = countBlock.toUnsignedInt();

            Block dataBlock;
            if (!readBuffer(dataBlock, titleSize)) { return false; }

            QByteArray array = byteArray(dataBlock);
            addToOve(_ove->codecString(array), titleType);

            //0x 00 AB 00 0C 00 00
            if (!jump(6)) { return false; }

            return true;
            }

      return false;
      }

//---------------------------------------------------------
//   TitleChunkParse::addToOve
//---------------------------------------------------------

void TitleChunkParse::addToOve(const QString& str, unsigned titleType)
      {
      if (str.isEmpty()) { return; }

      if (titleType == _titleType)
            _ove->addTitle(str);

      if (titleType == _annotateType)
            _ove->addAnnotate(str);

      if (titleType == _writerType)
            _ove->addWriter(str);

      if (titleType == _copyrightType)
            _ove->addCopyright(str);

      if (titleType == _headerType)
            _ove->addHeader(str);

      if (titleType == _footerType)
            _ove->addFooter(str);
      }

//---------------------------------------------------------
//   OveOrganizer
//---------------------------------------------------------

OveOrganizer::OveOrganizer(OveSong* ove)
      {
      _ove = ove;
      }

//---------------------------------------------------------
//   OveOrganizer::organize
//---------------------------------------------------------

void OveOrganizer::organize()
      {
      if (!_ove)
            return;

      organizeTracks();
      organizeAttributes();
      organizeMeasures();
      }

//---------------------------------------------------------
//   OveOrganizer::organizeAttributes
//---------------------------------------------------------

void OveOrganizer::organizeAttributes()
      {
      int i;
      int j;
      int k;

      // key
      if (_ove->lineCount() > 0) {
            Line* line = _ove->line(0);
            int partBarCount = _ove->partBarCount();
            int lastKey = 0;

            if (line) {
                  for (i = 0; i < line->staffCount(); ++i) {
                        QPair<int, int> partStaff = _ove->trackToPartStaff(i);
                        Staff* staff = line->staff(i);
                        lastKey = staff->keyType();

                        for (j = 0; j < partBarCount; ++j) {
                              MeasureData* measureData = _ove->measureData(partStaff.first, partStaff.second, j);

                              if (measureData) {
                                    Key* key = measureData->key();

                                    if (j == 0) {
                                          key->setKey(lastKey);
                                          key->setPreviousKey(lastKey);
                                          }

                                    if (!key->setKey()) {
                                          key->setKey(lastKey);
                                          key->setPreviousKey(lastKey);
                                          }
                                    else {
                                          if (key->key() != lastKey)
                                                lastKey = key->key();
                                          }
                                    }
                              }
                        }
                  }
            }

      // clef
      if (_ove->lineCount() > 0) {
            Line* line = _ove->line(0);
            int partBarCount = _ove->partBarCount();
            ClefType lastClefType = ClefType::Treble;

            if (line) {
                  for (i = 0; i < line->staffCount(); ++i) {
                        QPair<int, int> partStaff = _ove->trackToPartStaff(i);
                        Staff* staff = line->staff(i);
                        lastClefType = staff->clefType();

                        for (j = 0; j < partBarCount; ++j) {
                              MeasureData* measureData = _ove->measureData(partStaff.first, partStaff.second, j);

                              if (measureData) {
                                    Clef* clefPtr = measureData->clef();
                                    clefPtr->setClefType(lastClefType);

                                    const QList<MusicData*>& clefs = measureData->musicDatas(MusicDataType::Clef);

                                    for (k = 0; k < clefs.size(); ++k) {
                                          Clef* clef = static_cast<Clef*>(clefs[k]);
                                          lastClefType = clef->clefType();
                                          }
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   getStaff
//---------------------------------------------------------

static Staff* getStaff(OveSong* ove, int track)
      {
      if (ove->lineCount() > 0) {
            Line* line = ove->line(0);
            if (line && line->staffCount() > 0) {
                  Staff* staff = line->staff(track);
                  return staff;
                  }
            }

      return nullptr;
      }

//---------------------------------------------------------
//   OveOrganizer::organizeTracks
//---------------------------------------------------------

void OveOrganizer::organizeTracks()
      {
      int i;
      // QList<QPair<ClefType, int>> trackChannels;
      QList<Track*> tracks = _ove->tracks();
      QList<bool> comboStaveStarts;

      for (i = 0; i < tracks.size(); ++i)
            comboStaveStarts.push_back(false);

      for (i = 0; i < tracks.size(); ++i) {
            Staff* s = getStaff(_ove, i);
            if (s) {
                  if (s->groupType() == GroupType::Braces && s->groupStaffCount() == 1)
                        comboStaveStarts[i] = true;
                  }
#if 0
            if (i < tracks.size() - 1) {
                  if (tracks[i]->startClef() == ClefType::Treble &&
                     tracks[i+1]->startClef() == ClefType::Bass &&
                     tracks[i]->channel() == tracks[i+1]->get_channel()) {
                        }
                  }
#endif
            }

      int trackId = 0;
      QList<int> partStaffCounts;

      while (trackId < tracks.size()) {
            int partTrackCount = 1;

            if (comboStaveStarts[trackId])
                  partTrackCount = 2;

            partStaffCounts.push_back(partTrackCount);
            trackId += partTrackCount;
            }

      _ove->addPartStaffCounts(partStaffCounts);
      }

//---------------------------------------------------------
//   OveOrganizer::organizeMeasures
//---------------------------------------------------------

void OveOrganizer::organizeMeasures()
      {
      int trackBarCount = _ove->trackBarCount();

      for (int i = 0; i < _ove->partCount(); ++i) {
            int partStaffCount = _ove->staffCount(i);

            for (int j = 0; j < partStaffCount; ++j) {
                  for (int k = 0; k < trackBarCount; ++k) {
                        Measure* measure = _ove->measure(k);
                        MeasureData* measureData = _ove->measureData(i, j, k);

                        organizeMeasure(i, j, measure, measureData);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   OveOrganizer::organizeMeasure
//---------------------------------------------------------

void OveOrganizer::organizeMeasure(int part, int track, Measure* measure, MeasureData* measureData)
      {
      // note containers
      organizeContainers(part, track, measure, measureData);

      // single end data
      organizeMusicDatas(part, track, measure, measureData);

      // cross measure elements
      organizeCrossMeasureElements(part, track, measure, measureData);
      }

//---------------------------------------------------------
//   addToList
//---------------------------------------------------------

static void addToList(QList<int>& list, int number)
      {
      for (int i = 0; i < list.size(); ++i) {
            if (list[i] == number)
                  return;
            }

      list.push_back(number);
      }

//---------------------------------------------------------
//   OveOrganizer::organizeContainers
//---------------------------------------------------------

void OveOrganizer::organizeContainers(int /*part*/, int /*track*/, Measure* measure, MeasureData* measureData)
      {
      int i;
      QList<NoteContainer*> containers = measureData->noteContainers();
      int barUnits = measure->timeSig()->units();
      QList<int> voices;

      for (i = 0; i < containers.size(); ++i) {
            int endUnit = barUnits;
            if (i < containers.size() - 1)
                  endUnit = containers[i+1]->start()->offset();

            containers[i]->stop()->setOffset(endUnit);
            addToList(voices, containers[i]->voice());
            }

      // shift voices
      qSort(voices.begin(), voices.end());

      for (i = 0; i < voices.size(); ++i) {
            int voice = voices[i];
            // voice -> i
            for (int j = 0; j < containers.size(); ++j) {
                  int avoice = containers[j]->voice();
                  if (avoice == voice && avoice != i)
                        containers[j]->setVoice(i);
                  }
            }
      }

//---------------------------------------------------------
//   OveOrganizer::organizeMusicDatas
//---------------------------------------------------------

void OveOrganizer::organizeMusicDatas(int /*part*/, int /*track*/, Measure* measure, MeasureData* measureData)
      {
      int i;
      int barIndex = measure->barNumber()->index();
      QList<MusicData*> datas = measureData->musicDatas(MusicDataType::None);

      for (i = 0; i < datas.size(); ++i)
            datas[i]->start()->setMeasure(barIndex);
      }

//---------------------------------------------------------
//   OveOrganizer::organizeCrossMeasureElements
//---------------------------------------------------------

void OveOrganizer::organizeCrossMeasureElements(int part, int track, Measure* measure, MeasureData* measureData)
      {
      int i;
      QList<MusicData*> pairs = measureData->crossMeasureElements(MusicDataType::None, MeasureData::PairType::Start);

      for (i = 0; i < pairs.size(); ++i) {
            MusicData* pair = pairs[i];

            switch (pair->musicDataType()) {
                  case MusicDataType::Beam:
                  case MusicDataType::Glissando:
                  case MusicDataType::Slur:
                  case MusicDataType::Tie:
                  case MusicDataType::Tuplet:
                  case MusicDataType::Pedal:
                  case MusicDataType::Numeric_Ending:
                  // case MusicDataType::OctaveShift_EndPoint:
                  case MusicDataType::Measure_Repeat:
                        organizePairElement(pair, part, track, measure, measureData);
                        break;
                  case MusicDataType::OctaveShift: {
                        OctaveShift* octave = static_cast<OctaveShift*>(pair);
                        organizeOctaveShift(octave, measure, measureData);
                        break;
                        }
                  case MusicDataType::Wedge: {
                        Wedge* wedge = static_cast<Wedge*>(pair);
                        organizeWedge(wedge, part, track, measure, measureData);
                        break;
                        }
                  default:
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   OveOrganizer::organizePairElement
//---------------------------------------------------------

void OveOrganizer::organizePairElement(MusicData* data, int part, int track,
   Measure* measure, MeasureData* measureData) 
      {
      int bar1Index = measure->barNumber()->index();
      int bar2Index = bar1Index + data->stop()->measure();
      MeasureData* measureData2 = _ove->measureData(part, track, bar2Index);

      data->start()->setMeasure(bar1Index);

      if (measureData2 && measureData != measureData2)
            measureData2->addCrossMeasureElement(data, false);

      if (data->musicDataType() == MusicDataType::Tuplet) {
            Tuplet* tuplet = static_cast<Tuplet*>(data);
            const QList<NoteContainer*> containers = measureData->noteContainers();

            for (int i = 0; i < containers.size(); ++i) {
                  if (containers[i]->tick() > tuplet->tick())
                        break;

                  else if (containers[i]->tick() == tuplet->tick())
                        tuplet->setNoteType(containers[i]->noteType());
                  }

            int tupletTick = noteTypeToTick(tuplet->noteType(), _ove->isQuarter()) * tuplet->space();
            if (tuplet->tick() % tupletTick != 0) {
                  int newStartTick = (tuplet->tick() / tupletTick) * tupletTick;

                  for (int i = 0; i < containers.size(); ++i) {
                        if (containers[i]->tick() == newStartTick &&
                            containers[i]->tuplet() == tuplet->tuplet()) {
                              tuplet->setTick(containers[i]->tick());
                              tuplet->start()->setOffset(containers[i]->start()->offset());
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   OveOrganizer::organizeOctaveShift
//---------------------------------------------------------

void OveOrganizer::organizeOctaveShift(OctaveShift* octave, Measure* measure, MeasureData* measureData)
      {
      // octave shift
      int i;
      const QList<NoteContainer*> containers = measureData->noteContainers();
      int barIndex = measure->barNumber()->index();

      octave->start()->setMeasure(barIndex);

      for (i = 0; i < containers.size(); ++i) {
            int noteShift = octave->noteShift();
            int containerTick = containers[i]->tick();

            if (octave->tick() <= containerTick && octave->endTick() > containerTick)
                  containers[i]->setNoteShift(noteShift);
            }
      }

//---------------------------------------------------------
//   getMiddleUnit
//---------------------------------------------------------

static bool getMiddleUnit(OveSong* ove, int /*part*/, int /*track*/,
   Measure* measure1, Measure* measure2, int unit1, int /*unit2*/,
   Measure* middleMeasure, int& middleUnit)
      {
      Q_UNUSED(middleMeasure); // avoid (bogus?) warning to show up
      QList<int> barUnits;
      int i;
      int bar1Index = measure1->barNumber()->index();
      int bar2Index = measure2->barNumber()->index();
      int sumUnit = 0;

      for (int j = bar1Index; j <= bar2Index; ++j) {
            Measure* measure = ove->measure(j);
            barUnits.push_back(measure->timeSig()->units());
            sumUnit += measure->timeSig()->units();
            }

      int currentSumUnit = 0;
      for (i = 0; i < barUnits.size(); ++i) {
            int barUnit = barUnits[i];

            if (i == 0)
                  barUnit = barUnits[i] - unit1;

            if (currentSumUnit + barUnit < sumUnit / 2)
                  currentSumUnit += barUnit;
            else
                  break;
            }

      if (i < barUnits.size()) {
            int barMiddleIndex = bar1Index + i;
            middleMeasure = ove->measure(barMiddleIndex);
            middleUnit = sumUnit / 2 - currentSumUnit;

            return true;
            }

      return false;
      }

//---------------------------------------------------------
//   OveOrganizer::organizeWedge
//---------------------------------------------------------

void OveOrganizer::organizeWedge(Wedge* wedge, int part, int track,
   Measure* measure, MeasureData* measureData)
      {
      int bar1Index = measure->barNumber()->index();
      int bar2Index = bar1Index + wedge->stop()->measure();
      MeasureData* measureData2 = _ove->measureData(part, track, bar2Index);
      WedgeType wedgeType = wedge->wedgeType();

      if (wedge->wedgeType() == WedgeType::Double_Line)
            wedgeType = WedgeType::Crescendo_Line;

      wedge->start()->setMeasure(bar1Index);

      WedgeEndPoint* startPoint = new WedgeEndPoint();
      measureData->addMusicData(startPoint);

      startPoint->setTick(wedge->tick());
      startPoint->start()->setOffset(wedge->start()->offset());
      startPoint->setWedgeStart(true);
      startPoint->setWedgeType(wedgeType);
      startPoint->setHeight(wedge->height());

      WedgeEndPoint* stopPoint = new WedgeEndPoint();

      stopPoint->setTick(wedge->tick());
      stopPoint->start()->setOffset(wedge->stop()->offset());
      stopPoint->setWedgeStart(false);
      stopPoint->setWedgeType(wedgeType);
      stopPoint->setHeight(wedge->height());

      if (measureData2)
            measureData2->addMusicData(stopPoint);

      if (wedge->wedgeType() == WedgeType::Double_Line) {
            Measure* middleMeasure = nullptr;
            int _middleUnit = 0;

            getMiddleUnit(_ove, part, track, measure,
               _ove->measure(bar2Index), wedge->start()->offset(), wedge->stop()->offset(), middleMeasure, _middleUnit);

            if (_middleUnit != 0) {
                  WedgeEndPoint* midStopPoint = new WedgeEndPoint();
                  measureData->addMusicData(midStopPoint);

                  midStopPoint->setTick(wedge->tick());
                  midStopPoint->start()->setOffset(_middleUnit);
                  midStopPoint->setWedgeStart(false);
                  midStopPoint->setWedgeType(WedgeType::Crescendo_Line);
                  midStopPoint->setHeight(wedge->height());

                  WedgeEndPoint* midStartPoint = new WedgeEndPoint();
                  measureData->addMusicData(midStartPoint);

                  midStartPoint->setTick(wedge->tick());
                  midStartPoint->start()->setOffset(_middleUnit);
                  midStartPoint->setWedgeStart(true);
                  midStartPoint->setWedgeType(WedgeType::Decrescendo_Line);
                  midStartPoint->setHeight(wedge->height());
                  }
            }
      }

//---------------------------------------------------------
//   ChunkType
//---------------------------------------------------------

enum class ChunkType : char {
      OVSC = 00,
      TRKL,
      TRAK,
      PAGL,
      PAGE,
      LINL,
      LINE,
      STAF,
      BARL,
      MEAS,
      COND,
      BDAT,
      PACH,
      FNTS,
      ODEV,
      TITL,
      ALOT,
      ENGR,
      FMAP,
      PCPR,

      // Overture 3.6
      LYRC,

      NONE
      };

//---------------------------------------------------------
//   nameToChunkType
//---------------------------------------------------------

static ChunkType nameToChunkType(const NameBlock& name)
      {
      ChunkType type = ChunkType::NONE;

      if (name.isEqual("OVSC"))
            type = ChunkType::OVSC;
      else if (name.isEqual("TRKL"))
            type = ChunkType::TRKL;
      else if (name.isEqual("TRAK"))
            type = ChunkType::TRAK;
      else if (name.isEqual("PAGL"))
            type = ChunkType::PAGL;
      else if (name.isEqual("PAGE"))
            type = ChunkType::PAGE;
      else if (name.isEqual("LINL"))
            type = ChunkType::LINL;
      else if (name.isEqual("LINE"))
            type = ChunkType::LINE;
      else if (name.isEqual("STAF"))
            type = ChunkType::STAF;
      else if (name.isEqual("BARL"))
            type = ChunkType::BARL;
      else if (name.isEqual("MEAS"))
            type = ChunkType::MEAS;
      else if (name.isEqual("COND"))
            type = ChunkType::COND;
      else if (name.isEqual("BDAT"))
            type = ChunkType::BDAT;
      else if (name.isEqual("PACH"))
            type = ChunkType::PACH;
      else if (name.isEqual("FNTS"))
            type = ChunkType::FNTS;
      else if (name.isEqual("ODEV"))
            type = ChunkType::ODEV;
      else if (name.isEqual("TITL"))
            type = ChunkType::TITL;
      else if (name.isEqual("ALOT"))
            type = ChunkType::ALOT;
      else if (name.isEqual("ENGR"))
            type = ChunkType::ENGR;
      else if (name.isEqual("FMAP"))
            type = ChunkType::FMAP;
      else if (name.isEqual("PCPR"))
            type = ChunkType::PCPR;
      else if (name.isEqual("LYRC"))
            type = ChunkType::LYRC;

      return type;
      }

//---------------------------------------------------------
//   chunkTypeToMaxTimes
//---------------------------------------------------------

static int chunkTypeToMaxTimes(ChunkType type)
      {
      int maxTimes = -1; // no limit

      switch (type) {
            case ChunkType::OVSC:
                  maxTimes = 1;
                  break;
            case ChunkType::TRKL:
            // case ChunkType::TRAK:
                  maxTimes = 1;
                  break;
            case ChunkType::PAGL:
            // case ChunkType::PAGE:
                  maxTimes = 1;
                  break;
            // case ChunkType::LINE:
            // case ChunkType::STAF:
            case ChunkType::LINL:
                  maxTimes = 1;
                  break;
            // case ChunkType::MEAS:
            // case ChunkType::COND:
            // case ChunkType::BDAT:
            case ChunkType::BARL:
                  maxTimes = 1;
                  break;
            case ChunkType::PACH:
            case ChunkType::FNTS:
            case ChunkType::ODEV:
            case ChunkType::ALOT:
            case ChunkType::ENGR:
            case ChunkType::FMAP:
            case ChunkType::PCPR:
                  maxTimes = 1;
                  break;
            case ChunkType::TITL:
                  maxTimes = 8;
                  break;
            case ChunkType::LYRC:
                  maxTimes = 1;
                  break;
            // case ChunkType::NONE:
            default:
                  break;
            }

      return maxTimes;
      }

//---------------------------------------------------------
//   OveSerialize
//---------------------------------------------------------

OveSerialize::OveSerialize()
      {
      _ove          = nullptr;
      _streamHandle = nullptr;
      _notify       = nullptr;
      }

OveSerialize::~OveSerialize()
      {
      _ove = nullptr;
      if (_streamHandle) {
            delete _streamHandle;
            _streamHandle = nullptr;
            }
      _notify = nullptr;
      }

//---------------------------------------------------------
//   OveSerialize::messageOutError
//---------------------------------------------------------

void OveSerialize::messageOutError()
      {
      if (_notify)
            _notify->loadError();
      }

//---------------------------------------------------------
//   OveSerialize::messageOut
//---------------------------------------------------------

void OveSerialize::messageOut(const QString& str)
      {
      if (_notify)
            _notify->loadInfo(str);
      }

//---------------------------------------------------------
//   OveSerialize::load
//---------------------------------------------------------

bool OveSerialize::load()
      {
      if (!_streamHandle)
            return false;

      if (!readHeader()) {
            messageOutError();
            return false;
            }

      unsigned i;
      QMap<ChunkType, int> chunkTimes;
      //bool firstEnter = true;

      for (i = int(ChunkType::OVSC); i < int(ChunkType::NONE); ++i)
            chunkTimes[ChunkType(i)] = 0;

      ChunkType chunkType = ChunkType::NONE;

      do {
            NameBlock nameBlock;
            SizeChunk sizeChunk;

            if (!readNameBlock(nameBlock)) { return false; }

            chunkType = nameToChunkType(nameBlock);
            ++chunkTimes[chunkType];
            int maxTime = chunkTypeToMaxTimes(chunkType);

            if (maxTime > 0 && chunkTimes[chunkType] > maxTime) {
                  messageOut("format not support, chunk appear more than accept.\n");
                  return false;
                  }

            switch (chunkType) {
#if 0
                  case ChunkType::OVSC:
                        if (!readHeadData(&sizeChunk)) {
                              messageOutError();
                              return false;
                              }
                        break;
#endif
                  case ChunkType::TRKL:
                        if (!readTrackData()) {
                              messageOutError();
                              return false;
                              }
                        break;
                  case ChunkType::PAGL:
                        if (!readPageData()) {
                              messageOutError();
                              return false;
                              }
                        break;
                  case ChunkType::LINL:
                        if (!readLineData()) {
                              messageOutError();
                              return false;
                              }
                        break;
                  case ChunkType::BARL:
                        if (!readBarData()) {
                              messageOutError();
                              return false;
                              }
                        break;
                  case ChunkType::TRAK:
                  case ChunkType::PAGE:
                  case ChunkType::LINE:
                  case ChunkType::STAF:
                  case ChunkType::MEAS:
                  case ChunkType::COND:
                  case ChunkType::BDAT:
                        return false;
                        break;
                  case ChunkType::LYRC: {
                        SizeChunk lyricChunk;
                        if (!readSizeChunk(&lyricChunk)) {
                              messageOutError();
                              return false;
                              }

                        LyricChunkParse parse(_ove);

                        parse.setLyricChunk(&lyricChunk);
                        parse.parse();

                        break;
                        }
                  case ChunkType::TITL: {
                        SizeChunk titleChunk;
                        if (!readSizeChunk(&titleChunk)) {
                              messageOutError();
                              return false;
                              }

                        TitleChunkParse titleChunkParse(_ove);

                        titleChunkParse.setTitleChunk(&titleChunk);
                        titleChunkParse.parse();

                        break;
                        }
                  case ChunkType::PACH:
                  case ChunkType::FNTS:
                  case ChunkType::ODEV:
                  case ChunkType::ALOT:
                  case ChunkType::ENGR:
                  case ChunkType::FMAP:
                  case ChunkType::PCPR:
                        if (!readSizeChunk(&sizeChunk)) {
                              messageOutError();
                              return false;
                              }
                        break;
                  default:
#if 0
                        if (firstEnter) {
                              QString info = "Not compatible file, try to load and save with newer version, Overture 4 is recommended.\n";
                              messageOut(info);
                              messageOutError();

                              return false;
                              }
#endif
                        break;
                  }

            // firstEnter = false;
            }

      while (chunkType != ChunkType::NONE); // ?

      // if (!readOveEnd()) { return false; }

      // organize OveData
      Ove::OveOrganizer organizer(_ove);
      organizer.organize();

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::release
//---------------------------------------------------------

void OveSerialize::release()
      {
      delete this;
      }

//---------------------------------------------------------
//   OveSerialize::readHeader
//---------------------------------------------------------

bool OveSerialize::readHeader()
      {
      ChunkType chunkType = ChunkType::NONE;
      NameBlock nameBlock;
      SizeChunk sizeChunk;

      if (!readNameBlock(nameBlock))
            return false;

      chunkType = nameToChunkType(nameBlock);
      // int maxTime = chunkTypeToMaxTimes(chunkType);

      if (chunkType == ChunkType::OVSC) {
            if (readHeadData(&sizeChunk))
                  return true;
            }

      QString info = "Not compatible file, try to load and save with newer version, Overture 4 is recommended.\n";
      messageOut(info);

      return false;
      }

//---------------------------------------------------------
//   OveSerialize::readHeadData
//---------------------------------------------------------

bool OveSerialize::readHeadData(SizeChunk* ovscChunk)
      {
      if (!readSizeChunk(ovscChunk))
            return false;

      OvscParse ovscParse(_ove);

      ovscParse.setNotify(_notify);
      ovscParse.setOvsc(ovscChunk);

      return ovscParse.parse();
      }

//---------------------------------------------------------
//   OveSerialize::readTrackData
//---------------------------------------------------------

bool OveSerialize::readTrackData()
      {
      GroupChunk trackGroupChunk;

      if (!readGroupChunk(&trackGroupChunk))
            return false;

      unsigned i;
      unsigned short trackCount = trackGroupChunk.countBlock()->toCount();

      for (i = 0; i < trackCount; ++i) {
            SizeChunk* trackChunk = new SizeChunk();

            if (_ove->isVersion4()) {
                  if (!readChunkName(trackChunk, Chunk::TrackName))
                        return false;
                  if (!readSizeChunk(trackChunk))
                        return false;
                  } 
            else {
                  if (!readDataChunk(trackChunk->dataBlock(), SizeChunk::version3TrackSize))
                        return false;
                  }

            TrackParse trackParse(_ove);

            trackParse.setTrack(trackChunk);
            trackParse.parse();
            }

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::readPageData
//---------------------------------------------------------

bool OveSerialize::readPageData()
      {
      GroupChunk pageGroupChunk;

      if (!readGroupChunk(&pageGroupChunk))
            return false;

      unsigned short pageCount = pageGroupChunk.countBlock()->toCount();
      unsigned i;
      PageGroupParse parse(_ove);

      for (i = 0; i < pageCount; ++i) {
            SizeChunk* pageChunk = new SizeChunk();

            if (!readChunkName(pageChunk, Chunk::PageName))
                  return false;
            if (!readSizeChunk(pageChunk))
                  return false;

            parse.addPage(pageChunk);
            }

      if (!parse.parse())
            return false;

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::readLineData
//---------------------------------------------------------

bool OveSerialize::readLineData()
      {
      GroupChunk lineGroupChunk;
      if (!readGroupChunk(&lineGroupChunk))
            return false;

      unsigned short lineCount = lineGroupChunk.countBlock()->toCount();
      int i;
      unsigned j;
      QList<SizeChunk*> lineChunks;
      QList<SizeChunk*> staffChunks;

      for (i = 0; i < lineCount; ++i) {
            SizeChunk* lineChunk = new SizeChunk();

            if (!readChunkName(lineChunk, Chunk::LineName))
                  return false;
            if (!readSizeChunk(lineChunk))
                  return false;

            lineChunks.push_back(lineChunk);

            StaffCountGetter getter(_ove);
            unsigned staffCount = getter.staffCount(lineChunk);

            for (j = 0; j < staffCount; ++j) {
                  SizeChunk* staffChunk = new SizeChunk();

                  if (!readChunkName(staffChunk, Chunk::StaffName))
                        return false;
                  if (!readSizeChunk(staffChunk))
                        return false;

                  staffChunks.push_back(staffChunk);
                  }
            }

      LineGroupParse parse(_ove);

      parse.setLineGroup(&lineGroupChunk);

      for (i = 0; i < lineChunks.size(); ++i)
            parse.addLine(lineChunks[i]);

      for (i = 0; i < staffChunks.size(); ++i)
            parse.addStaff(staffChunks[i]);

      if (!parse.parse())
            return false;

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::readBarData
//---------------------------------------------------------

bool OveSerialize::readBarData()
      {
      GroupChunk barGroupChunk;
      if (!readGroupChunk(&barGroupChunk))
            return false;

      unsigned short measCount = barGroupChunk.countBlock()->toCount();
      int i;

      QList<SizeChunk*> measureChunks;
      QList<SizeChunk*> conductChunks;
      QList<SizeChunk*> bdatChunks;

      _ove->setTrackBarCount(measCount);

      // read chunks
      for (i = 0; i < measCount; ++i) {
            SizeChunk* measureChunkPtr = new SizeChunk();

            if (!readChunkName(measureChunkPtr, Chunk::MeasureName))
                  return false;
            if (!readSizeChunk(measureChunkPtr))
                  return false;

            measureChunks.push_back(measureChunkPtr);
            }

      for (i = 0; i < measCount; ++i) {
            SizeChunk* conductChunkPtr = new SizeChunk();

            if (!readChunkName(conductChunkPtr, Chunk::ConductName))
                  return false;

            if (!readSizeChunk(conductChunkPtr))
                  return false;

            conductChunks.push_back(conductChunkPtr);
            }

      int bdatCount = _ove->trackCount() * measCount;
      for (i = 0; i < bdatCount; ++i) {
            SizeChunk* batChunkPtr = new SizeChunk();

            if (!readChunkName(batChunkPtr, Chunk::BdatName))
                  return false;
            if (!readSizeChunk(batChunkPtr))
                  return false;

            bdatChunks.push_back(batChunkPtr);
            }

      // parse bars
      BarsParse barsParse(_ove);

      for (i = 0; i < measureChunks.size(); ++i)
            barsParse.addMeasure(measureChunks[i]);

      for (i = 0; i < conductChunks.size(); ++i)
            barsParse.addConduct(conductChunks[i]);

      for (i = 0; i < bdatChunks.size(); ++i)
            barsParse.addBdat(bdatChunks[i]);

      barsParse.setNotify(_notify);
      if (!barsParse.parse()) {
            return false;
            }

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::readOveEnd
//---------------------------------------------------------

bool OveSerialize::readOveEnd()
      {
      if (!_streamHandle)
            return false;

      const unsigned END_OVE1 = 0xffffffff;
      const unsigned END_OVE2 = 0x00000000;
      unsigned buffer;

      if (!_streamHandle->read((char*) &buffer, sizeof(unsigned)))
            return false;

      if (buffer != END_OVE1)
            return false;

      if (!_streamHandle->read((char*) &buffer, sizeof(unsigned)))
            return false;

      if (buffer != END_OVE2)
            return false;

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::readNameBlock
//---------------------------------------------------------

bool OveSerialize::readNameBlock(NameBlock& nameBlock)
      {
      if (!_streamHandle)
            return false;

      if (!_streamHandle->read((char*) nameBlock.data(), nameBlock.size()))
            return false;

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::readChunkName
//---------------------------------------------------------

bool OveSerialize::readChunkName(Chunk* /*chunk*/, const QString& name)
      {
      if (!_streamHandle)
            return false;

      NameBlock nameBlock;

      if (!_streamHandle->read((char*) nameBlock.data(), nameBlock.size()))
            return false;

      if (!(nameBlock.toStrByteArray() == name))
            return false;

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::readSizeChunk
//---------------------------------------------------------

bool OveSerialize::readSizeChunk(SizeChunk* sizeChunk)
      {
      if (!_streamHandle)
            return false;

      SizeBlock* sizeBlock = sizeChunk->sizeBlock();

      if (!_streamHandle->read((char*) sizeBlock->data(), sizeBlock->size()))
            return false;

      unsigned blockSize = sizeBlock->toSize();

      sizeChunk->dataBlock()->resize(blockSize);

      Block* dataBlock = sizeChunk->dataBlock();

      if (!_streamHandle->read((char*) dataBlock->data(), blockSize))
            return false;

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::readDataChunk
//---------------------------------------------------------

bool OveSerialize::readDataChunk(Block* block, unsigned size)
      {
      if (!_streamHandle)
            return false;

      block->resize(size);

      if (!_streamHandle->read((char*) block->data(), size))
            return false;

      return true;
      }

//---------------------------------------------------------
//   OveSerialize::readGroupChunk
//---------------------------------------------------------

bool OveSerialize::readGroupChunk(GroupChunk* groupChunk)
      {
      if (!_streamHandle)
            return false;

      CountBlock* countBlock = groupChunk->countBlock();

      if (!_streamHandle->read((char*) countBlock->data(), countBlock->size()))
            return false;

      return true;
      }

//---------------------------------------------------------
//   createOveStreamLoader
//---------------------------------------------------------

IOVEStreamLoader* createOveStreamLoader()
      {
      return new OveSerialize;
      }

}
