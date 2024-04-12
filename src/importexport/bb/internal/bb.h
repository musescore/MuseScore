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

#ifndef MU_IEX_BB_BB_H
#define MU_IEX_BB_BB_H

#include "engraving/compat/midi/event.h"
#include "engraving/dom/score.h"
#include "engraving/dom/sig.h"

namespace mu::iex::bb {
constexpr int MAX_BARS = 255;

class BBFile;
struct MNote;

//---------------------------------------------------------
//   BBTrack
//---------------------------------------------------------

class BBTrack
{
    BBFile* bb;
    engraving::EventList _events;
    int _outChannel;
    bool _drumTrack;

    void quantize(int startTick, int endTick, engraving::EventList* dst);

public:
    BBTrack(BBFile*);
    ~BBTrack();
    bool empty() const;
    const engraving::EventList events() const { return _events; }
    engraving::EventList& events() { return _events; }
    int outChannel() const { return _outChannel; }
    void setOutChannel(int n) { _outChannel = n; }
    void insert(const engraving::Event& e) { _events.insert(e); }
    void append(const engraving::Event& e) { _events.push_back(e); }

    void findChords();
    int separateVoices(int);
    void cleanup();

    friend class BBFile;
};

//---------------------------------------------------------
//   BBChord
//---------------------------------------------------------

struct BBChord {
    int beat;
    unsigned char bass;
    unsigned char root;
    unsigned char extension;
    BBChord()
    {
        beat = 0;
        bass = 0;
        root = 0;
        extension = 0;
    }
};

//---------------------------------------------------------
//   BBStyle
//---------------------------------------------------------

struct BBStyle {
    int timesigZ, timesigN;
};

//---------------------------------------------------------
//   BBStyle
//---------------------------------------------------------

static const BBStyle styles[] = {
    { 4, 4 },      // Jazz Swing
    { 12, 8 },     // Country 12/8
    { 4, 4 },      // Country 4/4
    { 4, 4 },      // Bossa Nova
    { 4, 4 },      // Ethnic
    { 4, 4 },      // Blues Shuffle
    { 4, 4 },      // Blues Straight
    { 3, 4 },      // Waltz
    { 4, 4 },      // Pop Ballad
    { 4, 4 },      // should be Rock Shuffle
    { 4, 4 },      // lite Rock
    { 4, 4 },      // medium Rock
    { 4, 4 },      // Heavy Rock
    { 4, 4 },      // Miami Rock
    { 4, 4 },      // Milly Pop
    { 4, 4 },      // Funk
    { 3, 4 },      // Jazz Waltz
    { 4, 4 },      // Rhumba
    { 4, 4 },      // Cha Cha
    { 4, 4 },      // Bouncy
    { 4, 4 },      // Irish
    { 12, 8 },     // Pop Ballad 12/8
    { 12, 8 },     // Country12/8 old
    { 4, 4 },      // Reggae
};

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

class BBFile
{
    QString _path;
    unsigned char _version;
    char* _title;
    int _style, _key, _bpm;

    unsigned char _barType[MAX_BARS];
    QList<BBChord> _chords;

    int _startChorus;
    int _endChorus;
    int _repeats;
    char* _styleName;
    QList<BBTrack*> _tracks;
    int _measures;
    engraving::TimeSigMap _siglist;

    QByteArray ba;
    const unsigned char* a;
    int size;
    int bbDivision;

    int timesigZ() { return styles[_style].timesigZ; }
    int timesigN() { return styles[_style].timesigN; }
    engraving::Fraction processPendingNotes(engraving::Score*, QList<MNote*>* notes, const engraving::Fraction&, int);

public:
    BBFile();
    ~BBFile();
    bool read(const QString&);
    QList<BBTrack*>* tracks() { return &_tracks; }
    int measures() const { return _measures; }
    const char* title() const { return _title; }
    engraving::TimeSigMap siglist() const { return _siglist; }
    QList<BBChord> chords() { return _chords; }
    int startChorus() const { return _startChorus; }
    int endChorus() const { return _endChorus; }
    int repeats() const { return _repeats; }
    int key() const { return _key; }
    void convertTrack(engraving::Score* score, BBTrack* track, int staffIdx);
};
}

#endif // MU_IEX_BB_BB_H
