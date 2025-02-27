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

#include "exportmidi.h"

#include "engraving/dom/key.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/synthesizerstate.h"
#include "engraving/dom/tempo.h"

#include "engraving/compat/midi/event.h"
#include "engraving/compat/midi/compatmidirender.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::midi {
//---------------------------------------------------------
//   writeHeader
//---------------------------------------------------------

void ExportMidi::writeHeader(const CompatMidiRendererInternal::Context& context)
{
    if (m_midiFile.tracks().empty()) {
        return;
    }
    MidiTrack& track  = m_midiFile.tracks().front();

    //--------------------------------------------
    //    write track names
    //--------------------------------------------

    int staffIdx = 0;
    for (auto& track1: m_midiFile.tracks()) {
        Staff* staff  = m_score->staff(staffIdx);

        muse::ByteArray partName = staff->partName().toUtf8();
        size_t len = partName.size() + 1;
        unsigned char* data = new unsigned char[len];

        memcpy(data, partName.constData(), len);

        MidiEvent ev;
        ev.setType(ME_META);
        ev.setMetaType(META_TRACK_NAME);
        ev.setEData(data);
        ev.setLen(static_cast<int>(len));

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
                LOGD("ExportMidi: unknown time signature %s",
                     qPrintable(ts.toString()));
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
            track.insert(CompatMidiRender::tick(context, is->first + tickOffset), ev);
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
                Key key = ik->second.concertKey();           // -7 -- +7
                ev.setMetaType(META_KEY_SIGNATURE);
                ev.setLen(2);
                unsigned char* data = new unsigned char[2];
                data[0] = int(key);
                data[1] = 0;          // major
                ev.setEData(data);
                int tick = ik->first + tickOffset;
                track1.insert(CompatMidiRender::tick(context, tick), ev);
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

    if (!context.applyCaesuras) {
        return;
    }

    const TempoMap* tempomap = context.pauseMap->tempomapWithPauses();
    BeatsPerSecond tempoMultiplier = tempomap->tempoMultiplier();
    for (auto it = tempomap->cbegin(); it != tempomap->cend(); ++it) {
        MidiEvent ev;
        ev.setType(ME_META);
        //
        // compute midi tempo: microseconds / quarter note
        //
        int tempo = lrint((1.0 / it->second.tempo.val * tempoMultiplier.val) * 1000000.0);

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
    m_midiFile.setDivision(Constants::DIVISION);
    m_midiFile.setFormat(1);
    std::vector<MidiTrack>& tracks = m_midiFile.tracks();

    for (size_t i = 0; i < m_score->nstaves(); ++i) {
        tracks.push_back(MidiTrack());
    }

    EventsHolder events;
    CompatMidiRendererInternal::Context context;
    context.eachStringHasChannel = false;
    context.instrumentsHaveEffects = false;
    context.harmonyChannelSetting = CompatMidiRendererInternal::HarmonyChannelSetting::DEFAULT;
    context.sndController = CompatMidiRender::getControllerForSnd(m_score, synthState.ccToUse());
    context.useDefaultArticulations = false;
    context.applyCaesuras = true;

    CompatMidiRender::renderScore(m_score, events, context, midiExpandRepeats);

    writeHeader(context);

    staff_idx_t staffIdx = 0;
    for (auto& track: tracks) {
        Staff* staff = m_score->staff(staffIdx);
        Part* part   = staff->part();

        track.setOutPort(part->midiPort());
        track.setOutChannel(part->midiChannel());

        // Pass through the all instruments in the part
        for (const auto& pair : part->instruments()) {
            // Pass through the all channels of the instrument
            // "normal", "pizzicato", "tremolo" for Strings,
            // "normal", "mute" for Trumpet
            for (const InstrChannel* instrChan : pair.second->channel()) {
                const InstrChannel* ch = part->masterScore()->playbackChannel(instrChan);
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

                for (size_t e = 0; e < events.size(); ++e) {
                    auto& multimap = events[e];
                    for (auto& item : multimap) {
                        const NPlayEvent& event = item.second;
                        if (event.isMuted()) {
                            continue;
                        }
                        if (event.discard() == staffIdx + 1 && event.velo() > 0) {
                            // turn note off so we can restrike it in another track
                            track.insert(CompatMidiRender::tick(context, item.first), MidiEvent(ME_NOTEON, channel,
                                                                                                event.pitch(), 0));
                        }

                        staff_idx_t equivalentStaffIdx = staffIdx;
                        for (Staff* st : m_score->masterScore()->staves()) {
                            if (staff->id() == st->id()) {
                                equivalentStaffIdx = st->idx();
                            }
                        }

                        if (event.getOriginatingStaff() != equivalentStaffIdx) {
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
                                track.insert(CompatMidiRender::tick(context, item.first), MidiEvent(ME_NOTEON, channel,
                                                                                                    event.note()->pitch(),
                                                                                                    event.velo()));
                            } else {
                                track.insert(CompatMidiRender::tick(context, item.first), MidiEvent(ME_NOTEON, channel,
                                                                                                    event.pitch(), event.velo()));
                            }
                        } else if (event.type() == ME_CONTROLLER) {
                            track.insert(CompatMidiRender::tick(context, item.first), MidiEvent(ME_CONTROLLER, channel,
                                                                                                event.controller(),
                                                                                                event.value()));
                        } else if (event.type() == ME_PITCHBEND) {
                            track.insert(CompatMidiRender::tick(context, item.first), MidiEvent(ME_PITCHBEND, channel,
                                                                                                event.dataA(), event.dataB()));
                        } else {
                            LOGD("writeMidi: unknown midi event 0x%02x", event.type());
                        }
                    }
                }
            }
        }

        // Export lyrics and RehearsalMarks as Meta events
        for (const RepeatSegment* rs : m_score->repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len();
            int tickOffset = rs->utick - rs->tick;

            // export Lyrics
            SegmentType st = SegmentType::ChordRest;
            for (Segment* seg = rs->firstMeasure()->first(st); seg && seg->tick().ticks() < endTick; seg = seg->next1(st)) {
                for (track_idx_t i = part->startTrack(); i < part->endTrack(); ++i) {
                    ChordRest* cr = toChordRest(seg->element(i));
                    if (cr) {
                        for (const auto& lyric : cr->lyrics()) {
                            muse::ByteArray lyricText = lyric->plainText().toUtf8();
                            size_t len = lyricText.size() + 1;
                            unsigned char* data = new unsigned char[len];

                            memcpy(data, lyricText.constData(), len);

                            MidiEvent ev;
                            ev.setType(ME_META);
                            ev.setMetaType(META_LYRIC);
                            ev.setEData(data);
                            ev.setLen(static_cast<int>(len));

                            int tick = cr->tick().ticks() + tickOffset;
                            track.insert(CompatMidiRender::tick(context, tick), ev);
                        }
                    }
                }
            }

            // export RehearsalMarks only for first track
            if (staffIdx == 0) {
                for (Segment* seg = rs->firstMeasure()->first(Segment::CHORD_REST_OR_TIME_TICK_TYPE);
                     seg && seg->tick().ticks() < endTick;
                     seg = seg->next1(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
                    for (EngravingItem* e : seg->annotations()) {
                        if (e->isRehearsalMark()) {
                            RehearsalMark* r = toRehearsalMark(e);
                            muse::ByteArray rText = r->plainText().toUtf8();
                            size_t len = rText.size() + 1;
                            unsigned char* data = new unsigned char[len];

                            memcpy(data, rText.constData(), len);

                            MidiEvent ev;
                            ev.setType(ME_META);
                            ev.setMetaType(META_MARKER);
                            ev.setEData(data);
                            ev.setLen(static_cast<int>(len));

                            int tick = r->segment()->tick().ticks() + tickOffset;
                            track.insert(CompatMidiRender::tick(context, tick), ev);
                        }
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
}
