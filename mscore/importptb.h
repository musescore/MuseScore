#pragma once

#include <libmscore/score.h>
#include <libmscore/mscore.h>
#include <libmscore/fraction.h>
#include <libmscore/fret.h>
#include <libmscore/chordrest.h>
#include <libmscore/slur.h>
#include <libmscore/clef.h>
#include <libmscore/keysig.h>
#include <libmscore/chordrest.h>
#include <libmscore/clef.h>
#include <libmscore/keysig.h>
#include <libmscore/hairpin.h>
#include <libmscore/ottava.h>
#include <libmscore/drumset.h>

namespace Ms {

class PalmMute;

class PowerTab {
            QFile*                  _file;
            MasterScore*            score;

            bool              readBoolean();
            unsigned char     readUChar();
            char              readChar();
            unsigned short    readShort();
            int               readInt();
            void              skip(int len = 1);
            std::string       readString(int length = -1);

            struct ptSongInfo {
                  int               classification{ 0 };
                  int               releaseType{ 0 };
                  int               albumType{ 0 };
                  int               day{ 0 };
                  int               month{ 0 };
                  int               year{ 0 };
                  int               style{ 0 };
                  int               level{ 0 };
                  bool              liverecording{ 0 };
                  std::string       name;
                  std::string       interpret;
                  std::string       album;
                  std::string       author;
                  std::string       lyricist;
                  std::string       arrenger;
                  std::string       guitarTranscriber;
                  std::string       bassTranscriber;
                  std::string       lyrics;
                  std::string       guitarInstructions;
                  std::string       bassInstructions;
                  std::string       instructions;
                  std::string       copyright;
                  };

            struct ptComponent {
                  enum Type {
                        Empty,
                        GuitarIn,
                        Tempo,
                        Symbol,
                        Bar,
                        Note,
                        Beat,
                        TDirection
                        };
                  virtual Type type() {
                        return Empty;
                        };
                  };

            struct stRhytmSlash : public ptComponent {
                  int position {0};
                  int duration {0};
                  bool triplet{ false };
                  bool tripletend{ false };
                  bool dotted{ false };
                  bool doubleDotted{ false };
                  bool is_rest{ false };
                  };

            struct ptGuitarIn : public ptComponent {
                  int rhytmSlash{ true };
                  int staff{ 0 };
                  int trackinfo{ 0 };
                  int section{ 0 };
                  int position{ 0 };
                  Type type() {
                        return GuitarIn;
                        }
                  };

            struct ptSymbol : public ptComponent {
                  int value{ 0 };
                  ptSymbol(int val) : value(val) {}
                  Type type() {
                        return Symbol;
                        }
                  };

            struct ptBar : public ptComponent {
                  int         measureNo{ 0 };
                  int         repeatClose{ 0 };
                  bool  repeatStart{ false };

                  int         numerator{ 0 };
                  int         denominator{ 0 };
                  Type type() {
                        return Bar;
                        }
                  };
            std::vector<ptBar*> bars;

            struct ptNote : public ptComponent {
                  int               value{ 0 };
                  int               str{ 0 };
                  int               bend{0};
                  bool        tied{ false };
                  bool        dead{ false };
                  bool        hammer{ false };
                  int               slide{ 0 };
                  Type type() {
                        return Note;
                        }
                  };

            struct ptChord : public ptComponent {
                  int key;
                  int formula;
                  int modification;
                  int extra;
                  int top_fret;
                  std::vector<int> frets;
                  };

            struct ptChordText : public ptComponent {
                  int position;
                  int key;
                  int formula;
                  int formula_mod;
                  int extra;
                  };

            struct ptBeat : public ptComponent {
                  int                           position{ 0 };
                  int                           staff{ 0 };
                  int                           voice{ 0 };
                  int                           mmrest{ 0 };
                  bool                    isRest{ false };
                  int                           duration{ 0 };
                  int                           enters{ 0 };
                  int                           times{ 0 };
                  bool                    dotted{ false };
                  bool                    tuplet{ false };
                  bool                    doubleDotted{ false };
                  bool                    vibrato{ false };
                  bool                    grace{ false };
                  bool                    arpegioUp{ false };
                  bool                    arpegioDown{ false };
                  bool                    palmMute{ false };
                  bool                    accent{ false };
                  bool                    staccato{ false };
                  std::vector<ptNote> notes;

                  ptBeat(int _staff, int _voice) : staff(_staff), voice(_voice) {}
                  Type type() {
                        return Beat;
                        }
                  };

            struct ptDirection : public ptComponent {
                  enum Direction {
                        DIRECTION_CODA = 0,
                        DIRECTION_DOUBLE_CODA = 1,
                        DIRECTION_SEGNO = 2,
                        DIRECTION_SEGNO_SEGNO = 3,
                        DIRECTION_FINE = 4,
                        DIRECTION_DA_CAPO = 5,
                        DIRECTION_DAL_SEGNO = 6,
                        DIRECTION_DAL_SEGNO_SEGNO = 7,
                        DIRECTION_TO_CODA = 8,
                        DIRECTION_TO_DOUBLE_CODA = 9,
                        DIRECTION_DA_CAPO_AL_CODA = 10,
                        DIRECTION_DA_CAPO_AL_DOUBLE_CODA = 11,
                        DIRECTION_DAL_SEGNO_AL_CODA = 12,
                        DIRECTION_DAL_SEGNO_AL_DOUBLE_CODA = 13,
                        DIRECTION_DAL_SEGNO_SEGNO_AL_CODA = 14,
                        DIRECTION_DAL_SEGNO_SEGNO_AL_DOUBLE_CODA = 15,
                        DIRECTION_DA_CAPO_AL_FINE = 16,
                        DIRECTION_DAL_SEGNO_AL_FINE = 17,
                        DIRECTION_DAL_SEGNO_SEGNO_AL_FINE = 18
                        };

                  enum ActiveSym {
                        ACTIVE_SYMBOL_NONE = 0,
                        ACTIVE_SYMBOL_DC = 1,
                        ACTIVE_SYMBOL_DS = 2,
                        ACTIVE_SYMBOL_DSS = 3
                        };

                  Direction direction{ DIRECTION_CODA };
                  ActiveSym activeSymbol{ ACTIVE_SYMBOL_NONE };
                  int repeat{ 0 };

                  ptDirection(int dir, int sym, int rep) : direction((Direction)dir),
                        activeSymbol((ActiveSym)sym), repeat(rep) {}
                  Type type() {
                        return TDirection;
                        }
                  };

            struct ptPosition {
                  int position{ 0 };
                  std::vector<shared_ptr<ptComponent>> components;
                  void addComponent(ptComponent* c);
                  };

            struct TrackInfo {
                  int number{ 0 };
                  std::string name;
                  int instrument{ 0 };
                  int volume{ 0 };
                  int balance{ 0 };
                  int reverb{ 0 };
                  int chorus{ 0 };
                  int tremolo{ 0 };
                  int phaser{ 0 };
                  int capo{ 0 };
                  std::string tuningName;
                  int offset{ 0 };
                  std::vector<int> strings;
                  int notes_count{ 0 };
                  };

            typedef std::list<shared_ptr<ptBeat>> tBeatList;

            struct ptTrack;
            struct ptSection {
                  int                                 number{ 0 };
                  int                                 staffs{ 0 };
                  std::string                   partName;
                  char                          partMarker;
                  std::vector<ptPosition> positions;

                  std::vector<int>        staffMap;

                  int tempo{ 0 };

                  std::list<stRhytmSlash> rhytm;

                  std::map<int, ptChordText> chordTextMap;
                  std::vector<tBeatList>  beats;
                  std::list<shared_ptr<ptBar>> bars;
                  bool                          readed{ false };

                  void copyTracks(ptTrack*);

                  ptSection(int num);
                  ptPosition& getPosition(int pos);
                  int getNextPositionNumber();
                  };

            struct chordData {
                  int a[3];
                  bool operator > (const chordData& other) const {
                        if (a[0] == other.a[0]) {
                              if (a[1] == other.a[1]) {
                                    return a[2] > other.a[2];
                                    }
                              return a[1] > other.a[1];
                              }
                        return a[0] > other.a[0];
                        }
                  bool operator < (const chordData& other) const {
                        if (a[0] == other.a[0]) {
                              if (a[1] == other.a[1]) {
                                    return a[2] < other.a[2];
                                    }
                              return a[1] < other.a[1];
                              }
                        return a[0] < other.a[0];
                        }
                  bool operator == (const chordData& other) const {
                        return memcmp(a, other.a, 3 * sizeof(int)) == 0;
                        }
                  };

            struct ptTrack {
                  std::vector<TrackInfo> infos;
                  std::vector<ptSection> sections;
                  ptSection& getSection(int ind);
                  std::map < chordData, ptChord > diagramMap;

                  std::list<ptGuitarIn> guitar_ins;
                  };

            struct ptSong {
                  ptSongInfo info;
                  ptTrack track1;
                  ptTrack track2;
                  };

            void              readSongInfo(ptSongInfo& info);
            void              readDataInstruments(ptTrack& info);
            void              readTrackInfo(ptTrack& info);
            void              readGuitarIn(ptTrack& info);
            void              readTempoMarker(ptTrack& info);
            void              readSectionSymbol(ptTrack& info);
            void              readSection(ptSection& sec);
            void              readBarLine(ptSection& sec);
            void              readDirection(ptSection& sec);
            void              readTimeSignature(ptBar* bar);
            void              readStaff(int staff, ptSection& sec);
            void              readPosition(int staff, int voice, ptSection& sec);
            void              readNote(ptBeat* beat);
            void              readRehearsalSign(ptSection& sec);
            void              readChordText(ptSection& sec);
            void              readChord(ptTrack& si);
            void              readRhytmSlash(ptSection& sec);

            void              readFloatingText();
            void              readFontSettings();
            void              readDynamic();
            void              readKeySignature();

            bool              readVersion();
            int               readHeaderItems();

            std::vector<int> lastStaffMap;
            std::vector<int> getStaffMap(ptSection& sec);
            int               repeatCount{ 0 };
            void              addToScore(ptSection& sec);

            Measure*          createMeasure(ptBar* bar, int tick);
            void              fillMeasure(tBeatList& elist, Measure* measure, int staff, std::vector<Note*>&);

            int               staves{ 0 };

            ptTrack*          curTrack;
            int               staffInc{ 0 };
            char              lastPart{ 0 };

            ptSection*        cur_section;

	      std::vector<PalmMute*> _palmMutes;
            void addPalmMute(Chord*);

      public:
            PowerTab(QFile* f, MasterScore* s) : _file(f), score(s) {}
            Score::FileError read();
      };

}
