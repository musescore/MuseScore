#include "gp7dombuilder.h"

#include <vector>

#include "global/log.h"

using namespace muse;

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

    int trackIdx = trackNode->toElement().attribute("id").value().toInt();
    auto track = std::make_unique<GPTrack>(trackIdx);
    XmlDomNode trackChildNode = trackNode->firstChild();
    String version = versionNode->toElement().text();

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
            int programm = readMidiProgramm(&trackChildNode);
            auto soundNode = trackChildNode.firstChild();
            while (!soundNode.isNull()) {
                GPTrack::Sound sound = readSounds(&soundNode);
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

int GP7DomBuilder::readMidiProgramm(XmlDomNode* trackChildNode) const
{
    int programm = trackChildNode->firstChild()
                   .firstChildElement("MIDI")
                   .firstChildElement("Program")
                   .text().toInt();

    return programm;
}

GPTrack::Sound GP7DomBuilder::readSounds(XmlDomNode* soundNode) const
{
    GPTrack::Sound result;

    result.programm = soundNode->firstChildElement("MIDI")
                      .firstChildElement("Program")
                      .text().toInt();
    result.name = soundNode->firstChildElement("Name").text();
    result.label = soundNode->firstChildElement("Label").text();
    result.path = soundNode->firstChildElement("Path").text();
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
