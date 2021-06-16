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

#include "exportmidi.h"

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/tempo.h"
#include "libmscore/sig.h"
#include "libmscore/key.h"
#include "libmscore/text.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "libmscore/synthesizerstate.h"

#include "framework/midi_old/midifile.h"
#include "framework/midi_old/event.h"

namespace Ms {
//---------------------------------------------------------
//   writeHeader
//---------------------------------------------------------

void ExportMidi::writeHeader()
{
    if (m_midiFile.tracks().isEmpty()) {
        return;
    }
    MidiTrack& track  = m_midiFile.tracks().front();

    //--------------------------------------------
    //    write track names
    //--------------------------------------------

    int staffIdx = 0;
    for (auto& track1: m_midiFile.tracks()) {
        Staff* staff  = m_score->staff(staffIdx);

        QByteArray partName = staff->partName().toUtf8();
        int len = partName.length() + 1;
        unsigned char* data = new unsigned char[len];

        memcpy(data, partName.data(), len);

        MidiEvent ev;
        ev.setType(ME_META);
        ev.setMetaType(META_TRACK_NAME);
        ev.setEData(data);
        ev.setLen(len);

        track1.insert(0, ev);

        ++staffIdx;
    }

    //--------------------------------------------
    //    write time signature
    //--------------------------------------------

    TimeSigMap* sigmap = m_score->sigmap();
    for (const RepeatSegment* rs : m_score->repeatList()) {
        int startTick  = rs->tick;
        int endTick    = startTick + rs->len();
        int tickOffset = rs->utick - rs->tick;

        auto bs = sigmap->lower_bound(startTick);
        auto es = sigmap->lower_bound(endTick);

        for (auto is = bs; is != es; ++is) {
            SigEvent se   = is->second;
            unsigned char* data = new unsigned char[4];
            Fraction ts(se.timesig());
            data[0] = ts.numerator();
            int n;
            switch (ts.denominator()) {
            case 1:  n = 0;
                break;
            case 2:  n = 1;
                break;
            case 4:  n = 2;
                break;
            case 8:  n = 3;
                break;
            case 16: n = 4;
                break;
            case 32: n = 5;
                break;
            default:
                n = 2;
                qDebug("ExportMidi: unknown time signature %s",
                       qPrintable(ts.print()));
                break;
            }
            data[1] = n;
            data[2] = 24;
            data[3] = 8;

            MidiEvent ev;
            ev.setType(ME_META);
            ev.setMetaType(META_TIME_SIGNATURE);
            ev.setEData(data);
            ev.setLen(4);
            track.insert(m_pauseMap.addPauseTicks(is->first + tickOffset), ev);
        }
    }

    //---------------------------------------------------
    //    write key signatures
    //    assume every staff corresponds to a midi track
    //---------------------------------------------------

    staffIdx = 0;
    for (auto& track1: m_midiFile.tracks()) {
        Staff* staff  = m_score->staff(staffIdx);
        KeyList* keys = staff->keyList();

        bool initialKeySigFound = false;
        for (const RepeatSegment* rs : m_score->repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len();
            int tickOffset = rs->utick - rs->tick;

            auto sk = keys->lower_bound(startTick);
            auto ek = keys->lower_bound(endTick);

            for (auto ik = sk; ik != ek; ++ik) {
                MidiEvent ev;
                ev.setType(ME_META);
                Key key       = ik->second.key();           // -7 -- +7
                ev.setMetaType(META_KEY_SIGNATURE);
                ev.setLen(2);
                unsigned char* data = new unsigned char[2];
                data[0]   = int(key);
                data[1]   = 0;          // major
                ev.setEData(data);
                int tick = ik->first + tickOffset;
                track1.insert(m_pauseMap.addPauseTicks(tick), ev);
                if (tick == 0) {
                    initialKeySigFound = true;
                }
            }
        }

        // fall back write a default C keysig if no initial keysig found
        if (!initialKeySigFound) {
            MidiEvent ev;
            ev.setType(ME_META);
            int key = 0;
            ev.setMetaType(META_KEY_SIGNATURE);
            ev.setLen(2);
            unsigned char* data = new unsigned char[2];
            data[0]   = key;
            data[1]   = 0;        // major
            ev.setEData(data);
            track1.insert(0, ev);
        }

        ++staffIdx;
    }

    //--------------------------------------------
    //    write tempo changes from PauseMap
    //     don't need to unwind or add pauses as this was done already
    //--------------------------------------------

    TempoMap* tempomap = m_pauseMap.tempomapWithPauses;
    qreal relTempo = tempomap->relTempo();
    for (auto it = tempomap->cbegin(); it != tempomap->cend(); ++it) {
        MidiEvent ev;
        ev.setType(ME_META);
        //
        // compute midi tempo: microseconds / quarter note
        //
        int tempo = lrint((1.0 / (it->second.tempo * relTempo)) * 1000000.0);

        ev.setMetaType(META_TEMPO);
        ev.setLen(3);
        unsigned char* data = new unsigned char[3];
        data[0]   = tempo >> 16;
        data[1]   = tempo >> 8;
        data[2]   = tempo;
        ev.setEData(data);
        track.insert(it->first, ev);
    }
}

//---------------------------------------------------------
//  write
//    export midi file
//    return false on error
//
//    The 3rd and 4th versions of write create a temporary, uninitialized synth state
//    so we can render the midi - it should fall back correctly to the defaults, with a warning.
//    These should only be used for tests. When actually rendering midi as a user action,
//    make sure to use the 1st and 2nd versions, passing the global musescore synth state
//    from mscore->synthesizerState() as the synthState parameter.
//---------------------------------------------------------

bool ExportMidi::write(QIODevice* device, bool midiExpandRepeats, bool exportRPNs, const SynthesizerState& synthState)
{
    m_midiFile.setDivision(MScore::division);
    m_midiFile.setFormat(1);
    QList<MidiTrack>& tracks = m_midiFile.tracks();

    for (int i = 0; i < m_score->nstaves(); ++i) {
        tracks.append(MidiTrack());
    }

    EventMap events;
    m_score->renderMidi(&events, false, midiExpandRepeats, synthState);

    m_pauseMap.calculate(m_score);
    writeHeader();

    int staffIdx = 0;
    for (auto& track: tracks) {
        Staff* staff = m_score->staff(staffIdx);
        Part* part   = staff->part();

        track.setOutPort(part->midiPort());
        track.setOutChannel(part->midiChannel());

        // Pass through the all instruments in the part
        const InstrumentList* il = part->instruments();
        for (auto j = il->begin(); j != il->end(); j++) {
            // Pass through the all channels of the instrument
            // "normal", "pizzicato", "tremolo" for Strings,
            // "normal", "mute" for Trumpet
            for (const Channel* instrChan : j->second->channel()) {
                const Channel* ch = part->masterScore()->playbackChannel(instrChan);
                char port    = part->masterScore()->midiPort(ch->channel());
                char channel = part->masterScore()->midiChannel(ch->channel());

                if (staff->isTop()) {
                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_RESET_ALL_CTRL, 0));
                    // We need this to get the correct pitch of bends
                    // Hidden under preferences because some software
                    // crashes when receiving RPNs: https://musescore.org/en/node/37431
                    if (channel != 9 && exportRPNs) {
                        // set pitch bend sensitivity to 12 semitones:
                        track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_LRPN, 0));
                        track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HRPN, 0));
                        track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HDATA, 12));

                        // reset fine tuning
                        /*track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_LRPN, 1));
                        track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HRPN, 0));
                        track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HDATA, 64));*/

                        // deactivate rpn
                        track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_LRPN, 127));
                        track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_HRPN, 127));
                    }

                    if (ch->program() != -1) {
                        track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_PROGRAM, ch->program()));
                    }
                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_VOLUME, ch->volume()));
                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_PANPOT, ch->pan()));
                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_REVERB_SEND, ch->reverb()));
                    track.insert(0, MidiEvent(ME_CONTROLLER, channel, CTRL_CHORUS_SEND, ch->chorus()));
                }

                // Export port to MIDI META event
                if (track.outPort() >= 0 && track.outPort() <= 127) {
                    MidiEvent ev;
                    ev.setType(ME_META);
                    ev.setMetaType(META_PORT_CHANGE);
                    ev.setLen(1);
                    unsigned char* data = new unsigned char[1];
                    data[0] = int(track.outPort());
                    ev.setEData(data);
                    track.insert(0, ev);
                }

                for (auto i = events.begin(); i != events.end(); ++i) {
                    const NPlayEvent& event = i->second;

                    if (event.isMuted()) {
                        continue;
                    }
                    if (event.discard() == staffIdx + 1 && event.velo() > 0) {
                        // turn note off so we can restrike it in another track
                        track.insert(m_pauseMap.addPauseTicks(i->first), MidiEvent(ME_NOTEON, channel,
                                                                                   event.pitch(), 0));
                    }

                    if (event.getOriginatingStaff() != staffIdx) {
                        continue;
                    }

                    if (event.discard() && event.velo() == 0) {
                        // ignore noteoff but restrike noteon
                        continue;
                    }

                    if (!exportRPNs && event.type() == ME_CONTROLLER && event.portamento()) {
                        // ignore portamento control events if exportRPN isn't switched on
                        continue;
                    }

                    char eventPort    = m_score->masterScore()->midiPort(event.channel());
                    char eventChannel = m_score->masterScore()->midiChannel(event.channel());
                    if (port != eventPort || channel != eventChannel) {
                        continue;
                    }

                    if (event.type() == ME_NOTEON) {
                        // use the note values instead of the event values if portamento is suppressed
                        if (!exportRPNs && event.portamento()) {
                            track.insert(m_pauseMap.addPauseTicks(i->first), MidiEvent(ME_NOTEON, channel,
                                                                                       event.note()->pitch(), event.velo()));
                        } else {
                            track.insert(m_pauseMap.addPauseTicks(i->first), MidiEvent(ME_NOTEON, channel,
                                                                                       event.pitch(), event.velo()));
                        }
                    } else if (event.type() == ME_CONTROLLER) {
                        track.insert(m_pauseMap.addPauseTicks(i->first), MidiEvent(ME_CONTROLLER, channel,
                                                                                   event.controller(), event.value()));
                    } else if (event.type() == ME_PITCHBEND) {
                        track.insert(m_pauseMap.addPauseTicks(i->first), MidiEvent(ME_PITCHBEND, channel,
                                                                                   event.dataA(), event.dataB()));
                    } else {
                        qDebug("writeMidi: unknown midi event 0x%02x", event.type());
                    }
                }
            }
        }
        ++staffIdx;
    }
    return !m_midiFile.write(device);
}

bool ExportMidi::write(const QString& name, bool midiExpandRepeats, bool exportRPNs, const SynthesizerState& synthState)
{
    m_file.setFileName(name);
    if (!m_file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return write(&m_file, midiExpandRepeats, exportRPNs, synthState);
}

bool ExportMidi::write(QIODevice* device, bool midiExpandRepeats, bool exportRPNs)
{
    SynthesizerState ss;
    return write(device, midiExpandRepeats, exportRPNs, ss);
}

bool ExportMidi::write(const QString& name, bool midiExpandRepeats, bool exportRPNs)
{
    SynthesizerState ss;
    return write(name, midiExpandRepeats, exportRPNs, ss);
}

//---------------------------------------------------------
//   PauseMap::calculate
//    MIDI files cannot contain pauses so insert extra ticks and tempo changes instead.
//    The PauseMap and new TempoMap are fully unwound to account for pauses on repeats.
//---------------------------------------------------------

void ExportMidi::PauseMap::calculate(const Score* s)
{
    Q_ASSERT(s);
    TimeSigMap* sigmap = s->sigmap();
    TempoMap* tempomap = s->tempomap();

    this->insert(std::pair<const int, int>(0, 0));    // can't start with a pause

    tempomapWithPauses = new TempoMap();
    tempomapWithPauses->setRelTempo(tempomap->relTempo());

    for (const RepeatSegment* rs : s->repeatList()) {
        int startTick  = rs->tick;
        int endTick    = startTick + rs->len();
        int tickOffset = rs->utick - rs->tick;

        auto se = tempomap->lower_bound(startTick);
        auto ee = tempomap->lower_bound(endTick + 1);   // +1 to include first tick of next RepeatSegment

        for (auto it = se; it != ee; ++it) {
            int tick = it->first;
            int utick = tick + tickOffset;

            if (it->second.pause == 0.0) {
                // We have a regular tempo change. Don't include tempo change from first tick of next RepeatSegment (it will be included later).
                if (tick != endTick) {
                    tempomapWithPauses->insert(std::pair<const int, TEvent>(this->addPauseTicks(utick), it->second));
                }
            } else {
                // We have a pause event. Don't include pauses from first tick of current RepeatSegment (it was included in the previous one).
                if (tick != startTick) {
                    Fraction timeSig(sigmap->timesig(tick).timesig());
                    qreal quarterNotesPerMeasure = (4.0 * timeSig.numerator()) / timeSig.denominator();
                    int ticksPerMeasure =  quarterNotesPerMeasure * MScore::division;           // store a full measure of ticks to keep barlines in same places
                    tempomapWithPauses->setTempo(this->addPauseTicks(utick), quarterNotesPerMeasure / it->second.pause);           // new tempo for pause
                    this->insert(std::pair<const int, int>(utick, ticksPerMeasure + this->offsetAtUTick(utick)));            // store running total of extra ticks
                    tempomapWithPauses->setTempo(this->addPauseTicks(utick), it->second.tempo);           // restore previous tempo
                }
            }
        }
    }
}

//---------------------------------------------------------
//   PauseMap::offsetAtUTick
//    In total, how many extra ticks have been inserted prior to this utick.
//---------------------------------------------------------

int ExportMidi::PauseMap::offsetAtUTick(int utick) const
{
    Q_ASSERT(!this->empty());   // make sure calculate was called
    auto i = upper_bound(utick);
    if (i != begin()) {
        --i;
    }
    return i->second;
}
}
