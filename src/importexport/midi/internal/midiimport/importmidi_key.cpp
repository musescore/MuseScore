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
#include "importmidi_key.h"
#include "importmidi_fraction.h"
#include "importmidi_chord.h"
#include "importmidi_inner.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/keylist.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "importmidi_operations.h"

// This simple key detection algorithm is from thesis
// "Inferring Score Level Musical Information From Low-Level Musical Data", 2004
// by Jürgen Kilian

using namespace mu::engraving;

namespace mu::iex::midi {
namespace MidiKey {
class KeyData
{
public:
    KeyData(Key key, int count)
        : key_(key), count_(count) {}

    Key key() const { return key_; }

    bool operator<(const KeyData& second) const
    {
        // choose key with max sum count of transitions
        if (count_ > second.count_) {
            return true;
        } else if (count_ < second.count_) {
            return false;
        }
        // if equal - prefer key with less accitential count
        return qAbs((int)key_) < qAbs((int)second.key_);
    }

private:
    Key key_;
    int count_;
};

void assignKeyListToStaff(const KeyList& kl, Staff* staff)
{
    Score* score = staff->score();
    const track_idx_t track = staff->idx() * VOICES;
    Key pkey = Key::C;

    for (auto it = kl.begin(); it != kl.end(); ++it) {
        const int tick = it->first;
        Key key  = it->second.key();
        if ((key == Key::C) && (key == pkey)) {       // don’t insert unnecessary C key // really?
            continue;
        }
        pkey = key;
        Measure* m = score->tick2measure(Fraction::fromTicks(tick));
        if (!m) {
            continue;
        }
        Segment* seg = m->getSegment(SegmentType::KeySig, Fraction::fromTicks(tick));
        KeySig* ks = Factory::createKeySig(seg);
        ks->setTrack(track);
        ks->setGenerated(false);
        ks->setKey(key);
        ks->mutldata()->setMag(staff->staffMag(Fraction::fromTicks(tick)));
        seg->add(ks);
    }
}

Key findKey(const QList<MTrack>& tracks)
{
    const int octave = 12;
    std::vector<int> counts(octave);

    for (const auto& track: tracks) {
        if (track.mtrack->drumTrack()) {
            continue;
        }
        for (auto it = track.chords.begin(); it != track.chords.end(); ++it) {
            const auto next = std::next(it);
            if (next == track.chords.end()) {
                continue;
            }

            for (const auto& note1: it->second.notes) {
                for (const auto& note2: next->second.notes) {
                    if (qAbs(note1.pitch - note2.pitch) == 1) {
                        ++counts[qMin(note1.pitch, note2.pitch) % octave];
                    }
                }
            }
        }
    }

    std::vector<KeyData> keys = {
        { Key::C_B, counts[3] + counts[10] },
        { Key::G_B, counts[10] + counts[5] },
        { Key::D_B, counts[5] + counts[0] },
        { Key::A_B, counts[0] + counts[7] },
        { Key::E_B, counts[7] + counts[2] },
        { Key::B_B, counts[2] + counts[9] },
        { Key::F, counts[9] + counts[4] },
        { Key::C, counts[4] + counts[11] },
        { Key::G, counts[11] + counts[6] },
        { Key::D, counts[6] + counts[1] },
        { Key::A, counts[1] + counts[8] },
        { Key::E, counts[8] + counts[3] },
        { Key::B, counts[3] + counts[10] },
        { Key::F_S, counts[10] + counts[5] },
        { Key::C_S, counts[5] + counts[0] }
    };

    std::sort(keys.begin(), keys.end());

    return keys[0].key();
}

void recognizeMainKeySig(QList<MTrack>& tracks)
{
    bool needToFindKey = false;

    const auto& opers = midiImportOperations;
    const bool isHuman = opers.data()->trackOpers.isHumanPerformance.value();
    if (isHuman) {
        needToFindKey = true;
    }

    if (!needToFindKey) {
        for (const MTrack& track: tracks) {
            if (track.mtrack->drumTrack()) {
                continue;
            }
            if (!track.hasKey) {
                needToFindKey = true;
                break;
            }
        }
    }
    if (!needToFindKey) {
        return;
    }

    const Key key = findKey(tracks);

    for (MTrack& track: tracks) {
        if (track.mtrack->drumTrack()) {
            continue;
        }

        if (!track.hasKey || isHuman) {
            KeySigEvent ke;
            Interval v = track.staff->part()->instrument()->transpose();
            Score* score = track.staff->score();
            ke.setConcertKey(key);
            if (!v.isZero() && !score->style().styleB(Sid::concertPitch)) {
                v.flip();
                ke.setKey(transposeKey(key, v));
            }

            KeyList& staffKeyList = *track.staff->keyList();
            staffKeyList[0] = ke;
            assignKeyListToStaff(staffKeyList, track.staff);
        }
    }
}
} // namespace MidiKey
} // namespace mu::iex::midi
