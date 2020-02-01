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

#ifndef OVE_DATA_H
#define OVE_DATA_H

#ifdef WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

namespace Ove {

class OveSong;
class Track;
class Page;
class Voice;
class Line;
class Staff;
class Measure;
class MeasureData;
class MusicData;
class OffsetElement;
class LineElement;
class PairEnds;
class Note;
class NoteContainer;
class Beam;
class Tie;
class Tuplet;
class Harmony;
class Clef;
class Lyric;
class Slur;
class MeasureText;
class Articulation;
class Glissando;
class Decorator;
class MeasureRepeat;
class Dynamic;
class Wedge;
class WedgeEndPoint;
class Pedal;
class Bracket;
class Expression;
class HarpPedal;
class MultiMeasureRest;
class OctaveShift;
class OctaveShiftEndPoint;
class BarNumber;
class Tempo;
class Text;
class TimeSignature;
class Key;
class RepeatSymbol;
class NumericEnding;
class MidiData;
class MidiController;
class MidiProgramChange;
class MidiChannelPressure;
class MidiPitchWheel;

const int TWELVE_TONE = 12;
const int INVALID_NOTE = -1;
const int OCTAVE = 7;

//---------------------------------------------------------
//   enum classes
//---------------------------------------------------------

enum class CondType : char {
      None,
      _timeParameters    = 0x09, // size - 7, TimeSignature
      Bar_Number         = 0x0A, // size, compatible with previous version
      Decorator          = 0x16,
      Tempo              = 0x1C, // size - 7
      Text               = 0x1D, // size - 7, Rehearsal | SystemText
      Expression         = 0x25, // size - 7, if set playback parameters in Expression, will store in COND
      Barline_Parameters = 0x30, // size - 7, include :|| repeat count
      Repeat             = 0x31, //
      Numeric_Ending     = 0x32,
      };

enum class BdatType : unsigned char {
      None,
      Raw_Note              = 0x70,
      Rest                  = 0x80,
      Note                  = 0x90,
      Beam                  = 0x10,
      Harmony               = 0x11,
      Clef                  = 0x12,
      Dynamic               = 0x13,
      Wedge                 = 0x14, // cresendo, decresendo
      Glissando             = 0x15,
      Decorator             = 0x16, // measure repeat | piano pedal | dotted barline
      Key                   = 0x17,
      Lyric                 = 0x18,
      Octave_Shift          = 0x19,
      Slur                  = 0x1B,
      Text                  = 0x1D,
      Tie                   = 0x1E,
      Tuplet                = 0x1F,
      Guitar_Bend           = 0x21, //
      Guitar_Barre          = 0x22, //
      Pedal                 = 0x23,
      Bracket               = 0x24, // () [] {}
      Expression            = 0x25,
      Harp_Pedal            = 0x26,
      Multi_Measure_Rest    = 0x27,
      Harmony_GuitarFrame   = 0x28,
      Graphics_40           = 0x40, // unknown
      Graphics_RoundRect    = 0x41,
      Graphics_Rect         = 0x42,
      Graphics_Round        = 0x43,
      Graphics_Line         = 0x44,
      Graphics_Curve        = 0x45,
      Graphics_WedgeSymbol  = 0x46,
      Midi_Controller       = 0xAB,
      Midi_Program_Change   = 0xAC,
      Midi_Channel_Pressure = 0xAD,
      Midi_Pitch_Wheel      = 0xAE,
      Bar_End               = 0xFF,
      };

enum class MusicDataType : char {
      None,

      // attributes
      Clef,
      Key,
      Measure_Repeat,

      // sound
      Tempo,

      // direction
      Dynamic,
      Wedge,
      Wedge_EndPoint,
      OctaveShift,
      OctaveShift_EndPoint,
      Expression,
      Repeat,
      Text,
      Harp_Pedal,
      Pedal,

      // note & harmony
      Note_Container,
      Harmony,

      // note's children
      Beam,
      Glissando,
      Lyric,
      Slur,
      Tie,
      Tuplet,

      // barline
      Numeric_Ending,

      Bracket,
      Bar_End,
      Decorator,
      Multi_Measure_Rest,
      };

enum class MidiType : signed char {
      None = -1,
      Controller,
      Program_Change,
      Channel_Pressure,
      Pitch_Wheel,
      };

enum class ClefType : char {
      Treble = 0x00, //0x00
      Bass,          //0x01
      Alto,          //0x02
      UpAlto,        //0x03
      DownDownAlto,  //0x04
      DownAlto,      //0x05
      UpUpAlto,      //0x06
      Treble8va,     //0x07
      Bass8va,       //0x08
      Treble8vb,     //0x09
      Bass8vb,       //0x0A
      Percussion1,   //0x0B
      Percussion2,   //0x0C
      TAB            //0x0D
      };

enum class GroupType : char {
      None = 0,
      Braces,
      Brackets
      };

enum class AccidentalType : char {
      Normal              = 0x0,
      Sharp               = 0x1,
      Flat                = 0x2,
      Natural             = 0x3,
      DoubleSharp         = 0x4,
      DoubleFlat          = 0x5,
      Sharp_Caution       = 0x9,
      Flat_Caution        = 0xA,
      Natural_Caution     = 0xB,
      DoubleSharp_Caution = 0xC,
      DoubleFlat_Caution  = 0xD
      };

enum class NoteHeadType : char {
      Standard = 0x00,
      Invisible,
      Rhythmic_Slash,
      Percussion,
      Closed_Rhythm,
      Open_Rhythm,
      Closed_Slash,
      Open_Slash,
      Closed_Do,
      Open_Do,
      Closed_Re,
      Open_Re,
      Closed_Mi,
      Open_Mi,
      Closed_Fa,
      Open_Fa,
      Closed_Sol,
      Open_Sol,
      Closed_La,
      Open_La,
      Closed_Ti,
      Open_Ti
      };

enum class TiePos : char {
      None     = 0x0,
      LeftEnd  = 0x1,
      RightEnd = 0x2
      };

enum class ArticulationType : char {
      Major_Trill                  = 0x00,
      Minor_Trill                  = 0x01,
      Trill_Section                = 0x02,
      Inverted_Short_Mordent       = 0x03,
      Inverted_Long_Mordent        = 0x04,
      Short_Mordent                = 0x05,
      Turn                         = 0x06,
      Finger_1                     = 0x07,
      Finger_2                     = 0x08,
      Finger_3                     = 0x09,
      Finger_4                     = 0x0A,
      Finger_5                     = 0x0B,
      Flat_Accidental_For_Trill    = 0x0C,
      Sharp_Accidental_For_Trill   = 0x0D,
      Natural_Accidental_For_Trill = 0x0E,
      Marcato                      = 0x0F,
      Marcato_Dot                  = 0x10,
      Heavy_Attack                 = 0x11,
      SForzando                    = 0x12,
      SForzando_Dot                = 0x13,
      Heavier_Attack               = 0x14,
      SForzando_Inverted           = 0x15,
      SForzando_Dot_Inverted       = 0x16,
      Staccatissimo                = 0x17,
      Staccato                     = 0x18,
      Tenuto                       = 0x19,
      Up_Bow                       = 0x1A,
      Down_Bow                     = 0x1B,
      Up_Bow_Inverted              = 0x1C,
      Down_Bow_Inverted            = 0x1D,
      Arpeggio                     = 0x1E,
      Tremolo_Eighth               = 0x1F,
      Tremolo_Sixteenth            = 0x20,
      Tremolo_Thirty_Second        = 0x21,
      Tremolo_Sixty_Fourth         = 0x22,
      Natural_Harmonic             = 0x23,
      Artificial_Harmonic          = 0x24,
      Plus_Sign                    = 0x25,
      Fermata                      = 0x26,
      Fermata_Inverted             = 0x27,
      Pedal_Down                   = 0x28,
      Pedal_Up                     = 0x29,
      Pause                        = 0x2A,
      Grand_Pause                  = 0x2B,
      Toe_Pedal                    = 0x2C,
      Heel_Pedal                   = 0x2D,
      Toe_To_Heel_Pedal            = 0x2E,
      Heel_To_Toe_Pedal            = 0x2F,
      Open_String                  = 0x30, // finger 0 in guitar
      Guitar_Lift                  = 0x46,
      Guitar_Slide_Up              = 0x47,
      Guitar_Rip                   = 0x48,
      Guitar_Fall_Off              = 0x49,
      Guitar_Slide_Down            = 0x4A,
      Guitar_Spill                 = 0x4B,
      Guitar_Flip                  = 0x4C,
      Guitar_Smear                 = 0x4D,
      Guitar_Bend                  = 0x4E,
      Guitar_Doit                  = 0x4F,
      Guitar_Plop                  = 0x50,
      Guitar_Wow_Wow               = 0x51,
      Guitar_Thumb                 = 0x64,
      Guitar_Index_Finger          = 0x65,
      Guitar_Middle_Finger         = 0x66,
      Guitar_Ring_Finger           = 0x67,
      Guitar_Pinky_Finger          = 0x68,
      Guitar_Tap                   = 0x69,
      Guitar_Hammer                = 0x6A,
      Guitar_Pluck                 = 0x6B,

      None

      /* Detached_Legato,
      Spiccato,
      Scoop,
      Plop,
      Doit,
      Falloff,
      Breath_Mark,
      Caesura */
      };

enum class NoteType : char {
      Note_DoubleWhole = 0x0,
      Note_Whole       = 0x1,
      Note_Half        = 0x2,
      Note_Quarter     = 0x3,
      Note_Eight       = 0x4,
      Note_Sixteen     = 0x5,
      Note_32          = 0x6,
      Note_64          = 0x7,
      Note_128         = 0x8,
      Note_256         = 0x9,

      Note_None
      };

inline int noteTypeToTick(NoteType type, int quarter)
      {
      int c = int(pow(2.0, int(type)));
      return quarter * 4 * 2 / c;
      }

enum class DynamicType : char {
      PPPP = 0,
      PPP,
      PP,
      P,
      MP,
      MF,
      F,
      FF,
      FFF,
      FFFF,
      SF,
      FZ,
      SFZ,
      SFFZ,
      FP,
      SFP
      };

enum class WedgeType : char {
      Crescendo_Line = 0, // <
      Double_Line,        // <>, not appear in xml
      Decrescendo_Line,   // >
      Crescendo,          // cresc., not appear in xml, will create Expression
      Decrescendo         // decresc., not appear in xml, will create Expression
      };

enum class BracketType : char {
      Parentheses = 0,
      Braces,
      Brackets
      };

enum class HarpPedalType : char {
      Graph = 0,
      Char,
      Char_Cut,
      Change
      };

enum class OctaveShiftType : char {
      OS_8 = 0,
      OS_Minus_8,
      OS_15,
      OS_Minus_15
      };

enum class OctaveShiftPosition : char {
      Start = 0,
      Continue,
      Stop
      };

enum class RepeatType : char {
      Segno = 0,
      Coda,
      ToCoda,
      DSAlCoda,
      DSAlFine,
      DCAlCoda,
      DCAlFine,
      Fine,

      Null
      };

enum class BarLineType : char {
      Default = 0,  // 0x00 will be | or final (at last measure)
      Double,       // 0x01 ||
      Repeat_Left,  // 0x02 ||:
      Repeat_Right, // 0x03 :||
      Final,        // 0x04
      Dashed,       // 0x05
      Null          // 0x06
      };

enum class NoteDuration {
      D_256   = 15,
      D_128   = D_256 * 2,         // 30
      D_64    = D_128 * 2,         // 60
      D_32    = D_64 * 2,          // 120
      D_16    = D_32 * 2,          // 240
      D_8     = D_16 * 2,          // 480
      D_4     = D_8 * 2,           // 960
      D_2     = D_4 * 2,           // 1920
      D_Whole = D_2 * 2,           // 3840
      D_Double_Whole = D_Whole * 2 // 7680
      };

enum class ToneType : char {
      C = 0,
      D,
      E,
      F,
      G,
      A,
      B
      };

enum class KeyType : char {
      Key_C = 0,		// C
      Key_Bass_1,		// F
      Key_Bass_2,		// Bb
      Key_Bass_3,		// Eb
      Key_Bass_4,		// Ab
      Key_Bass_5,		// Db
      Key_Bass_6,		// Gb
      Key_Bass_7,		// Cb
      Key_Sharp_1,	// G
      Key_Sharp_2,	// D
      Key_Sharp_3,	// A
      Key_Sharp_4,	// E
      Key_Sharp_5,	// B
      Key_Sharp_6,	// F#
      Key_Sharp_7		// C#
      };

//---------------------------------------------------------
//   IOveNotify
//---------------------------------------------------------

class IOveNotify {
   public:
      IOveNotify() {}
      virtual ~IOveNotify() {}

      virtual void loadInfo(const QString& info) = 0;
      virtual void loadError() = 0;
      virtual void loadPosition(int currentMeasure, int totalMeasure, int currentTrack, int totalTrack) = 0;
      };

//---------------------------------------------------------
//   IOVEStreamLoader
//---------------------------------------------------------

class IOVEStreamLoader {
   public:
      IOVEStreamLoader() {}
      virtual ~IOVEStreamLoader() {}

      virtual void setNotify(IOveNotify* notify) = 0;
      virtual void setFileStream(unsigned char* buffer, unsigned size) = 0;
      virtual void setOve(OveSong* ove) = 0;

      // read stream, set read data to setOve(ove)
      virtual bool load() = 0;

      virtual void release() = 0;
      };

DLL_EXPORT IOVEStreamLoader* createOveStreamLoader();

//---------------------------------------------------------
//   TickElement
//---------------------------------------------------------

class TickElement {
      int _tick;

   public:
      TickElement();
      virtual ~TickElement() {}

      void setTick(int tick)  { _tick = tick; }
      int tick() const        { return _tick; }
      };

//---------------------------------------------------------
//   MeasurePos
//---------------------------------------------------------

class MeasurePos {
      int _measure;
      int _offset;

   public:
      MeasurePos();
      virtual ~MeasurePos() {}

      void setMeasure(int measure) { _measure = measure; }
      int measure() const          { return _measure;    }

      void setOffset(int offset)   { _offset = offset;   }
      int offset() const           { return _offset;     }

      MeasurePos shiftMeasure(int measure) const;
      MeasurePos shiftOffset(int offset) const; // ignore cross measure

      bool operator==(const MeasurePos& mp) const;
      bool operator!=(const MeasurePos& mp) const;
      bool operator<(const MeasurePos& mp) const;
      bool operator<=(const MeasurePos& mp) const;
      bool operator>(const MeasurePos& mp) const;
      bool operator>=(const MeasurePos& mp) const;
      };

//---------------------------------------------------------
//   PairElement
//---------------------------------------------------------

class PairElement {
      MeasurePos* _start;
      MeasurePos* _stop;

   public:
      PairElement();
      virtual ~PairElement();

      MeasurePos* start() const { return _start; }
      MeasurePos* stop() const  { return _stop;  }
      };

//---------------------------------------------------------
//   PairEnds
//---------------------------------------------------------

class PairEnds {
      LineElement* _leftLine;
      LineElement* _rightLine;
      OffsetElement* _leftShoulder;
      OffsetElement* _rightShoulder;

   public:
      PairEnds();
      virtual ~PairEnds();

      LineElement* leftLine() const        { return _leftLine;      }
      LineElement* rightLine() const       { return _rightLine;     }

      OffsetElement* leftShoulder() const  { return _leftShoulder;  }
      OffsetElement* rightShoulder() const { return _rightShoulder; }
      };

//---------------------------------------------------------
//   LineElement
//---------------------------------------------------------

class LineElement {
      int _line;

   public:
      LineElement();
      virtual ~LineElement() {}

      // middle line (3rd line of each clef) is set 0
      virtual void setLine(int line) { _line = line; }
      virtual int line() const       { return _line; }
      };

//---------------------------------------------------------
//   OffsetElement
//---------------------------------------------------------

class OffsetElement {
      int _xOffset;
      int _yOffset;

   public:
      OffsetElement();
      virtual ~OffsetElement() {}

      virtual void setXOffset(int offset) { _xOffset = offset; }
      virtual int xOffset() const         { return _xOffset;   }

      virtual void setYOffset(int offset) { _yOffset = offset; }
      virtual int yOffset() const         { return _yOffset;   }
      };

//---------------------------------------------------------
//   LengthElement
//---------------------------------------------------------

class LengthElement {
      int _length; // tick

   public:
      LengthElement();
      virtual ~LengthElement() {}

      void setLength(int length) { _length = length; }
      int length() const         { return _length;   }
      };

//---------------------------------------------------------
//   MusicData
//    Base class of many ove music element
//---------------------------------------------------------

class MusicData : public TickElement, public PairElement, public OffsetElement {
      bool _show;
      unsigned _color;
      unsigned _voice;

   protected:
      MusicDataType _musicDataType;

   public:
      MusicData();
      virtual ~MusicData() {}

      enum class XmlDataType : char {
            Attributes = 0, NoteBeam, Notations, Direction, None
            };
      static XmlDataType xmlDataType(MusicDataType type);
      // static bool isPairElement(MusicDataType type);

      void setShow(bool show)       { _show = show;   }
      bool show() const             { return _show;   }

      void setColor(unsigned color) { _color = color; } // not exists in ove 3
      unsigned color() const        { return _color;  }

      void setVoice(unsigned voice) { _voice = voice; }
      unsigned voice() const        { return _voice;  }

      MusicDataType musicDataType() const { return _musicDataType; }

      void copyCommonBlock(const MusicData& source);
      };

//---------------------------------------------------------
//   MidiData
//---------------------------------------------------------

class MidiData : public TickElement {
   protected:
      MidiType _midiType;

   public:
      MidiData();
      virtual ~MidiData() {}

      MidiType midiType() const { return _midiType; }
      };

//---------------------------------------------------------
//   OveSong
//---------------------------------------------------------

class OveSong {
      bool _version4;
      int _quarter;

      bool _showPageMargin;
      bool _showTransposeTrack;
      bool _showLineBreak;
      bool _showRuler;
      bool _showColor;
      bool _playRepeat;

      QList<QString> _titles;
      QList<QString> _annotates;
      QList<QString> _writers;
      QList<QString> _copyrights;
      QList<QString> _headers;
      QList<QString> _footers;

      QList<Track*> _tracks;
      QList<Page*> _pages;
      QList<Line*> _lines;
      QList<Measure*> _measures;
      QList<MeasureData*> _measureDatas;
      int _trackBarCount;	//equal to measures_.size()

      QList<int> _partStaffCounts;
      QTextCodec* _codec;

   public:
      OveSong();
      ~OveSong();

      void setIsVersion4(bool version4 = true) { _version4 = version4;        }
      bool isVersion4() const                  { return _version4;            }

      void setQuarter(int tick)                { _quarter = tick;             }
      int isQuarter() const                    { return _quarter;             }

      void setShowPageMargin(bool show)        { _showPageMargin = show;      }
      bool showPageMargin() const              { return _showPageMargin;      }

      void setShowTransposeTrack(bool show)    { _showTransposeTrack = show;  }
      bool showTransposeTrack() const          { return _showTransposeTrack;  }

      void setShowLineBreak(bool show)         { _showLineBreak = show;       }
      bool showLineBreak() const               { return _showLineBreak;       }
      
      void setShowRuler(bool show)             { _showRuler = show;           }
      bool showRuler() const                   { return _showRuler;           }

      void setShowColor(bool show)             { _showColor = show;           }
      bool showColor() const                   { return _showColor;           }

      void setPlayRepeat(bool play)            { _playRepeat = play;          }
      bool playRepeat() const                  { return _playRepeat;          }

      enum class PlayStyle : char {
            Record, Swing, Notation
            };

      void setPlayStyle(PlayStyle style)       { _playStyle = style;          }
      PlayStyle playStyle() const              { return _playStyle;           }

      void addTitle(const QString& str)        { _titles.push_back(str);      }
      QList<QString> titles() const            { return _titles;              }

      void addAnnotate(const QString& str)     { _annotates.push_back(str);   }
      QList<QString> annotates() const         { return _annotates;           }

      void addWriter(const QString& str)       { _writers.push_back(str);     }
      QList<QString> writers() const           { return _writers;             }

      void addCopyright(const QString& str)    { _copyrights.push_back(str);  }
      QList<QString> copyrights() const        { return _copyrights;          }

      void addHeader(const QString& str)       { _headers.push_back(str);     }
      QList<QString> headers() const           { return _headers;             }

      void addFooter(const QString& str)       { _footers.push_back(str);     }
      QList<QString> footers() const           { return _footers;             }

      void addTrack(Track* t)                  { _tracks.push_back(t);        }
      int trackCount() const                   { return _tracks.size();       }
      QList<Track*> tracks() const             { return _tracks;              }
      Track* track(int part, int staff) const;

      void setTrackBarCount(int count)         { _trackBarCount = count;      }
      int trackBarCount() const                { return _trackBarCount;       }

      void addPage(Page* page)                 { _pages.push_back(page);      }
      int pageCount() const                    { return _pages.size();        }
      Page* page(int idx) const;

      void addLine(Line* line)                 { _lines.push_back(line);      }
      int lineCount() const                    { return _lines.size();        }
      Line* line(int idx) const;

      void addMeasure(Measure* m)              { _measures.push_back(m);      }
      int measureCount() const                 { return _measures.size();     }
      Measure* measure(int m) const;

      void addMeasureData(MeasureData* md)     { _measureDatas.push_back(md); }
      int measureDataCount() const             { return _measureDatas.size(); }
      MeasureData* measureData(int part, int staff/*= 0*/, int bar) const;
      MeasureData* measureData(int track, int bar) const;

      // tool
      void addPartStaffCounts(const QList<int>& partStaffCounts);
      int partCount() const                    { return _partStaffCounts.size(); }
      int staffCount(int part) const;
      int partBarCount() const   { return _measureDatas.size() / _tracks.size(); }

      void clear();

      QPair<int, int> trackToPartStaff(int track) const;

      void setTextCodecName(const QString& codecName) { _codec = QTextCodec::codecForName(codecName.toLatin1()); }
      QString codecString(const QByteArray& text);

   private:
      PlayStyle _playStyle;

      int partStaffToTrack(int part, int staff) const;
      };

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

class Voice {
      int _channel;    // [0, 15]
      int _volume;     // [-1, 127], -1 default
      int _pitchShift; // [-36, 36]
      int _pan;        // [-64, 63]
      int _patch;      // [0, 127]
      int _stemType;   // 0, 1, 2

   public:
      Voice();
      ~Voice() {}

      void setChannel(int channel)       { _channel = channel;       }
      int channel() const                { return _channel;          }

      void setVolume(int volume)         { _volume = volume;         }
      int volume() const                 { return _volume;           }

      void setPitchShift(int pitchShift) { _pitchShift = pitchShift; }
      int pitchShift() const             { return _pitchShift;       }

      void setPan(int pan)               { _pan = pan;               }
      int pan() const                    { return _pan;              }

      void setPatch(int patch)           { _patch = patch;           }
      int patch() const                  { return _patch;            }

      void setStemType(int stemType)     { _stemType = stemType;     }
      int stemType() const               { return _stemType;         }

      static int defaultPatch()          { return -1;                }
      static int defaultVolume()         { return -1;                }
      };

//---------------------------------------------------------
//   Track
//---------------------------------------------------------

class Track {
      int _number;
      QString _name;
      QString _briefName;
      unsigned _patch;
      int _channel;
      int _transpose;
      bool _showTranspose;
      int _noteShift;
      ClefType _startClef;
      ClefType _transposeClef;
      unsigned _displayPercent;
      KeyType _startKey;
      int _voiceCount;
      QList<Voice*> _voices;

      bool _showName;
      bool _showBriefName;
      bool _showKeyEachLine;
      bool _showLegerLine;
      bool _showClef;
      bool _showTimeSignature;
      bool _showKeySignature;
      bool _showBarline;
      bool _showClefEachLine;

      bool _fillWithRest;
      bool _flatTail;

      bool _mute;
      bool _solo;

      int _part;

   public:
      Track();
      ~Track();

      void setName(const QString& str)      { _name = str;               }
      QString name() const                  { return _name;              }

      void setBriefName(const QString& str) { _briefName = str;          }
      QString briefName() const             { return _briefName;         }

      void setPatch(unsigned patch)         { _patch = patch;            } // -1: percussion
      unsigned patch() const                { return _patch;             }

      void setChannel(int channel)          { _channel = channel;        }
      int channel() const                   { return _channel;           }

      void setShowName(bool show)           { _showName = show;          }
      bool showName() const                 { return _showName;          }

      void setShowBriefName(bool show)      { _showBriefName = show;     }
      bool showBriefName() const            { return _showBriefName;     }

      void setMute(bool mute)               { _mute = mute;              }
      bool mute() const                     { return _mute;              }

      void setSolo(bool solo)               { _solo = solo;              }
      bool solo() const                     { return _solo;              }

      void setShowKeyEachLine(bool show)    { _showKeyEachLine = show;   }
      bool showKeyEachLine() const          { return _showKeyEachLine;   }

      void setVoiceCount(int voices)        { _voiceCount = voices;      }
      int voiceCount() const                { return _voiceCount;        }

      void addVoice(Voice* voice)           { _voices.push_back(voice);  }
      QList<Voice*> voices() const          { return _voices;            }

      void setShowTranspose(bool show)      { _showTranspose = show;     }
      bool showTranspose() const            { return _showTranspose;     }

      void setTranspose(int transpose)      { _transpose = transpose;    }
      int transpose() const                 { return _transpose;         }

      void setNoteShift(int shift)          { _noteShift = shift;        }
      int noteShift() const                 { return _noteShift;         }

      void setStartClef(ClefType clef)      { _startClef = clef;         }
      ClefType startClef() const            { return _startClef;         }

      void setTransposeClef(ClefType clef)  { _transposeClef = clef;     }
      ClefType transposeClef() const        { return _transposeClef;     }

      void setStartKey(KeyType key)         { _startKey = key;           }
      KeyType startKey() const              { return _startKey;          }

      void setDisplayPercent(unsigned percent/*25~100*/) { _displayPercent = percent; }
      unsigned displayPercent() const                    { return _displayPercent;    }

      void setShowLegerLine(bool show)      { _showLegerLine = show;     }
      bool showLegerLine() const            { return _showLegerLine;     }

      void setShowClef(bool show)           { _showClef = show;          }
      bool showClef() const                 { return _showClef;          }

      void setShowTimeSignature(bool show)  { _showTimeSignature = show; }
      bool showTimeSignature() const        { return _showTimeSignature; }

      void setShowKeySignature(bool show)   { _showKeySignature = show;  }
      bool showKeySignature() const         { return _showKeySignature;  }

      void setShowBarline(bool show)        { _showBarline = show;       }
      bool showBarline() const              { return _showBarline;       }

      void setFillWithRest(bool fill)       { _fillWithRest = fill;      }
      bool fillWithRest() const             { return _fillWithRest;      }

      void setFlatTail(bool flat)           { _flatTail = flat;          }
      bool flatTail() const                 { return _flatTail;          }

      void setShowClefEachLine(bool show)   { _showClefEachLine = show;  }
      bool showClefEachLine() const         { return _showClefEachLine;  }

      struct DrumNode {
            int _line;
            int _headType;
            int _pitch;
            int _voice;

         public:
            DrumNode():
                  _line(0), _headType(0), _pitch(0), _voice(0) {}
            };
      void addDrum(const DrumNode& node)    { _drumKit.push_back(node);  }
      QList<DrumNode> drumKit() const       { return _drumKit;           }

      void clear();

      void setPart(int part)                { _part = part;              }
      int part() const                      { return _part;              }

   private:
      QList<DrumNode> _drumKit;
      };

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

class Page {
      int _beginLine;
      int _lineCount;

      int _lineInterval;
      int _staffInterval;
      int _staffInlineInterval;

      int _lineBarCount;
      int _pageLineCount;

      int _leftMargin;
      int _topMargin;
      int _rightMargin;
      int _bottomMargin;

      int _pageWidth;
      int _pageHeight;

   public:
      Page();
      ~Page() {}

      void setBeginLine(int line)               { _beginLine = line;               }
      int beginLine() const                     { return _beginLine;               }

      void setLineCount(int count)              { _lineCount = count;              }
      int lineCount() const                     { return _lineCount;               }

      void setLineInterval(int interval)        { _lineInterval = interval;        } // between system
      int lineInterval() const                  { return _lineInterval;            }

      void setStaffInterval(int interval)       { _staffInterval = interval;       }
      int staffInterval() const                 { return _staffInterval;           }

      void setStaffInlineInterval(int interval) { _staffInlineInterval = interval; } // between treble-bass staff
      int staffInlineInterval() const           { return _staffInlineInterval;     }

      void setLineBarCount(int count)           { _lineBarCount = count;           }
      int lineBarCount() const                  { return _lineBarCount;            }

      void setPageLineCount(int count)          { _pageLineCount = count;          }
      int pageLineCount() const                 { return _pageLineCount;           }

      void setLeftMargin(int margin)            { _leftMargin = margin;            }
      int leftMargin() const                    { return _leftMargin;              }

      void setTopMargin(int margin)             { _topMargin = margin;             }
      int topMargin() const                     { return _topMargin;               }

      void setRightMargin(int margin)           { _rightMargin = margin;           }
      int rightMargin() const                   { return _rightMargin;             }

      void setBottomMargin(int margin)          { _bottomMargin = margin;          }
      int bottomMargin() const                  { return _bottomMargin;            }

      void setPageWidth(int width)              { _pageWidth = width;              }
      int pageWidth() const                     { return _pageWidth;               }

      void setPageHeight(int height)            { _pageHeight = height;            }
      int pageHeight() const                    { return _pageHeight;              }
      };

//---------------------------------------------------------
//   Line
//---------------------------------------------------------

class Line {
      QList<Staff*> _staffs;
      unsigned _beginBar;
      unsigned _barCount;
      int _yOffset;
      int _leftXOffset;
      int _rightXOffset;

   public:
      Line();
      ~Line();

      void addStaff(Staff* staff)      { _staffs.push_back(staff); }
      int staffCount() const           { return _staffs.size();    }
      Staff* staff(int idx) const;

      void setBeginBar(unsigned bar)   { _beginBar = bar;          }
      unsigned beginBar() const        { return _beginBar;         }

      void setBarCount(unsigned count) { _barCount = count;        }
      unsigned barCount() const        { return _barCount;         }

      void setYOffset(int offset)      { _yOffset = offset;        }
      int yOffset() const              { return _yOffset;          }

      void setLeftXOffset(int offset)  { _leftXOffset = offset;    }
      int leftXOffset() const          { return _leftXOffset;      }

      void setRightXOffset(int offset) { _rightXOffset = offset;   }
      int rightXOffset() const         { return _rightXOffset;     }
      };

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

class Staff : public OffsetElement {
      ClefType _clef;
      int _keyType;
      bool _visible;
      GroupType _groupType;
      int _groupStaffCount;

   public:
      Staff();
      virtual ~Staff() {}

      void setClefType(ClefType clef)    { _clef = clef;             }
      ClefType clefType() const          { return _clef;             }

      void setKeyType(int key)           { _keyType = key;           }
      int keyType() const                { return _keyType;          }

      void setVisible(bool visible)      { _visible = visible;       }
      bool visible() const               { return _visible;          }

      void setGroupType(GroupType type)  { _groupType = type;        }
      GroupType groupType() const        { return _groupType;        }

      void setGroupStaffCount(int count) { _groupStaffCount = count; }
      int groupStaffCount() const        { return _groupStaffCount;  }
      };

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

class Note : public LineElement {
      bool _isRest;
      unsigned _note;
      AccidentalType _accidental;
      bool _showAccidental;
      unsigned _onVelocity;
      unsigned _offVelocity;
      NoteHeadType _headType;
      TiePos _tiePos;
      int _offsetStaff;
      bool _show;
      int _offsetTick; // for playback

   public:
      Note();
      virtual ~Note() {}

      void setIsRest(bool rest)               { _isRest = rest;          }
      bool isRest() const                     { return _isRest;          }

      void setNote(unsigned note)             { _note = note;            }
      unsigned note() const                   { return _note;            }

      void setAccidental(AccidentalType type) { _accidental = type;      }
      AccidentalType accidental() const       { return _accidental;      }

      void setShowAccidental(bool show)       { _showAccidental = show;  }
      bool showAccidental() const             { return _showAccidental;  }

      void setOnVelocity(unsigned velocity)   { _onVelocity = velocity;  }
      unsigned onVelocity() const             { return _onVelocity;      }

      void setOffVelocity(unsigned velocity)  { _offVelocity = velocity; }
      unsigned offVelocity() const            { return _offVelocity;     }

      void setHeadType(NoteHeadType type)     { _headType = type;        }
      NoteHeadType headType() const           { return _headType;        }

      void setTiePos(TiePos tiePos)           { _tiePos = tiePos;        }
      TiePos tiePos() const                   { return _tiePos;          }

      void setOffsetStaff(int offset)         { _offsetStaff = offset;   } // cross staff notes
      int offsetStaff() const                 { return _offsetStaff;     }

      void setShow(bool show)                 { _show = show;            }
      bool show() const                       { return _show;            }

      void setOffsetTick(int offset)          { _offsetTick = offset;    }
      int offsetTick() const                  { return _offsetTick;      }
      };

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

class Articulation : public OffsetElement {
      ArticulationType _type;
      bool _above;

      bool _changeSoundEffect;
      QPair<int, int> _soundEffect;
      bool _changeLength;
      int _lengthPercentage;
      bool _changeVelocity;
      int _velocity;
      bool _changeExtraLength;
      int _extraLength;

      bool _auxiliaryFirst;
      NoteType _trillRate;
      int _trillNoteLength;

   public:
      Articulation();
      virtual ~Articulation() {}

      void setArticulationType(ArticulationType type) { _type = type;   }
      ArticulationType articulationType() const       { return _type;   }

      void setPlacementAbove(bool above)              { _above = above; }
      bool placementAbove() const                     { return _above;  }

      // for midi
      bool willAffectNotes() const;

      bool isTrill() const;

      // for xml
      enum class XmlType : char {
            Articulation,
            Technical,
            Arpeggiate,
            Ornament,
            Fermata,
            Direction,

            Unknown
            };
      XmlType xmlType() const;

      // sound setting
      bool changeSoundEffect() const      { return _changeSoundEffect; }
      void setSoundEffect(int soundFrom, int soundTo);
      QPair<int, int> soundEffect() const { return _soundEffect;       }

      bool changeLength() const           { return _changeLength;      }
      void setLengthPercentage(int percentage);
      int lengthPercentage() const        { return _lengthPercentage;  }

      bool changeVelocity() const         { return _changeVelocity;    }
      enum class VelocityType : char {
            Offset,
            SetValue,
            Percentage
            };
      void setVelocityType(VelocityType type);
      VelocityType velocityType() const   { return _velocityType;      }

      void setVelocity(int value)         { _velocity = value;         }
      int velocity() const                { return _velocity;          }

      bool changeExtraLength() const      { return _changeExtraLength; }
      void setExtraLength(int length);
      int extraLength() const             { return _extraLength;       }

      enum class TrillInterval : char {
            Diatonic = 0,
            Chromatic,
            Whole
            };
      void setTrillInterval(TrillInterval interval) { _trillInterval = interval; }
      TrillInterval trillInterval() const           { return _trillInterval;     }

      void setAuxiliaryFirst(bool first)  { _auxiliaryFirst = first;   }
      bool auxiliaryFirst() const         { return _auxiliaryFirst;    }

      void setTrillRate(NoteType rate)    { _trillRate = rate;         }
      NoteType trillRate() const          { return _trillRate;         }

      void setTrillNoteLength(int length) { _trillNoteLength = length; }
      int trillNoteLength() const         { return _trillNoteLength;   }

      enum class AccelerateType : char {
            None = 0,
            Slow,
            Normal,
            Fast
            };
      void setAccelerateType(AccelerateType type) { _accelerateType = type; }
      AccelerateType accelerateType() const       { return _accelerateType; }

   private:
      VelocityType _velocityType;
      TrillInterval _trillInterval;
      AccelerateType _accelerateType;
      };

//---------------------------------------------------------
//   NoteContainer
//---------------------------------------------------------

class NoteContainer : public MusicData, public LengthElement {
      bool _isGrace;
      bool _isCue;
      bool _isRest;
      bool _isRaw;
      NoteType _noteType;
      int _dot;
      NoteType _graceNoteType;
      int _tuplet;
      int _space;
      bool _inBeam;
      bool _up;
      bool _showStem;
      int _stemLength; // line count span
      int _noteShift;

      QList<Note*> _notes;
      QList<Articulation*> _articulations;

   public:
      NoteContainer();
      virtual ~NoteContainer();

      void setIsGrace(bool grace)                { _isGrace = grace;      }
      bool isGrace() const                       { return _isGrace;       }

      void setIsCue(bool cue)                    { _isCue = cue;          }
      bool isCue() const                         { return _isCue;         }

      void setIsRest(bool rest)                  { _isRest = rest;        }
      bool isRest() const                        { return _isRest;        }

      void setIsRaw(bool raw)                    { _isRaw = raw;          }
      bool isRaw() const                         { return _isRaw;         }

      void setNoteType(NoteType type);
      NoteType noteType() const                  { return _noteType;      }

      void setDot(int dot)                       { _dot = dot;            }
      int dot() const                            { return _dot;           }

      void setGraceNoteType(NoteType type)       { _graceNoteType = type; }
      NoteType graceNoteType() const             { return _graceNoteType; }

      void setInBeam(bool in)                    { _inBeam = in;          }
      bool inBeam() const                        { return _inBeam;        }

      void setUp(bool up)                        { _up = up;              }
      bool up() const                            { return _up;            }

      void setShowStem(bool show)                { _showStem = show;      }
      bool showStem() const                      { return _showStem;      }

      void setStemLength(int line)               { _stemLength = line;    }
      int stemLength() const                     { return _stemLength;    }

      void setTuplet(int tuplet)                 { _tuplet = tuplet;      }
      int tuplet() const                         { return _tuplet;        }

      void setSpace(int space)                   { _space = space;        }
      int space() const                          { return _space;         }

      void addNoteRest(Note* note)               { _notes.push_back(note);        }
      QList<Note*> notesRests() const            { return _notes;                 }

      void addArticulation(Articulation* art)    { _articulations.push_back(art); }
      QList<Articulation*> articulations() const { return _articulations;         }

      void setNoteShift(int octave)              { _noteShift = octave;   }
      int noteShift() const                      { return _noteShift;     }

      int offsetStaff() const;

      int duration() const;
      };

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

class Beam : public MusicData, public PairEnds {
      bool _isGrace;
      QList<QPair<MeasurePos, MeasurePos>> _lines;

   public:
      Beam();
      virtual ~Beam() {}

      void setIsGrace(bool grace) { _isGrace = grace; }
      bool isGrace() const        { return _isGrace;  }

      void addLine(const MeasurePos& startMp, const MeasurePos& endMp) { _lines.push_back(qMakePair(startMp, endMp)); }
      const QList<QPair<MeasurePos, MeasurePos>> lines() const         { return _lines; }
      };

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

class Tie : public MusicData, public PairEnds {
      bool _showOnTop;
      int _note;
      int _height;

   public:
      Tie();
      virtual ~Tie() {}

      void setShowOnTop(bool top) { _showOnTop = top;  }
      bool showOnTop() const      { return _showOnTop; }

      void setNote(int note)      { _note = note;      } // note value tie point to
      int note() const            { return _note;      }

      void setHeight(int height)  { _height = height;  }
      int height() const          { return _height;    }
      };

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

class Glissando : public MusicData, public PairEnds {
      bool _straight;
      QString _text;
      int _lineThick;

   public:
      Glissando();
      virtual ~Glissando() {}

      void setStraightWavy(bool straight) { _straight = straight; }
      bool straightWavy() const           { return _straight;     }

      void setText(const QString& text)   { _text = text;         }
      QString text() const                { return _text;         }

      void setLineThick(int thick)        { _lineThick = thick;   }
      int lineThick() const               { return _lineThick;    }
      };

//---------------------------------------------------------
//   Decorator
//---------------------------------------------------------

class Decorator : public MusicData {
      ArticulationType _articulationType;

   public:
      Decorator();
      virtual ~Decorator() {}

      enum class DecoratorType : char {
            Dotted_Barline = 0,
            Articulation
            };
      void setDecoratorType(DecoratorType type)       { _decoratorType = type;    }
      DecoratorType decoratorType() const             { return _decoratorType;    }

      void setArticulationType(ArticulationType type) { _articulationType = type; }
      ArticulationType articulationType() const       { return _articulationType; }

   private:
      DecoratorType _decoratorType;
      };

//---------------------------------------------------------
//   MeasureRepeat
//---------------------------------------------------------

class MeasureRepeat : public MusicData {
      bool _singleRepeat;

   public:
      MeasureRepeat();
      virtual ~MeasureRepeat() {}

      void setSingleRepeat(bool single); // false : qreal
      bool singleRepeat() const { return _singleRepeat; }
      };

//---------------------------------------------------------
//   Tuplet
//---------------------------------------------------------

class Tuplet : public MusicData, public PairEnds {
      int _tuplet;
      int _space;
      int _height;
      NoteType _noteType;
      OffsetElement* _mark;

   public:
      Tuplet();
      virtual ~Tuplet();

      void setTuplet(int tuplet = 3)    { _tuplet = tuplet; }
      int tuplet() const                { return _tuplet;   }

      void setSpace(int space = 2)      { _space = space;   }
      int space() const                 { return _space;    }

      void setHeight(int height)        { _height = height; }
      int height() const                { return _height;   }

      void setNoteType(NoteType type)   { _noteType = type; }
      NoteType noteType() const         { return _noteType; }

      OffsetElement* markHandle() const { return _mark;     }
      };

//---------------------------------------------------------
//   Harmony
//---------------------------------------------------------

class Harmony : public MusicData, public LengthElement {
      QString _harmonyType;
      int _root;
      int _bass;
      int _alterRoot;
      int _alterBass;
      bool _bassOnBottom;
      int _angle;

   public:
      Harmony();
      virtual ~Harmony() {}

      void setHarmonyType(QString type) { _harmonyType = type;  }
      QString harmonyType() const       { return _harmonyType;  }

      void setRoot(int root = 0)        { _root = root;         } // C
      int root() const                  { return _root;         }

      void setBass(int bass)            { _bass = bass;         }
      int bass() const                  { return _bass;         }

      void setAlterRoot(int val)        { _alterRoot = val;     }
      int alterRoot() const             { return _alterRoot;    }

      void setAlterBass(int val)        { _alterBass = val;     }
      int alterBass() const             { return _alterBass;    }

      void setBassOnBottom(bool on)     { _bassOnBottom = on;   }
      bool bassOnBottom() const         { return _bassOnBottom; }

      void setAngle(int angle)          { _angle = angle;       }
      int angle() const                 { return _angle;        }
      };

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

class Clef : public MusicData, public LineElement {
      ClefType _clefType;

   public:
      Clef();
      virtual ~Clef() {}

      void setClefType(ClefType type) { _clefType = type; }
      ClefType clefType() const       { return _clefType; }
      };

//---------------------------------------------------------
//   Lyric
//---------------------------------------------------------

class Lyric : public MusicData {
      QString _lyric;
      int _verse;

   public:
      Lyric();
      virtual ~Lyric() {}

      void setLyric(const QString& lyricText) { _lyric = lyricText; }
      QString lyric() const                   { return _lyric;      }

      void setVerse(int verse)                { _verse = verse;     }
      int verse() const                       { return _verse;      }
      };

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

class Slur : public MusicData, public PairEnds {
      int _containerCount;
      bool _showOnTop;
      int _noteTimePercent;
      OffsetElement* _handle2;
      OffsetElement* _handle3;

   public:
      Slur();
      virtual ~Slur();

      void setContainerCount(int count)    { _containerCount = count;    } // span
      int containerCount() const           { return _containerCount;     }

      void setShowOnTop(bool top)          { _showOnTop = top;           }
      bool showOnTop() const               { return _showOnTop;          }

      OffsetElement* handle2() const       { return _handle2;            }
      OffsetElement* handle3() const       { return _handle3;            }

      void setNoteTimePercent(int percent) { _noteTimePercent = percent; } // 50% ~ 200%
      int noteTimePercent() const          { return _noteTimePercent;    }
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

class Dynamic : public MusicData {
      DynamicType _dynamicType;
      bool _play;
      int _velocity;

   public:
      Dynamic();
      virtual ~Dynamic() {}

      void setDynamicType(DynamicType type) { _dynamicType = type; }
      DynamicType dynamicType() const       { return _dynamicType; }

      void setPlay(bool play)               { _play = play;        }
      bool play() const                     { return _play;        }

      void setVelocity(int vel)             { _velocity = vel;     }
      int velocity() const                  { return _velocity;    }
      };

//---------------------------------------------------------
//   WedgeEndPoint
//---------------------------------------------------------

class WedgeEndPoint : public MusicData {
      int _height;
      WedgeType _wedgeType;
      bool _wedgeStart;

   public:
      WedgeEndPoint();
      virtual ~WedgeEndPoint() {}

      void setWedgeType(WedgeType type)   { _wedgeType = type;        }
      WedgeType wedgeType() const         { return _wedgeType;        }

      void setHeight(int height)          { _height = height;         }
      int height() const                  { return _height;           }

      void setWedgeStart(bool wedgeStart) { _wedgeStart = wedgeStart; }
      bool wedgeStart() const             { return _wedgeStart;       }
      };

//---------------------------------------------------------
//   Wedge
//---------------------------------------------------------

class Wedge : public MusicData {
      int _height;
      WedgeType _wedgeType;

   public:
      Wedge();
      virtual ~Wedge() {}

      void setWedgeType(WedgeType type) { _wedgeType = type; }
      WedgeType wedgeType() const       { return _wedgeType; }

      void setHeight(int height)        { _height = height;  }
      int height() const                { return _height;    }
      };

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

class Pedal : public MusicData, public PairEnds {
      bool _half;
      bool _play;
      int _playOffset;
      OffsetElement* _pedalHandle;

   public:
      Pedal();
      virtual ~Pedal();

      void setHalf(bool half)            { _half = half;         }
      bool half() const                  { return _half;         }

      void setPlay(bool play)            { _play = play;         }
      bool play() const                  { return _play;         }

      void setPlayOffset(int offset)     { _playOffset = offset; } // -127~127
      int playOffset() const             { return _playOffset;   }

      OffsetElement* pedalHandle() const { return _pedalHandle;  } // only on half pedal
      };

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

class Bracket : public MusicData, public PairEnds {
      int _height;
      BracketType _bracketType;

   public:
      Bracket();
      virtual ~Bracket() {}

      void setHeight(int height)            { _height = height;    }
      int height() const                    { return _height;      }

      void setBracketType(BracketType type) { _bracketType = type; }
      BracketType bracketType() const       { return _bracketType; }
      };

//---------------------------------------------------------
//   Expression
//---------------------------------------------------------

class Expression : public MusicData {
      QString _text;

   public:
      Expression();
      virtual ~Expression() {}

      void setText(const QString& str) { _text = str;  }
      QString text() const             { return _text; }
      };

//---------------------------------------------------------
//   HarpPedal
//---------------------------------------------------------

class HarpPedal : public MusicData {
      HarpPedalType _type;
      int _showCharFlag;

   public:
      HarpPedal();
      virtual ~HarpPedal() {}

      void setType(HarpPedalType type) { _type = type;         }
      HarpPedalType type() const       { return _type;         }

      // each bit is a bool, total 7 bools
      void setShowCharFlag(int flag)   { _showCharFlag = flag; }
      int showCharFlag() const         { return _showCharFlag; }
      };

//---------------------------------------------------------
//   OctaveShift
//---------------------------------------------------------

class OctaveShift : public MusicData, public LengthElement {
      OctaveShiftType _octaveShiftType;
      OctaveShiftPosition _octaveShiftPosition;
      int _endTick;

   public:
      OctaveShift();
      virtual ~OctaveShift() {}

      void setOctaveShiftType(OctaveShiftType type) { _octaveShiftType = type; }
      OctaveShiftType octaveShiftType() const       { return _octaveShiftType; }

      void setOctaveShiftPosition(OctaveShiftPosition position) { _octaveShiftPosition = position; }
      OctaveShiftPosition octaveShiftPosition() const           { return _octaveShiftPosition;     }

      int noteShift() const;

      void setEndTick(int tick) { _endTick = tick; }
      int endTick() const       { return _endTick; }
      };

//---------------------------------------------------------
//   OctaveShiftEndPoint
//---------------------------------------------------------

class OctaveShiftEndPoint : public MusicData, public LengthElement {
      OctaveShiftType _octaveShiftType;
      OctaveShiftPosition _octaveShiftPosition;
      int _endTick;

   public:
      OctaveShiftEndPoint();
      virtual ~OctaveShiftEndPoint() {}

      void setOctaveShiftType(OctaveShiftType type) { _octaveShiftType = type; }
      OctaveShiftType octaveShiftType() const       { return _octaveShiftType; }

      void setOctaveShiftPosition(OctaveShiftPosition position) { _octaveShiftPosition = position; }
      OctaveShiftPosition octaveShiftPosition() const           { return _octaveShiftPosition;     }

      void setEndTick(int tick) { _endTick = tick; }
      int endTick() const       { return _endTick; }
      };

//---------------------------------------------------------
//   MultiMeasureRest
//---------------------------------------------------------

class MultiMeasureRest : public MusicData {
      int _measureCount;

   public:
      MultiMeasureRest();
      virtual ~MultiMeasureRest() {}

      void setMeasureCount(int count) { _measureCount = count; }
      int measureCount() const        { return _measureCount;  }
      };

//---------------------------------------------------------
//   Tempo
//---------------------------------------------------------

class Tempo : public MusicData {
      bool _showMark;
      bool _showBeforeText;
      bool _showParentheses;
      qreal _typeTempo;
      QString _leftText;
      QString _rightText;
      bool _swingEighth;
      NoteType _leftNoteType;
      NoteType _rightNoteType;
      bool _leftNoteDot;
      bool _rightNoteDot;
      int _rightSideType;

   public:
      Tempo();
      virtual ~Tempo() {}

      void setShowMark(bool show)           { _showMark = show;        }
      bool showMark() const                 { return _showMark;        }

      void setShowBeforeText(bool show)     { _showBeforeText = show;  }
      bool showBeforeText() const           { return _showBeforeText;  }

      void setShowParentheses(bool show)    { _showParentheses = show; }
      bool showParentheses() const          { return _showParentheses; }

      void setTypeTempo(qreal tempo)        { _typeTempo = tempo;      } // 0x2580 = 96.00
      qreal typeTempo() const               { return _typeTempo;       }
      qreal quarterTempo() const;

      void setLeftText(const QString& str)  { _leftText = str;         } // string at left of the mark
      QString leftText() const              { return _leftText;        }

      void setRightText(const QString& str) { _rightText = str;        }
      QString rightText() const             { return _rightText;       }

      void setSwingEighth(bool swing)       { _swingEighth = swing;    }
      bool swingEighth() const              { return _swingEighth;     }

      void setLeftNoteType(NoteType type)   { _leftNoteType = type;    }
      NoteType leftNoteType() const         { return _leftNoteType;    }

      void setRightNoteType(NoteType type)  { _rightNoteType = type;   }
      NoteType rightNoteType() const        { return _rightNoteType;   }

      void setLeftNoteDot(bool showDot)     { _leftNoteDot = showDot;  }
      bool leftNoteDot() const              { return _leftNoteDot;     }

      void setRightNoteDot(bool showDot)    { _rightNoteDot = showDot; }
      bool rightNoteDot() const             { return _rightNoteDot;    }

      void setRightSideType(int type)       { _rightSideType = type;   }
      int rightSideType() const             { return _rightSideType;   }
      };

//---------------------------------------------------------
//  Text
//---------------------------------------------------------

class Text : public MusicData, public LengthElement {
      int _horizontalMargin;
      int _verticalMargin;
      int _lineThick;
      QString _text;
      int _width;
      int _height;

   public:
      Text();
      virtual ~Text() {}

      enum class TextType : char {
            Rehearsal,
            SystemText,
            MeasureText
            };

      void setTextType(TextType type)      { _textType = type;           }
      TextType textType() const            { return _textType;           }

      void setHorizontalMargin(int margin) { _horizontalMargin = margin; }
      int horizontalMargin() const         { return _horizontalMargin;   }

      void setVerticalMargin(int margin)   { _verticalMargin = margin;   }
      int verticalMargin() const           { return _verticalMargin;     }

      void setLineThick(int thick)         { _lineThick = thick;         }
      int lineThick() const                { return _lineThick;          }

      void setText(const QString& str)     { _text = str;                }
      QString text() const                 { return _text;               }

      void setWidth(int width)             { _width = width;             }
      int width() const                    { return _width;              }

      void setHeight(int height)           { _height = height;           }
      int height() const                   { return _height;             }

   private:
      TextType _textType;
      };

//---------------------------------------------------------
//   TimeSignature
//---------------------------------------------------------

class TimeSignature : public MusicData {
      int _numerator;
      int _denominator;
      bool _isSymbol;
      int _beatLength;
      int _barLength;

      struct BeatNode {
            int _startUnit;
            int _lengthUnit;
            int _startTick;

            BeatNode() :
                  _startUnit(0), _lengthUnit(0), _startTick(0) {}
      };
      QList<BeatNode> _beats;
      int _barLengthUnits;

      bool _replaceFont;
      bool _showBeatGroup;

      int _groupNumerator1;
      int _groupNumerator2;
      int _groupNumerator3;
      int _groupDenominator1;
      int _groupDenominator2;
      int _groupDenominator3;

      int _beamGroup1;
      int _beamGroup2;
      int _beamGroup3;
      int _beamGroup4;

      int _beamCount16th;
      int _beamCount32th;

   public:
      TimeSignature();
      virtual ~TimeSignature() {}

      void setNumerator(int numerator)     { _numerator = numerator;     }
      int numerator() const                { return _numerator;          }

      void setDenominator(int denominator) { _denominator = denominator; }
      int denominator() const              { return _denominator;        }

      void setIsSymbol(bool symbol)        { _isSymbol = symbol;         } // 4/4: common, 2/2: cut
      bool isSymbol() const;

      void setBeatLength(int length)       { _beatLength = length;       } // tick
      int beatLength() const               { return _beatLength;         }

      void setBarLength(int length)        { _barLength = length;        } // tick
      int barLength() const                { return _barLength;          }

      void addBeat(int startUnit, int lengthUnit, int startTick);
      void endAddBeat();
      int units() const                    { return _barLengthUnits;     }

      void setReplaceFont(bool replace)    { _replaceFont = replace;     }
      bool replaceFont() const             { return _replaceFont;        }

      void setShowBeatGroup(bool show)     { _showBeatGroup = show;      }
      bool showBeatGroup() const           { return _showBeatGroup;      }

      void setGroupNumerator1(int numerator)     { _groupNumerator1 = numerator;     }
      void setGroupNumerator2(int numerator)     { _groupNumerator2 = numerator;     }
      void setGroupNumerator3(int numerator)     { _groupNumerator3 = numerator;     }
      void setGroupDenominator1(int denominator) { _groupDenominator1 = denominator; }
      void setGroupDenominator2(int denominator) { _groupDenominator2 = denominator; }
      void setGroupDenominator3(int denominator) { _groupDenominator3 = denominator; }

      void setBeamGroup1(int count)        { _beamGroup1 = count;        }
      void setBeamGroup2(int count)        { _beamGroup2 = count;        }
      void setBeamGroup3(int count)        { _beamGroup3 = count;        }
      void setBeamGroup4(int count)        { _beamGroup4 = count;        }

      void set16thBeamCount(int count)     { _beamCount16th = count;     }
      void set32thBeamCount(int count)     { _beamCount32th = count;     }
      };

//---------------------------------------------------------
//   Key
//---------------------------------------------------------

class Key : public MusicData {
      int _key;
      bool _set;
      int _previousKey;
      int _symbolCount;

   public:
      Key();
      virtual ~Key() {}

      void setKey(int key); // C=0x0, G=0x8, C#=0xE, F=0x1, Db=0x7
      int key() const                { return _key;          }
      bool setKey() const            { return _set;          }

      void setPreviousKey(int key)   { _previousKey = key;   }
      int previousKey() const        { return _previousKey;  }

      void setSymbolCount(int count) { _symbolCount = count; }
      int symbolCount() const        { return _symbolCount;  }
      };

//---------------------------------------------------------
//   RepeatSymbol
//---------------------------------------------------------

class RepeatSymbol : public MusicData {
      QString _text;
      RepeatType _repeatType;

   public:
      RepeatSymbol();
      virtual ~RepeatSymbol() {}

      void setText(const QString& text)         { _text = text;             }
      QString text() const                      { return _text;             }

      void setRepeatType(RepeatType repeatType) { _repeatType = repeatType; }
      RepeatType repeatType() const             { return _repeatType;       }
      };

//---------------------------------------------------------
//   NumericEnding
//---------------------------------------------------------

class NumericEnding : public MusicData, public PairEnds {
      int _height;
      QString _text;
      OffsetElement* _numericHandle;

   public:
      NumericEnding();
      virtual ~NumericEnding();

      OffsetElement* numericHandle() const { return _numericHandle; }

      void setHeight(int height)           { _height = height;      }
      int height() const                   { return _height;        }

      void setText(const QString& text)    { _text = text;          }
      QString text() const                 { return _text;          }

      QList<int> numbers() const;
      int jumpCount() const;
      };

//---------------------------------------------------------
//   BarNumber
//---------------------------------------------------------

class BarNumber : public MusicData {
      int _index;
      bool _showOnParagraphStart;
      int _align;
      int _showFlag;
      int _barRange;
      QString _prefix;

   public:
      BarNumber();
      virtual ~BarNumber() {}

      void setIndex(int index)                { _index = index;               }
      int index() const                       { return _index;                }

      void setShowOnParagraphStart(bool show) { _showOnParagraphStart = show; }
      bool showOnParagraphStart() const       { return _showOnParagraphStart; }

      void setAlign(int align)                { _align = align;               } // 0: left, 1: center, 2: right
      int align() const                       { return _align;                }

      void setShowFlag(int flag)              { _showFlag = flag;             } // 0: page, 1: staff, 2: bar, 3: none
      int showFlag() const                    { return _showFlag;             }

      void setShowEveryBarCount(int count)    { _barRange = count;            }
      int showEveryBarCount() const           { return _barRange;             }

      void setPrefix(const QString& str)      { _prefix = str;                }
      QString prefix() const                  { return _prefix;               }
      };

//---------------------------------------------------------
//   MidiController
//---------------------------------------------------------

class MidiController : public MidiData {
      int _controller;
      int _value;

   public:
      MidiController();
      virtual ~MidiController() {}

      void setController(int number) { _controller = number; }
      int controller() const         { return _controller;   }

      void setValue(int value)       { _value = value;       }
      int value() const              { return _value;        }
      };

//---------------------------------------------------------
//   MidiProgramChange
//---------------------------------------------------------

class MidiProgramChange : public MidiData {
      int _patch;

   public:
      MidiProgramChange();
      virtual ~MidiProgramChange() {}

      void setPatch(int patch) { _patch = patch; }
      int patch() const        { return _patch;  }
      };

//---------------------------------------------------------
//   MidiChannelPressure
//---------------------------------------------------------

class MidiChannelPressure : public MidiData {
      int _pressure;

   public:
      MidiChannelPressure();
      virtual ~MidiChannelPressure() {}

      void setPressure(int pressure) { _pressure = pressure; }
      int pressure() const           { return _pressure;     }
      };

//---------------------------------------------------------
//   MidiPitchWheel
//---------------------------------------------------------

class MidiPitchWheel : public MidiData {
      int _value;

   public:
      MidiPitchWheel();
      virtual ~MidiPitchWheel() {}

      void setValue(int value) { _value = value; }
      int value() const        { return _value;  }
      };

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

class Measure : public LengthElement {
      BarNumber* _barNumber;
      TimeSignature* _timeSig;

      BarLineType _leftBarlineType;
      BarLineType _rightBarlineType;
      int _repeatCount;
      qreal _typeTempo; // based on some type
      bool _isPickup;
      bool _isMultiMeasureRest;
      int _multiMeasureRestCount;

   public:
      Measure(int index = 0);
      virtual ~Measure();

   private:
      Measure();

   public:
      BarNumber* barNumber() const                 { return _barNumber;              }
      TimeSignature* timeSig() const               { return _timeSig;                }

      void setLeftBarlineType(BarLineType type)    { _leftBarlineType = type;        }
      BarLineType leftBarlineType() const          { return _leftBarlineType;        }

      void setRightBarlineType(BarLineType type)   { _rightBarlineType = type;       }
      BarLineType rightBarlineType() const         { return _rightBarlineType;       }

      // set when rightBarlineType == BarLineType::Repeat_Right
      void setBackwardRepeatCount(int repeatCount) { _repeatCount = repeatCount;     }
      int backwardRepeatCount() const              { return _repeatCount;            }

      void setTypeTempo(qreal tempo)               { _typeTempo = tempo;             }
      qreal typeTempo() const                      { return _typeTempo;              }

      void setIsPickup(bool pickup)                { _isPickup = pickup;             }
      bool isPickup() const                        { return _isPickup;               }

      void setIsMultiMeasureRest(bool mm)          { _isMultiMeasureRest = mm;       }
      bool isMultiMeasureRest() const              { return _isMultiMeasureRest;     }

      void setMultiMeasureRestCount(int count)     { _multiMeasureRestCount = count; }
      int multiMeasureRestCount() const            { return _multiMeasureRestCount;  }

   private:
      void clear();
      };

//---------------------------------------------------------
//   MeasureData
//---------------------------------------------------------

class MeasureData {
      Key* _key;
      Clef* _clef;
      QList<MusicData*> _musicDatas;
      QList<NoteContainer*> _noteContainers;
      QList<QPair<MusicData*, bool>> _crossMeasureElements;
      QList<MidiData*> _midiDatas;

   public:
      MeasureData();
      ~MeasureData();

      Clef* clef() const { return _clef; }
      Key* key() const   { return _key;  }

      void addNoteContainer(NoteContainer* ptr)    { _noteContainers.push_back(ptr); }
      QList<NoteContainer*> noteContainers() const { return _noteContainers;         }

      // put Tempo, Text, RepeatSymbol to MeasureData at part=0 && staff=0
      void addMusicData(MusicData* ptr)            { _musicDatas.push_back(ptr);     }
      // if type == MusicDataType::None, return all
      QList<MusicData*> musicDatas(MusicDataType type); // MusicXml: note | direction | harmony

      // put NumericEnding to MeasureData at part=0 && staff=0
      void addCrossMeasureElement(MusicData* ptr, bool start) { _crossMeasureElements.push_back(qMakePair(ptr, start)); }
      enum class PairType : char {
            Start,
            Stop,
            All
            };
      QList<MusicData*> crossMeasureElements(MusicDataType type, PairType pairType);

      // for midi
      void addMidiData(MidiData* data)             { _midiDatas.push_back(data);     }
      QList<MidiData*> midiDatas(MidiType type);
      };

//---------------------------------------------------------
//   StreamHandle
//---------------------------------------------------------

class StreamHandle {
      int _size;
      int _curPos;
      unsigned char* _point;

   public:
      StreamHandle(unsigned char* p, int size);
      virtual ~StreamHandle();

   private:
      StreamHandle();

   public:
      virtual bool read(char* buff, int size);
      virtual bool write(char* /*buff*/, int /*size*/) { return true; }
      };

//---------------------------------------------------------
//   Block
//    Base block, or resizable block in ove to store data
//---------------------------------------------------------

class Block {
      // char [-128, 127], unsigned char [0, 255]
      QList<unsigned char> _data;

   public:
      Block();
      explicit Block(unsigned size);
      virtual ~Block() {}

      // size > 0, check this in use code
      virtual void resize(unsigned count) { doResize(count); }

      const unsigned char* data() const { return &_data.front(); }
      unsigned char* data()             { return &_data.front(); }
      int size() const                  { return _data.size();   }

      bool operator==(const Block& block) const;
      bool operator!=(const Block& block) const;

      bool toBool() const;
      unsigned toUnsignedInt() const;
      int toInt() const;
      QByteArray toStrByteArray() const; // string
      QByteArray fixedSizeBufferToStrByteArray() const; // string

   private:
      void doResize(unsigned count);
      };

//---------------------------------------------------------
//   FixedBlock
//---------------------------------------------------------

class FixedBlock : public Block {
   public:
      explicit FixedBlock(unsigned count);
      virtual ~FixedBlock() {}

   private:
      FixedBlock();

      // can't resize
      virtual void resize(unsigned count);
      };

//---------------------------------------------------------
//   SizeBlock
//    4 byte block in ove to store size
//---------------------------------------------------------

class SizeBlock : public FixedBlock {
   public:
      SizeBlock();
      virtual ~SizeBlock() {}

      unsigned toSize() const;
      };

//---------------------------------------------------------
//   NameBlock
//    4 byte block in ove to store name
//---------------------------------------------------------

class NameBlock : public FixedBlock {
   public:
      NameBlock();
      virtual ~NameBlock() {}

      // ignore data more than 4 bytes
      bool isEqual(const QString& name) const;
      };

//---------------------------------------------------------
//   CountBlock
//    2 byte block in ove to store count
//---------------------------------------------------------

class CountBlock : public FixedBlock {
   public:
      CountBlock();
      virtual ~CountBlock() {}

      unsigned short toCount() const;
      };

//---------------------------------------------------------
//   Chunk
//    content : name
//---------------------------------------------------------

class Chunk {
   public:
      Chunk();
      virtual ~Chunk() {}

      static const QString TrackName;
      static const QString PageName;
      static const QString LineName;
      static const QString StaffName;
      static const QString MeasureName;
      static const QString ConductName;
      static const QString BdatName;

      NameBlock name() const;

   protected:
      NameBlock _nameBlock;
      };

//---------------------------------------------------------
//   SizeChunk
//    content : name / size / data
//---------------------------------------------------------

class SizeChunk : public Chunk {
   public:
      SizeChunk();
      virtual ~SizeChunk();

      SizeBlock* sizeBlock() const { return _sizeBlock; }
      Block* dataBlock() const     { return _dataBlock; }

      static const unsigned version3TrackSize;

   protected:
      SizeBlock* _sizeBlock;
      Block* _dataBlock;
      };

//---------------------------------------------------------
//   GroupChunk
//    content : name / count
//---------------------------------------------------------

class GroupChunk : public Chunk {
   public:
      GroupChunk();
      virtual ~GroupChunk();

      CountBlock* countBlock() const { return _childCount; }

   protected:
      CountBlock* _childCount;
      };

//---------------------------------------------------------
//   BasicParse
//---------------------------------------------------------

class BasicParse {
   public:
      BasicParse(OveSong* ove);
      virtual ~BasicParse();

   private:
      BasicParse();

   public:
      void setNotify(IOveNotify* notify) { _notify = notify; }
      virtual bool parse()               { return false;     }

   protected:
      bool readBuffer(Block& placeHolder, int size);
      bool jump(int offset);

      void messageOut(const QString& str);

      OveSong* _ove;
      StreamHandle* _handle;
      IOveNotify* _notify;
      };

//---------------------------------------------------------
//   OvscParse
//---------------------------------------------------------

class OvscParse : public BasicParse {
      SizeChunk* _chunk;

   public:
      OvscParse(OveSong* ove);
      virtual ~OvscParse();

      void setOvsc(SizeChunk* chunk) { _chunk = chunk; }

      virtual bool parse();
      };

//---------------------------------------------------------
//   TrackParse
//---------------------------------------------------------

class TrackParse : public BasicParse {
      SizeChunk* _chunk;

   public:
      TrackParse(OveSong* ove);
      virtual ~TrackParse();

      void setTrack(SizeChunk* chunk) { _chunk = chunk; }

      virtual bool parse();
      };

//---------------------------------------------------------
//   GroupParse
//---------------------------------------------------------

class GroupParse : BasicParse {
      QList<SizeChunk*> _sizeChunks;

   public:
      GroupParse(OveSong* ove);
      virtual ~GroupParse();

      void addSizeChunk(SizeChunk* sizeChunk) { _sizeChunks.push_back(sizeChunk); }

      virtual bool parse() { return false; }
      };

//---------------------------------------------------------
//   PageGroupParse
//---------------------------------------------------------

class PageGroupParse : public BasicParse {
      QList<SizeChunk*> _pageChunks;

   public:
      PageGroupParse(OveSong* ove);
      virtual ~PageGroupParse();

      void addPage(SizeChunk* chunk) { _pageChunks.push_back(chunk); }

      virtual bool parse();

   private:
      bool parsePage(SizeChunk* chunk, Page* page);
      };

//---------------------------------------------------------
//   StaffCountGetter
//---------------------------------------------------------

class StaffCountGetter : public BasicParse {
   public:
      StaffCountGetter(OveSong* ove);
      virtual ~StaffCountGetter() {}

      unsigned staffCount(SizeChunk* chunk);
      };

//---------------------------------------------------------
//   LineGroupParse
//---------------------------------------------------------

class LineGroupParse : public BasicParse {
      GroupChunk* _chunk;
      QList<SizeChunk*> _lineChunks;
      QList<SizeChunk*> _staffChunks;

   public:
      LineGroupParse(OveSong* ove);
      virtual ~LineGroupParse();

      void setLineGroup(GroupChunk* chunk) { _chunk = chunk;                }
      void addLine(SizeChunk* chunk)       { _lineChunks.push_back(chunk);  }
      void addStaff(SizeChunk* chunk)      { _staffChunks.push_back(chunk); }

      virtual bool parse();

   private:
      bool parseLine(SizeChunk* chunk, Line* line);
      bool parseStaff(SizeChunk* chunk, Staff* staff);
      };

//---------------------------------------------------------
//   BarsParse
//---------------------------------------------------------

class BarsParse : public BasicParse {
      QList<SizeChunk*> _measureChunks;
      QList<SizeChunk*> _conductChunks;
      QList<SizeChunk*> _bdatChunks;

   public:
      BarsParse(OveSong* ove);
      virtual ~BarsParse();

      void addMeasure(SizeChunk* chunk) { _measureChunks.push_back(chunk); }
      void addConduct(SizeChunk* chunk) { _conductChunks.push_back(chunk); }
      void addBdat(SizeChunk* chunk)    { _bdatChunks.push_back(chunk);    }

      virtual bool parse();

      bool parseMeas(Measure* measure, SizeChunk* chunk);
      bool parseCond(Measure* measure, MeasureData* measureData, SizeChunk* chunk);
      bool parseBdat(Measure* measure, MeasureData* measureData, SizeChunk* chunk);

      bool condElementType(unsigned byteData, CondType& type);
      bool bdatElementType(unsigned byteData, BdatType& type);

      // COND
      bool parseTimeSignature(Measure* measure, int length);
      bool parseTimeSignatureParameters(Measure* measure, int length);
      bool parseRepeatSymbol(MeasureData* measureData, int length);
      bool parseNumericEnding(MeasureData* measureData, int length);
      bool parseTempo(MeasureData* measureData, int length);
      bool parseBarNumber(Measure* measure, int length);
      bool parseText(MeasureData* measureData, int length);
      bool parseBarlineParameters(Measure* measure, int length);

      // BDAT
      bool parseNoteRest(MeasureData* measureData, int length, BdatType type);
      bool parseBeam(MeasureData* measureData, int length);
      bool parseTie(MeasureData* measureData, int length);
      bool parseTuplet(MeasureData* measureData, int length);
      bool parseHarmony(MeasureData* measureData, int length);
      bool parseClef(MeasureData* measureData, int length);
      bool parseLyric(MeasureData* measureData, int length);
      bool parseSlur(MeasureData* measureData, int length);
      bool parseGlissando(MeasureData* measureData, int length);
      bool parseDecorator(MeasureData* measureData, int length);
      bool parseDynamic(MeasureData* measureData, int length);
      bool parseWedge(MeasureData* measureData, int length);
      bool parseKey(MeasureData* measureData, int length);
      bool parsePedal(MeasureData* measureData, int length);
      bool parseBracket(MeasureData* measureData, int length);
      bool parseExpression(MeasureData* measureData, int length);
      bool parseHarpPedal(MeasureData* measureData, int length);
      bool parseMultiMeasureRest(MeasureData* measureData, int length);
      bool parseHarmonyGuitarFrame(MeasureData* measureData, int length);
      bool parseOctaveShift(MeasureData* measureData, int length);
      bool parseMidiController(MeasureData* measureData, int length);
      bool parseMidiProgramChange(MeasureData* measureData, int length);
      bool parseMidiChannelPressure(MeasureData* measureData, int length);
      bool parseMidiPitchWheel(MeasureData* measureData, int length);

      bool parseSizeBlock(int length);
      bool parseMidiCommon(MidiData* ptr);
      bool parseCommonBlock(MusicData* ptr);
      bool parseOffsetCommonBlock(MusicData* ptr);
      bool parsePairLinesBlock(PairEnds* ptr); // size == 2
      bool parseOffsetElement(OffsetElement* ptr); // size == 2
      };

//---------------------------------------------------------
//   LyricChunkParse
//---------------------------------------------------------

class LyricChunkParse : public BasicParse {
      SizeChunk* _chunk;

   public:
      LyricChunkParse(OveSong* ove);
      virtual ~LyricChunkParse();

      void setLyricChunk(SizeChunk* chunk) { _chunk = chunk; }

      virtual bool parse();

   private:
      struct LyricInfo {
            int _track;
            int _measure;
            int _verse;
            int _voice;
            int _wordCount;
            int _lyricSize;
            QString _name;
            QString _lyric;
            int _font;
            int _fontSize;
            int _fontStyle;

            LyricInfo() :
                  _track(0), _measure(0), _verse(0), _voice(0), _wordCount(0),
                  _lyricSize(0), _name(QString()), _lyric(QString()),
                  _font(0), _fontSize(12), _fontStyle(0) {}
            };

      void processLyricInfo(const LyricInfo& info);
      };

//---------------------------------------------------------
//   TitleChunkParse
//---------------------------------------------------------

class TitleChunkParse : public BasicParse {
      unsigned _titleType;
      unsigned _annotateType;
      unsigned _writerType;
      unsigned _copyrightType;
      unsigned _headerType;
      unsigned _footerType;

      SizeChunk* _chunk;

   public:
      TitleChunkParse(OveSong* ove);
      virtual ~TitleChunkParse();

      void setTitleChunk(SizeChunk* chunk) { _chunk = chunk; }

      virtual bool parse();

   private:
      void addToOve(const QString& str, unsigned titleType);
      };

//---------------------------------------------------------
//   OveOrganizer
//---------------------------------------------------------

class OveOrganizer {
      OveSong* _ove;

   public:
      OveOrganizer(OveSong* ove);
      virtual ~OveOrganizer() {}

      void organize();

   private:
      void organizeAttributes();
      void organizeTracks();
      void organizeMeasures();
      void organizeMeasure(int part, int track, Measure* measure, MeasureData* measureData);

      void organizeContainers(int part, int track, Measure* measure, MeasureData* measureData);
      void organizeMusicDatas(int part, int track, Measure* measure, MeasureData* measureData);
      void organizeCrossMeasureElements(int part, int track, Measure* measure, MeasureData* measureData);

      void organizePairElement(MusicData* data, int part, int track, Measure* measure, MeasureData* measureData);
      void organizeOctaveShift(OctaveShift* octave, Measure* measure, MeasureData* measureData);
      void organizeWedge(Wedge* wedge, int part, int track, Measure* measure, MeasureData* measureData);
      };

//---------------------------------------------------------
//   OveSerialize
//---------------------------------------------------------

class OveSerialize : public IOVEStreamLoader {
      OveSong* _ove;
      StreamHandle* _streamHandle;
      IOveNotify* _notify;

   public:
      OveSerialize();
      virtual ~OveSerialize();

      virtual void setOve(OveSong* ove)          { _ove = ove;       }
      virtual void setFileStream(unsigned char* buffer, unsigned size)
         { _streamHandle = new StreamHandle(buffer, size); }
      virtual void setNotify(IOveNotify* notify) { _notify = notify; }
      virtual bool load();

      virtual void release();

   private:
      bool readNameBlock(NameBlock& nameBlock);
      bool readChunkName(Chunk* chunk, const QString& name);
      bool readSizeChunk(SizeChunk* sizeChunk); // contains a SizeChunk and data buffer
      bool readDataChunk(Block* block, unsigned size);
      bool readGroupChunk(GroupChunk* groupChunk);

      bool readHeader();
      bool readHeadData(SizeChunk* ovscChunk);
      bool readTrackData();
      bool readPageData();
      bool readLineData();
      bool readBarData();
      bool readOveEnd();

      void messageOutError();
      void messageOut(const QString& str);
      };

}

#endif
