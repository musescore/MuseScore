#include "gp6dombuilder.h"

#include <set>

#include "global/log.h"

namespace Ms {
std::pair<int, std::unique_ptr<GPTrack> > GP6DomBuilder::createGPTrack(QDomNode* trackNode)
{
    static const std::set<QString> sUnusedNodes = {
        "Color",                                        // we dont use icon color for the tracks
        "SystemsDefaultLayout", "SystemsLayout",        // we have our own layout algorithms
        "SystemsDefautLayout",                          // GP has a typo here :)
        "PalmMute",                                     // currently our synthesizer is unable to simulate this feature
        "AutoAccentuation",                             // currently our synthesizer is unable to simulate this feature
        "PlayingStyle",                                 // currently we ignore playing style
        "UseOneChannelPerString",                       // we have our own channel management system in synth
        "PartSounding",                                 // don't know what this is
        "PlaybackState"                                 // ignored
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
        } else if (nodeName == "GeneralMidi") {
            if (trackChildNode.toElement().hasChildNodes()) {
                auto programm = trackChildNode.firstChildElement("Program").text().toInt();
                track->setProgramm(programm);
                int midiChannel = trackChildNode.firstChildElement("PrimaryChannel").text().toInt();
                track->setMidiChannel(midiChannel);
            }
        } else if (nodeName == "ShortName") {
            track->setShortName(trackChildNode.toElement().text());
        } else if (nodeName == "Properties") {
            readTrackProperties(&trackChildNode, track.get());
        } else if (nodeName == "Instrument") {
            setUpInstrument(&trackChildNode, track.get());
        } else if ("Lyrics" == nodeName) {
            readLyrics(trackChildNode, track.get());
        } else if (sUnusedNodes.find(nodeName) != sUnusedNodes.end()) {
            // these nodes are not used (see comment to the sUnusedNodes variable)
        } else {
            LOGW() << "unknown GP track tag" << nodeName << "\n";
        }

        trackChildNode = trackChildNode.nextSibling();
    }

    return std::make_pair(trackIdx, std::move(track));
}

void GP6DomBuilder::setUpInstrument(QDomNode* trackChildNode, GPTrack* track)
{
    auto ref = trackChildNode->attributes().namedItem("ref").toAttr().value();
    track->setInstrument(ref);
    if (ref.endsWith("-gs") || ref.startsWith("2")) { // grand staff
        track->setStaffCount(2);
    }

    if (ref.contains("gtr") || ref.contains("bass")) {
        //! NOTE Guitar notation is transponced to octave by default (see music grammar)
        track->setTransponce(-12);

        track->setIsGuitar(true);
    }
}
} //ebd Ms namespace
