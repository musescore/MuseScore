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

#include "instrumentsconverter.h"

#include "libmscore/instrument.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/drumset.h"

using namespace mu::notation;

Ms::Instrument InstrumentsConverter::convertInstrument(const Instrument& instrument)
{
    Ms::Instrument result;
    result.setAmateurPitchRange(instrument.amateurPitchRange.min, instrument.amateurPitchRange.max);
    result.setProfessionalPitchRange(instrument.professionalPitchRange.min, instrument.professionalPitchRange.max);

    for (const Ms::StaffName& longName : instrument.longNames) {
        result.addLongName(Ms::StaffName(longName.name(), longName.pos()));
    }

    for (const Ms::StaffName& shortName : instrument.shortNames) {
        result.addShortName(Ms::StaffName(shortName.name(), shortName.pos()));
    }

    result.setTrackName(instrument.name);
    result.setTranspose(instrument.transpose);
    result.setId(instrument.id);
    result.setInstrumentId(instrument.musicXMLid);

    if (instrument.useDrumset) {
        result.setDrumset(instrument.drumset ? instrument.drumset : Ms::smDrumset);
    }

    for (int i = 0; i < instrument.staves; ++i) {
        result.setClefType(i, instrument.clefs[i]);
    }

    result.setMidiActions(convertMidiActions(instrument.midiActions));
    result.setArticulation(instrument.midiArticulations);

    result.clearChannels();

    for (const InstrumentChannel& channel : instrument.channels) {
        result.appendChannel(new InstrumentChannel(channel));
    }

    result.setStringData(instrument.stringData);
    result.setSingleNoteDynamics(instrument.singleNoteDynamics);
    result.setTrait(instrument.trait);

    return result;
}

Instrument InstrumentsConverter::convertInstrument(const Ms::Instrument& instrument)
{
    Instrument result;
    result.amateurPitchRange = PitchRange(instrument.minPitchA(), instrument.maxPitchA());
    result.professionalPitchRange = PitchRange(instrument.minPitchP(), instrument.maxPitchP());

    for (const Ms::StaffName& longName: instrument.longNames()) {
        result.longNames << StaffName(longName.name(), longName.pos());
    }

    for (const Ms::StaffName& shortName: instrument.shortNames()) {
        result.shortNames << StaffName(shortName.name(), shortName.pos());
    }

    result.name = instrument.trackName();
    result.transpose = instrument.transpose();
    result.id = instrument.getId();
    result.musicXMLid = instrument.instrumentId();
    result.useDrumset = instrument.useDrumset();
    result.drumset = instrument.drumset();

    result.staves = instrument.cleffTypeCount();
    for (int i = 0; i < instrument.cleffTypeCount(); ++i) {
        result.clefs[i] = instrument.clefType(i);
    }

    result.midiActions = convertMidiActions(instrument.midiActions());
    result.midiArticulations = instrument.articulation();

    for (const InstrumentChannel* channel : instrument.channel()) {
        result.channels.append(*channel);
    }

    result.stringData = *instrument.stringData();
    result.singleNoteDynamics = instrument.singleNoteDynamics();
    result.trait = instrument.trait();

    return result;
}

Instrument InstrumentsConverter::convertInstrument(const Ms::InstrumentTemplate& templ)
{
    Ms::Instrument msInstrument = Ms::Instrument::fromTemplate(&templ);
    Instrument result = convertInstrument(msInstrument);
    result.templateId = templ.id;
    result.familyId = templ.family ? templ.family->id : QString();
    result.sequenceOrder = templ.sequenceOrder;

    for (const Ms::InstrumentGenre* msGenre : templ.genres) {
        result.genreIds << msGenre->id;
    }

    return result;
}

QList<Ms::NamedEventList> InstrumentsConverter::convertMidiActions(const MidiActionList& midiActions)
{
    QList<Ms::NamedEventList> result;

    for (const MidiAction& action: midiActions) {
        Ms::NamedEventList event;
        event.name = action.name;
        event.descr = action.description;

        for (const midi::Event& midiEvent: action.events) {
            Ms::MidiCoreEvent midiCoreEvent;
            midiCoreEvent.setType(static_cast<uchar>(midiEvent.type()));
            midiCoreEvent.setChannel(midiCoreEvent.channel());
            //!FIXME
            //midiCoreEvent.setData(midiEvent.a, midiEvent.b);
            event.events.push_back(midiCoreEvent);
        }
    }

    return result;
}

MidiActionList InstrumentsConverter::convertMidiActions(const QList<Ms::NamedEventList>& midiActions)
{
    MidiActionList result;

    for (const Ms::NamedEventList& coreAction: midiActions) {
        MidiAction action;
        action.name = coreAction.name;
        action.description = coreAction.descr;

        for (const Ms::MidiCoreEvent& midiCoreEvent: coreAction.events) {
            midi::Event midiEvent(midiCoreEvent.channel(),
                                  static_cast<midi::EventType>(midiCoreEvent.type()),
                                  midiCoreEvent.dataA(),
                                  midiCoreEvent.dataB()
                                  );

            action.events.push_back(midiEvent);
        }
    }

    return result;
}
