//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: pitchspelling.cpp 5163 2011-12-30 09:57:08Z wschweer $
//
//  Copyright (C) 2007-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

//  This file contains the implementation of an pitch spelling
//  algorithmus from Emilios Cambouropoulos as published in:
//  "Automatic Pitch Spelling: From Numbers to Sharps and Flats"

#include "event.h"
#include "note.h"
#include "key.h"
#include "pitchspelling.h"
#include "staff.h"
#include "chord.h"
#include "score.h"

//---------------------------------------------------------
//   tpcIsValid
//---------------------------------------------------------

bool tpcIsValid(int val)
      {
      return val >= -1 && val <= 33;
      }

//---------------------------------------------------------
//   step2tpc
//    alter =  -2 <= 2   (bb b - # ##)
//---------------------------------------------------------

int step2tpc(int step, int alter)
      {
      //    TPC - tonal pitch classes
      //    "line of fifth's" LOF

      static const int spellings[] = {
      //     bb  b   -   #  ##
             0,  7, 14, 21, 28,  // C
             2,  9, 16, 23, 30,  // D
             4, 11, 18, 25, 32,  // E
            -1,  6, 13, 20, 27,  // F
             1,  8, 15, 22, 29,  // G
             3, 10, 17, 24, 31,  // A
             5, 12, 19, 26, 33,  // B
             };

      int i = step * 5 + 2 + alter;
      if (i < 0 || (i >= int(sizeof(spellings)/sizeof(*spellings))))
            return INVALID_TPC;
      return spellings[i];
      };

//---------------------------------------------------------
//   tpc2step
//---------------------------------------------------------

int tpc2step(int tpc)
      {
      // 14 - C
      // 15 % 7 = 1
      //                            f  c  g  d  a  e  b
      static const int steps[7] = { 3, 0, 4, 1, 5, 2, 6 };
      return steps[(tpc+1) % 7];
      }

//---------------------------------------------------------
//   step2tpc
//---------------------------------------------------------

int step2tpc(const QString& stepName, int alter)
      {
      if (stepName.isEmpty())
            return INVALID_TPC;
      char c = stepName[0].toLower().toAscii();
      int r = -1;
      switch (c) {
            case 'c': r = 0; break;
            case 'd': r = 1; break;
            case 'e': r = 2; break;
            case 'f': r = 3; break;
            case 'g': r = 4; break;
            case 'a': r = 5; break;
            case 'b': r = 6; break;
            }
      return step2tpc(r, alter);
      }

//---------------------------------------------------------
//   tpc2pitch
//---------------------------------------------------------

int tpc2pitch(int tpc)
      {
      tpc += 1;

      static int pitches[] = {
//step:     F   C   G   D   A   E   B
            3, -2,  5,  0,  7,  2,  9,     // bb
            4, -1,  6,  1,  8,  3, 10,     // b
            5,  0,  7,  2,  9,  4, 11,     // -
            6,  1,  8,  3, 10,  5, 12,     // #
            7,  2,  9,  4, 11,  6, 13      // ##
            };
      if (tpc < 0 || tpc >= int(sizeof(pitches)/sizeof(*pitches))) {
            qDebug("tpc %d >= %d\n", tpc, int(sizeof(pitches)/sizeof(*pitches)));
            abort();
            }
      return pitches[tpc];
      }

//---------------------------------------------------------
//   printTpc
//    print note name
//---------------------------------------------------------

QString tpc2name(int tpc, bool germanNames)
      {
      const char names[] = "FCGDAEB";
      int acc   = ((tpc+1) / 7) - 2;
      QString s(names[(tpc + 1) % 7]);
      if (germanNames && (s == "B")) {
            if (acc == -1)
                  return s;
            s = "H";
            }
      switch(acc) {
            case -2: s += "bb"; break;
            case -1: s += "b";  break;
            case  0: break;
            case  1: s += "#";  break;
            case  2: s += "##"; break;
            default:
                  qDebug("tpc2name(%d): acc %d\n", tpc, acc);
                  s += "??";
                  break;
            }
      return s;
      }

void tpc2name(int tpc, bool germanNames, QChar* name, int* acc)
      {
      const char names[]  = "FCGDAEB";
      const char gnames[] = "FCGDAEH";
      *acc   = ((tpc+1) / 7) - 2;
      int idx = (tpc + 1) % 7;
      *name = QChar::fromLatin1(germanNames ? gnames[idx] : names[idx]);
      }

//---------------------------------------------------------
//   tpc2stepName
//---------------------------------------------------------

QString tpc2stepName(int tpc)
      {
      const char names[] = "FCGDAEB";
      return QString(names[(tpc + 1) % 7]);
      }

// table of alternative spellings for one octave
// each entry is the TPC of the note
//    tab1 does not contain double sharps
//    tab2 does not contain double flats

static const int tab1[24] = {
      14,  2,  // 60  C   Dbb
      21,  9,  // 61  C#  Db
      16,  4,  // 62  D   Ebb
      23, 11,  // 63  D#  Eb
      18,  6,  // 64  E   Fb
      13,  1,  // 65  F   Gbb
      20,  8,  // 66  F#  Gb
      15,  3,  // 67  G   Abb
      22, 10,  // 68  G#  Ab
      17,  5,  // 69  A   Bbb
      24, 12,  // 70  A#  Bb
      19,  7,  // 71  B   Cb
      };

static const int tab2[24] = {
      26, 14,  // 60  B#  C
      21,  9,  // 61  C#  Db
      28, 16,  // 62  C## D
      23, 11,  // 63  D#  Eb
      30, 18,  // 64  D## E
      25, 13,  // 65  E#  F
      20,  8,  // 66  F#  Gb
      27, 15,  // 67  F## G
      22, 10,  // 68  G#  Ab
      29, 17,  // 69  G## A
      24, 12,  // 70  A#  Bb
      31, 19,  // 71  A## B
      };

int intervalPenalty[13] = {
      0, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 3, 3
      };

//---------------------------------------------------------
//   enharmonicSpelling
//---------------------------------------------------------

static const bool enharmonicSpelling[15][34] = {
      {
//Ces f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      0, 0, 0, 0, 0, 0, 0, // b
      1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Ges f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 0, 0, 0, 0, 0, 0, // b
      0, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Des f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//As  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 1, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Es  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 1, 1, 1,
      0, 0, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Bb  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 1, 1,
      1, 0, 0, 1, 1, 1, 1, // #     // (ws) penalty for f#
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//F   f  c  g  d  a  e  b           // extra penalty for a# b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//C   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//G   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 0, 0, 0, 0, // b
      1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//D   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 0, 0, 0, // b
      1, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//A   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 1, 0, 0, // b
      1, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//E   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 1, 1, 0, // b
      1, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      0, 0, 1, 1, 1, 1, 1  // ##
      },
      {
//H   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 1, 1, 1, // b
      1, 1, 1, 1, 1, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Fis f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 1, 1, 1, // b
      100, 1, 1, 1, 1, 1, 0,
      0, 0, 0, 0, 0, 0, 0, // #
      0, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Cis f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b  //Fis
      100, 1, 1, 1, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, // #
      0, 0, 1, 1, 1, 1, 1  // ##
      }
      };

//---------------------------------------------------------
//   penalty
//---------------------------------------------------------

static int penalty(int lof1, int lof2, int k)
      {
      if (k < 0 || k >= 15) {
            qDebug("illegal key %d >= 15\n", k);
            abort();
            }
      if (lof1 < 0 || lof1 >= 34)
            abort();
      if (lof2 < 0 || lof2 >= 34)
            abort();
      int penalty  = enharmonicSpelling[k][lof1] * 4 + enharmonicSpelling[k][lof2] * 4;
      int distance = lof2 > lof1 ? lof2 - lof1 : lof1 - lof2;
      if (distance > 12)
            penalty += 3;
      else
            penalty += intervalPenalty[distance];
      return penalty;
      }

static const int WINDOW       = 9;
static const int WINDOW_SHIFT = 3;
static const int ASIZE        = 1024;   // 2 ** WINDOW

//---------------------------------------------------------
//   computeWindow
//---------------------------------------------------------

static int computeWindow(const QList<Event>& notes, int start, int end, int keyIdx)
      {
      int p   = 10000;
      int idx = -1;
      int pitch[10];

      if ((end-start) >= 10 || start == end)
            abort();

      int i = start;
      int k = 0;
      while (i < end)
            pitch[k++] = notes[i++].pitch() % 12;

      for (; k < 10; ++k)
            pitch[k] = pitch[k-1];

      for (int i = 0; i < 512; ++i) {
            int pa    = 0;
            int pb    = 0;
            int l     = pitch[0] * 2 + (i & 1);
            if ((l < 0) || (l >= int(sizeof(tab1)/sizeof(*tab1))))
                  abort();
            int lof1a = tab1[l];
            int lof1b = tab2[l];

            for (int k = 1; k < 10; ++k) {
                  int l = pitch[k] * 2 + ((i & (1 << k)) >> k);
                  if ((l < 0) || (l >= int(sizeof(tab1)/sizeof(*tab1))))
                        abort();
                  int lof2a = tab1[l];
                  int lof2b = tab2[l];
                  pa += penalty(lof1a, lof2a, keyIdx);
                  pb += penalty(lof1b, lof2b, keyIdx);
                  lof1a = lof2a;
                  lof1b = lof2b;
                  }
            if (pa < pb) {
                  if (pa < p) {
                        p   = pa;
                        idx = i;
                        }
                  }
            else {
                  if (pb < p) {
                        p   = pb;
                        idx = i * -1;
                        }
                  }
            }
      return idx;
      }

//---------------------------------------------------------
//   tpc
//---------------------------------------------------------

int tpc(int idx, int pitch, int opt)
      {
      const int* tab;
      if (opt < 0) {
            tab = tab2;
            opt *= -1;
            }
      else
            tab = tab1;
      int i = (pitch % 12) * 2 + ((opt & (1 << idx)) >> idx);
      if (i < 0 || i >= 24)
            abort();
      return tab[i];
      }

//---------------------------------------------------------
//   computeWindow
//---------------------------------------------------------

int computeWindow(const QList<Note*>& notes, int start, int end)
      {
      int p   = 10000;
      int idx = -1;
      int pitch[10];
      int key[10];

      int i = start;
      int k = 0;
      while (i < end) {
            pitch[k] = notes[i]->pitch() % 12;
            int tick = notes[i]->chord()->tick();
            key[k]   = notes[i]->staff()->keymap()->key(tick).accidentalType() + 7;
            if (key[k] < 0 || key[k] > 14) {
                  qDebug("illegal key at tick %d: %d, window %d-%d\n",
                     tick, key[k] - 7, start, end);
                  return 0;
                  // abort();
                  }
            ++k;
            ++i;
            }

      for (; k < 10; ++k) {
            pitch[k] = pitch[k-1];
            key[k]   = key[k-1];
            }

      for (int i = 0; i < 512; ++i) {
            int pa    = 0;
            int pb    = 0;
            int l     = pitch[0] * 2 + (i & 1);
            if (l < 0 || l > int(sizeof(tab1)/sizeof(*tab1)))
                  abort();
            int lof1a = tab1[l];
            int lof1b = tab2[l];

            for (int k = 1; k < 10; ++k) {
                  int l = pitch[k] * 2 + ((i & (1 << k)) >> k);
                  if (l < 0 || l > int(sizeof(tab1)/sizeof(*tab1)))
                        abort();
                  int lof2a = tab1[l];
                  int lof2b = tab2[l];
                  pa += penalty(lof1a, lof2a, key[k]);
                  pb += penalty(lof1b, lof2b, key[k]);
                  lof1a = lof2a;
                  lof1b = lof2b;
                  }
            if (pa < pb) {
                  if (pa < p) {
                        p   = pa;
                        idx = i;
                        }
                  }
            else {
                  if (pb < p) {
                        p   = pb;
                        idx = i * -1;
                        }
                  }
            }
/*      qDebug("compute window\n   ");
      for (int i = 0; i < 10; ++i)
            qDebug("%2d ", pitch[i]);
      qDebug("\n   ");
      for (int i = 0; i < 10; ++i)
            qDebug("%2d ", key[i]);
      qDebug("\n   ");
      for (int i = 0; i < 10; ++i)
            qDebug("%2d ", tpc(i, pitch[i], idx));
      qDebug("\n");
*/
      return idx;
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void spell(QList<Event>& notes, int key)
      {
      key += 7;

      int n = notes.size();
      if (n == 0)
            return;

      int start = 0;
      while (start < n) {
            int end = start + WINDOW;
            if (end > n)
                  end = n;
            int opt = computeWindow(notes, start, end, key);
            const int* tab;
            if (opt < 0) {
                  tab = tab2;
                  opt *= -1;
                  }
            else
                  tab = tab1;

            if (start == 0) {
                  notes[0].setTpc(tab[(notes[0].pitch() % 12) * 2 + (opt & 1)]);
                  if (n > 1)
                        notes[1].setTpc(tab[(notes[1].pitch() % 12) * 2 + ((opt & 2)>>1)]);
                  if (n > 2)
                        notes[2].setTpc(tab[(notes[2].pitch() % 12) * 2 + ((opt & 4)>>2)]);
                  }
            if ((end - start) >= 6) {
                  notes[start+3].setTpc(tab[(notes[start+3].pitch() % 12) * 2 + ((opt &  8) >> 3)]);
                  notes[start+4].setTpc(tab[(notes[start+4].pitch() % 12) * 2 + ((opt & 16) >> 4)]);
                  notes[start+5].setTpc(tab[(notes[start+5].pitch() % 12) * 2 + ((opt & 32) >> 5)]);
                  }
            if (end == n) {
                  int n = end - start;
                  int k;
                  switch(n - 6) {
                        case 3:
                              k = end - start - 3;
                              notes[end-3].setTpc(tab[(notes[end-3].pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 2:
                              k = end - start - 2;
                              notes[end-2].setTpc(tab[(notes[end-2].pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 1:
                              k = end - start - 1;
                              notes[end-1].setTpc(tab[(notes[end-1].pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        }
                  break;
                  }
            // advance to next window
            start += 3;
            }
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spellNotelist(QList<Note*>& notes)
      {
      int n = notes.size();

      int start = 0;
      while (start < n) {
            int end = start + WINDOW;
            if (end > n)
                  end = n;
            int opt = computeWindow(notes, start, end);
            const int* tab;
            if (opt < 0) {
                  tab = tab2;
                  opt *= -1;
                  }
            else
                  tab = tab1;

            if (start == 0) {
                  undoChangeTpc(notes[0], tab[(notes[0]->pitch() % 12) * 2 + (opt & 1)]);
                  if (n > 1)
                        undoChangeTpc(notes[1], tab[(notes[1]->pitch() % 12) * 2 + ((opt & 2)>>1)]);
                  if (n > 2)
                        undoChangeTpc(notes[2], tab[(notes[2]->pitch() % 12) * 2 + ((opt & 4)>>2)]);
                  }
            if ((end - start) >= 6) {
                  undoChangeTpc(notes[start+3], tab[(notes[start+3]->pitch() % 12) * 2 + ((opt &  8) >> 3)]);
                  undoChangeTpc(notes[start+4], tab[(notes[start+4]->pitch() % 12) * 2 + ((opt & 16) >> 4)]);
                  undoChangeTpc(notes[start+5], tab[(notes[start+5]->pitch() % 12) * 2 + ((opt & 32) >> 5)]);
                  }
            if (end == n) {
                  int n = end - start;
                  int k;
                  switch(n - 6) {
                        case 3:
                              k = end - start - 3;
                              undoChangeTpc(notes[end-3], tab[(notes[end-3]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 2:
                              k = end - start - 2;
                              undoChangeTpc(notes[end-2], tab[(notes[end-2]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 1:
                              k = end - start - 1;
                              undoChangeTpc(notes[end-1], tab[(notes[end-1]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        }
                  break;
                  }
            // advance to next window
            start += 3;
            }
      }

//---------------------------------------------------------
//   pitch2tpc2
//---------------------------------------------------------

int pitch2tpc2(int pitch, bool preferSharp)
      {
      int step = pitch % 12;

      static int ptab[15][12] = {
//              c  c#   d  d#   e   f  f#   g  g#   a  a#   b
            {  14, 21, 16, 23, 18, 13, 20, 15, 22, 17, 24, 19 },    // sharp
//              c  db  d  eb   e   f  gb   g  ab   a  bb   b
            {  14, 9, 16, 11, 18, 13, 8 , 15, 10, 17, 12, 19 },     // flat
            };
      return ptab[preferSharp ? 0 : 1][step];
      }

//---------------------------------------------------------
//   pitch2tpc
//    preferred pitch spelling depending on key
//    key -7 - +7
//---------------------------------------------------------

int pitch2tpc(int pitch, int key)
      {
      int step = pitch % 12;

      // ======TODO: fill table with reasonable values ===========

      static int ptab[15][12] = {
//              c  c#   d  d#   e   f  f#   g  g#   a  a#   b
            {  14,  9, 16, 11,  6, 13,  8, 15, 10, 17, 12,  7 },     // Ces
            {  14,  9, 16, 11,  6, 13,  8, 15, 10, 17, 12,  7 },     // Ges
            {  14,  9, 16, 11,  6, 13,  8, 15, 10, 17, 12,  7 },     // Des
            {  14,  9, 16, 11,  6, 13,  8, 15, 10, 17, 12,  7 },     // As
            {  14,  9, 16, 11,  6, 13,  8, 15, 10, 17, 12,  7 },     // Es
            {  14,  9, 16, 11,  6, 13,  8, 15, 10, 17, 12,  7 },     // B
            {  14,  9, 16, 11, 18, 13,  8, 15, 10, 17, 12,  7 },     // F

            {  14, 21, 16, 23, 18, 13, 20, 15, 22, 17, 24, 19 },     // C

            {  14, 21, 16, 23, 18, 13, 20, 15, 22, 17, 24, 19 },     // G
            {  14, 21, 16, 23, 18, 13, 20, 15, 22, 17, 24, 19 },     // D
            {  14, 21, 16, 23, 18, 13, 20, 15, 22, 17, 24, 19 },     // A
            {  14, 21, 16, 23, 18, 13, 20, 15, 22, 17, 24, 19 },     // E
            {  14, 21, 16, 23, 18, 13, 20, 15, 22, 17, 24, 19 },     // H
            {  14, 21, 16, 23, 18, 25, 20, 15, 22, 17, 24, 19 },     // Fis
            {  26, 21, 16, 23, 18, 25, 20, 15, 22, 17, 24, 19 },     // Cis
            };
// qDebug("pitch2tpc %d(%d) %d = %d\n", pitch, step, key, ptab[key+7][step]);
      return ptab[key+7][step];
      }

//---------------------------------------------------------
//   computeLine
//---------------------------------------------------------

int computeLine(int tpc, int pitch)
      {
      int line       = tpc2step(tpc) + (pitch/12) * 7;
      int tpcPitch   = tpc2pitch(tpc);
      if (tpcPitch < 0)
            line += 7;
      else
            line -= (tpcPitch / 12) * 7;
      return line;
      }

//---------------------------------------------------------
//   pitch2line
//---------------------------------------------------------

int pitch2line(int pitch)
      {
      int tpc = pitch2tpc(pitch);
      return tpc2step(tpc) + (pitch / 12) * 7;
      }


