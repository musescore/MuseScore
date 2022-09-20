#include "gp7dombuilder.h"

#include <vector>

#include "global/log.h"

namespace mu::engraving {
std::pair<int, std::unique_ptr<GPTrack> > GP7DomBuilder::createGPTrack(XmlDomNode* trackNode)
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
            track->setProgramm(programm);
        } else if (nodeName == u"Staves") {
            auto staffNode = trackChildNode.firstChild();
            int staffCount = 0;
            while (!staffNode.isNull()) {
                auto propertyNode = staffNode.firstChild();
                readTrackProperties(&propertyNode, track.get());
                staffNode = staffNode.nextSibling();
                staffCount++;
            }
            track->setStaffCount(staffCount);
        } else if (nodeName == u"Transpose") {
            auto octaveNode = trackChildNode.firstChildElement("Octave");
            int octave = octaveNode.toElement().text().toInt();
            auto chromaticNode = trackChildNode.firstChildElement("Chromatic");
            int chromatic = chromaticNode.toElement().text().toInt();
            int transpose = 12 * octave + chromatic;
            track->setTranspose(transpose);
        } else if (nodeName == u"Lyrics") {
            readLyrics(trackChildNode, track.get());
        } else if (nodeName == u"NotationPatch") {
            auto innerNode = trackChildNode.firstChildElement("LineCount");
            if (!innerNode.isNull()) {
                track->setLineCount(innerNode.toElement().text().toInt());
            }
        } else if (sUnused.find(nodeName) != sUnused.end()) {
            // Ignored nodes
        } else {
            //LOGD() << "unknown GP track tag: " << nodeName << "\n";
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
} //end Ms namespace
