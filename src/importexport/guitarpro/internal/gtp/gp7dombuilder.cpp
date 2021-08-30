#include "gp7dombuilder.h"

#include <vector>

#include "global/log.h"

namespace Ms {
std::pair<int, std::unique_ptr<GPTrack> > GP7DomBuilder::createGPTrack(QDomNode* trackNode)
{
    static const std::set<QString> sUnused = {
        "Color", "SystemsDefautLayout", "SystemsLayout", "AutoBrush",
        "PalmMute", "AutoAccentuation", "PlayingStyle",
        "UseOneChannelPerString", "IconId", "InstrumentSet",
        "ForcedSound", "PlaybackState", "AudioEngineState",
        "Automations"
    };

    int trackIdx = trackNode->attributes().namedItem("id").toAttr().value().toInt();
    auto track = std::make_unique<GPTrack>(trackIdx);
    QDomNode trackChildNode = trackNode->firstChild();

    while (!trackChildNode.isNull()) {
        QString nodeName = trackChildNode.nodeName();
        if (nodeName == "Name") {
            track->setName(trackChildNode.toElement().text());
        } else if (nodeName == "RSE") {
            GPTrack::RSE rse = readTrackRSE(&trackChildNode);
            track->setRSE(rse);
        } else if (nodeName == "MidiConnection") {
            int midiChannel = readMidiChannel(&trackChildNode);
            track->setMidiChannel(midiChannel);
        } else if (nodeName == "ShortName") {
            track->setShortName(trackChildNode.toElement().text());
        } else if (nodeName == "Sounds") {
            int programm = readMidiProgramm(&trackChildNode);
            track->setProgramm(programm);
        } else if (nodeName == "Staves") {
            auto staffNode = trackChildNode.firstChild();
            int staffCount = 0;
            while (!staffNode.isNull()) {
                auto propertyNode = staffNode.firstChild();
                readTrackProperties(&propertyNode, track.get());
                staffNode = staffNode.nextSibling();
                staffCount++;
            }
            track->setStaffCount(staffCount);
        } else if (nodeName == "Transpose") {
            auto octaveNode = trackChildNode.firstChildElement("Octave");
            int octave = octaveNode.toElement().text().toInt();
            auto chromaticNode = trackChildNode.firstChildElement("Chromatic");
            int chromatic = chromaticNode.toElement().text().toInt();
            int transponce = 12 * octave + chromatic;
            track->setTransponce(transponce);
        } else if ("Lyrics" == nodeName) {
            readLyrics(trackChildNode, track.get());
        } else if (sUnused.find(nodeName) != sUnused.end()) {
            // Ignored nodes
        } else {
            LOGD() << "unknown GP track tag: " << nodeName << "\n";
        }

        trackChildNode = trackChildNode.nextSibling();
    }

    return std::make_pair(trackIdx, std::move(track));
}

int GP7DomBuilder::readMidiChannel(QDomNode* trackChildNode) const
{
    int channel
        =trackChildNode->firstChildElement("PrimaryChannel")
          .text().toInt();

    return channel;
}

int GP7DomBuilder::readMidiProgramm(QDomNode* trackChildNode) const
{
    int programm = trackChildNode->firstChild()
                   .firstChildElement("MIDI")
                   .firstChildElement("Program")
                   .text().toInt();

    return programm;
}
} //end Ms namespace
