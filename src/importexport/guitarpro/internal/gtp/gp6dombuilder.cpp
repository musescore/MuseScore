#include "gp6dombuilder.h"

#include <set>

#include "global/log.h"

namespace mu::engraving {
std::pair<int, std::unique_ptr<GPTrack> > GP6DomBuilder::createGPTrack(QDomNode* trackNode)
{
    static const std::set<mu::String> sUnusedNodes = {
        u"Color",                                        // we dont use icon color for the tracks
        u"SystemsDefaultLayout", u"SystemsLayout",        // we have our own layout algorithms
        u"SystemsDefautLayout",                          // GP has a typo here :)
        u"PalmMute",                                     // currently our synthesizer is unable to simulate this feature
        u"AutoAccentuation",                             // currently our synthesizer is unable to simulate this feature
        u"PlayingStyle",                                 // currently we ignore playing style
        u"UseOneChannelPerString",                       // we have our own channel management system in synth
        u"PartSounding",                                 // don't know what this is
        u"PlaybackState"                                 // ignored
    };

    int trackIdx = trackNode->attributes().namedItem("id").toAttr().value().toInt();
    auto track = std::make_unique<GPTrack>(trackIdx);
    QDomNode trackChildNode = trackNode->firstChild();

    while (!trackChildNode.isNull()) {
        mu::String nodeName = trackChildNode.nodeName();
        if (nodeName == u"Name") {
            track->setName(trackChildNode.toElement().text());
        } else if (nodeName == u"RSE") {
            GPTrack::RSE rse = readTrackRSE(&trackChildNode);
            track->setRSE(rse);
        } else if (nodeName == u"GeneralMidi") {
            if (trackChildNode.toElement().hasChildNodes()) {
                auto programm = trackChildNode.firstChildElement("Program").text().toInt();
                track->setProgramm(programm);
                int midiChannel = trackChildNode.firstChildElement("PrimaryChannel").text().toInt();
                track->setMidiChannel(midiChannel);
            }
        } else if (nodeName == u"ShortName") {
            track->setShortName(trackChildNode.toElement().text());
        } else if (nodeName == u"Properties") {
            readTrackProperties(&trackChildNode, track.get());
        } else if (nodeName == u"Instrument") {
            setUpInstrument(&trackChildNode, track.get());
        } else if (nodeName == u"Lyrics") {
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
    String ref = trackChildNode->attributes().namedItem("ref").toAttr().value();
    track->setInstrument(ref);
    if (ref.endsWith(u"-gs") || ref.startsWith(u"2")) { // grand staff
        track->setStaffCount(2);
    }

    if (ref.contains(u"gtr") || ref.contains(u"bass")) {
        //! NOTE Guitar notation is transposed to octave by default (see music grammar)
        track->setTranspose(-12);

        track->setIsGuitar(true);
    }
}
} //ebd Ms namespace
