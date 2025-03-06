#include "gp7dombuilder.h"

#include <vector>

#include "global/log.h"

using namespace muse;

namespace {
std::map<String, int> RSE2MidiProgram = {
    // Acoustic Guitars
    { u"Stringed/Acoustic Guitars/Steel Guitar", 25 },
    { u"Stringed/Acoustic Guitars/12 String Steel", 25 },
    { u"Stringed/Acoustic Guitars/Nylon Guitar", 24 },
    { u"Stringed/Acoustic Guitars/Resonator", 25 },
    // Electric Guitars
    { u"Stringed/Electric Guitars/Clean Guitar", 27 },
    { u"Stringed/Electric Guitars/Jazz Guitar", 26 },
    { u"Stringed/Electric Guitars/12 Strings Electric Guitar", 27 },
    { u"Stringed/Electric Guitars/Overdrive Guitar", 29 },
    { u"Stringed/Electric Guitars/Distortion Guitar", 30 },
    { u"Stringed/Electric Guitars/Electric Sitar", 104 },
    // Bass Guitars
    { u"Stringed/Basses/Clean Bass", 33 },
    { u"Stringed/Basses/Slap Bass", 37 },
    { u"Stringed/Basses/Crunch Bass", 33 },
    { u"Stringed/Basses/Acoustic Bass", 32 },
    { u"Stringed/Basses/Fretless Bass", 35 },
    { u"Stringed/Basses/Upright Bass", 32 },
    { u"Stringed/Basses/Synth Bass", 39 },
    // Other Stringed instruments
    { u"Stringed/Other Stringed Instruments/Ukulele", 24 },
    { u"Stringed/Other Stringed Instruments/Banjo", 105 },
    { u"Stringed/Other Stringed Instruments/Mandolin", 25 },
    { u"Stringed/Other Stringed Instruments/Pedal Steel", 26 },
    // Keyboard
    { u"Orchestra/Keyboard/Acoustic Piano", 1 },
    { u"Orchestra/Keyboard/Electric Piano", 4 },
    { u"Orchestra/Keyboard/Organ", 16 },
    { u"Orchestra/Keyboard/Clavinet", 6 },
    { u"Orchestra/Keyboard/Accordion", 21 },
    // Synth
    { u"Orchestra/Synth/Brass", 62 },
    { u"Orchestra/Synth/Keyboard", 98 },
    { u"Orchestra/Synth/Lead", 87 },
    { u"Orchestra/Synth/Bass", 38 },
    { u"Orchestra/Synth/Pad", 90 },
    { u"Orchestra/Synth/Sequencer", 99 },
    // Strings
    { u"Orchestra/Strings/Violin", 40 },
    { u"Orchestra/Strings/Viola", 41 },
    { u"Orchestra/Strings/Cello", 42 },
    { u"Orchestra/Strings/Contrabass", 43 },
    { u"Orchestra/Strings/Harp", 46 },
    // Winds
    { u"Orchestra/Winds/Harmonica", 22 },
    { u"Orchestra/Winds/Trumpet", 56 },
    { u"Orchestra/Winds/Trombone", 57 },
    { u"Orchestra/Winds/Tuba", 58 },
    { u"Orchestra/Winds/Saxophone", 65 },
    { u"Orchestra/Winds/Clarinet", 71 },
    { u"Orchestra/Winds/Bassoon", 70 },
    { u"Orchestra/Winds/Flute", 73 },
    { u"Orchestra/Winds/Other Winds", 74 },
    // Other Instruments
    { u"Orchestra/Other/Celesta", 8 },
    { u"Orchestra/Other/Vibraphone", 11 },
    { u"Orchestra/Other/Xylophone", 13 },
    { u"Orchestra/Other/Singer", 52 },
    { u"Orchestra/Other/Timpani", 47 },
};
}

namespace mu::iex::guitarpro {
std::pair<int, std::unique_ptr<GPTrack> > GP7DomBuilder::createGPTrack(XmlDomNode* trackNode, XmlDomNode* versionNode)
{
    static const std::set<String> sUnused = {
        u"Color", u"SystemsDefautLayout", u"SystemsLayout", u"AutoBrush",
        u"PalmMute", u"AutoAccentuation", u"PlayingStyle",
        u"UseOneChannelPerString", u"IconId", u"InstrumentSet",
        u"ForcedSound", u"PlaybackState", u"AudioEngineState",
        u"Automations"
    };

    int trackIdx = trackNode->attribute("id").toInt();
    auto track = std::make_unique<GPTrack>(trackIdx);
    XmlDomNode trackChildNode = trackNode->firstChild();
    String version = versionNode->toElement().text();
    bool isRSE = u"RSE" == trackNode->firstChildElement("AudioEngineState").text();

    while (!trackChildNode.isNull()) {
        String nodeName = trackChildNode.nodeName();
        if (nodeName == u"Name") {
            track->setName(trackChildNode.toElement().text());
        } else if (nodeName == u"RSE") {
            GPTrack::RSE rse = readTrackRSE(&trackChildNode);
            track->setRSE(rse);
        } else if (nodeName == u"MidiConnection") {
            int midiChannel = readMidiChannel(&trackChildNode);
            track->setMidiChannel(midiChannel);
        } else if (nodeName == u"ShortName") {
            track->setShortName(trackChildNode.toElement().text());
        } else if (nodeName == u"Sounds") {
            String firstSoundPath = trackChildNode.firstChild().firstChildElement("Path").text();
            int programm = readMidiProgramm(&trackChildNode, isRSE, firstSoundPath);
            auto soundNode = trackChildNode.firstChild();
            while (!soundNode.isNull()) {
                GPTrack::Sound sound = readSounds(&soundNode, isRSE);
                track->addSound(sound);

                soundNode = soundNode.nextSibling();
            }
            track->setProgramm(programm);
        } else if (nodeName == u"Staves") {
            auto staffNode = trackChildNode.firstChild();
            int staffCount = 0;
            while (!staffNode.isNull()) {
                auto propertyNode = staffNode.firstChild();
                // there is a bug in gp v 7.0.0
                // All parts marked to use flat for tuning string,
                // but in real world gp uses tuning presets
                // sp we have to ignore <Flats/> and <TuningFlat> props
                readTrackProperties(&propertyNode, track.get(), version == "7");
                staffNode = staffNode.nextSibling();
                staffCount++;
            }
            track->setStaffCount(staffCount);
        } else if (nodeName == u"NotationPatch") {
            auto innerNode = trackChildNode.firstChildElement("LineCount");
            if (!innerNode.isNull()) {
                track->setLineCount(innerNode.toElement().text().toInt());
            }
        } else if (nodeName == u"Transpose") {
            auto octaveNode = trackChildNode.firstChildElement("Octave");
            int octave = octaveNode.toElement().text().toInt();
            auto chromaticNode = trackChildNode.firstChildElement("Chromatic");
            int chromatic = chromaticNode.toElement().text().toInt();
            int transpose = 12 * octave + chromatic;
            track->setTranspose(transpose);
        } else if (nodeName == u"Lyrics") {
            readLyrics(trackChildNode, track.get());
        } else if (nodeName == u"Automations") {
            auto automationNode = trackChildNode.firstChild();
            while (!automationNode.isNull()) {
                GPTrack::SoundAutomation automation =  readTrackAutomation(&automationNode);
                if (!automation.type.isEmpty()) {
                    track->addSoundAutomation(automation);
                }
                automationNode = automationNode.nextSibling();
            }
        } else if (sUnused.find(nodeName) != sUnused.end()) {
            // Ignored nodes
        }

        trackChildNode = trackChildNode.nextSibling();
    }

    return std::make_pair(trackIdx, std::move(track));
}

int GP7DomBuilder::readMidiChannel(XmlDomNode* trackChildNode) const
{
    int channel
        =trackChildNode->firstChildElement("PrimaryChannel")
          .text().toInt();

    return channel;
}

int GP7DomBuilder::readMidiProgramm(XmlDomNode* trackChildNode, bool isRSE, const String& soundPath) const
{
    int programm = trackChildNode->firstChild()
                   .firstChildElement("MIDI")
                   .firstChildElement("Program")
                   .text().toInt();
    if (isRSE) {
        if (auto it = RSE2MidiProgram.find(soundPath); it != RSE2MidiProgram.end()) {
            programm = it->second;
        }
    }

    return programm;
}

GPTrack::Sound GP7DomBuilder::readSounds(XmlDomNode* soundNode, bool isRSE) const
{
    GPTrack::Sound result;
    result.path = soundNode->firstChildElement("Path").text();
    if (isRSE) {
        if (auto it = RSE2MidiProgram.find(result.path); it != RSE2MidiProgram.end()) {
            result.programm = it->second;
        } else {
            result.programm = soundNode->firstChildElement("MIDI")
                              .firstChildElement("Program")
                              .text().toInt();
        }
    } else {
        result.programm = soundNode->firstChildElement("MIDI")
                          .firstChildElement("Program")
                          .text().toInt();
    }
    result.name = soundNode->firstChildElement("Name").text();
    result.label = soundNode->firstChildElement("Label").text();
    result.role = soundNode->firstChildElement("Role").text();

    return result;
}

GPTrack::SoundAutomation GP7DomBuilder::readTrackAutomation(XmlDomNode* automationNode) const
{
    GPTrack::SoundAutomation result;

    String type = automationNode->firstChildElement("Type").text();
    if (type != u"Sound") {
        return result;
    }

    result.type = type;
    result.linear = automationNode->firstChildElement("Linear").text() == u"true";
    result.bar = automationNode->firstChildElement("Bar").text().toInt();
    result.value = automationNode->firstChildElement("Value").text();
    result.position = automationNode->firstChildElement("Position").text().toFloat();

    return result;
}
} // namespace mu::iex::guitarpro
