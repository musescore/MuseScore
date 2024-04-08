#include "gp6dombuilder.h"

#include <set>

#include "global/log.h"

using namespace muse;

namespace mu::iex::guitarpro {
std::pair<int, std::unique_ptr<GPTrack> > GP6DomBuilder::createGPTrack(XmlDomNode* trackNode, XmlDomNode* versionNode)
{
    UNUSED(versionNode);
    static const std::set<String> sUnusedNodes = {
        u"Color",                                        // we don't use icon color for the tracks
        u"SystemsDefaultLayout", u"SystemsLayout",       // we have our own layout algorithms
        u"SystemsDefautLayout",                          // GP has a typo here :)
        u"PalmMute",                                     // currently our synthesizer is unable to simulate this feature
        u"AutoAccentuation",                             // currently our synthesizer is unable to simulate this feature
        u"PlayingStyle",                                 // currently we ignore playing style
        u"UseOneChannelPerString",                       // we have our own channel management system in synth
        u"PartSounding",                                 // don't know what this is
        u"PlaybackState"                                 // ignored
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

            XmlDomNode bankChangesNode = trackChildNode.firstChildElement("BankChanges");
            if (!bankChangesNode.isNull()) {
                XmlDomNode bankChange = bankChangesNode.firstChild();
                while (!bankChange.isNull()) {
                    GPTrack::SoundAutomation automation =  readRsePickUp(bankChange);
                    track->addSoundAutomation(automation);
                    bankChange = bankChange.nextSibling();
                }
            }
        } else if (nodeName == u"GeneralMidi") {
            if (trackChildNode.hasChildNodes()) {
                auto programm = trackChildNode.firstChildElement("Program").text().toInt();
                track->setProgramm(programm);
                int midiChannel = trackChildNode.firstChildElement("PrimaryChannel").text().toInt();
                track->setMidiChannel(midiChannel);
            }
        } else if (nodeName == u"ShortName") {
            track->setShortName(trackChildNode.toElement().text());
        } else if (nodeName == u"Properties") {
            readTrackProperties(&trackChildNode, track.get(), false);
        } else if (nodeName == u"Instrument") {
            setUpInstrument(&trackChildNode, track.get());
        } else if (nodeName == u"Lyrics") {
            readLyrics(trackChildNode, track.get());
        } else if (sUnusedNodes.find(nodeName) != sUnusedNodes.end()) {
            // these nodes are not used (see comment to the sUnusedNodes variable)
        }

        trackChildNode = trackChildNode.nextSibling();
    }

    return std::make_pair(trackIdx, std::move(track));
}

void GP6DomBuilder::setUpInstrument(XmlDomNode* trackChildNode, GPTrack* track)
{
    String ref = trackChildNode->attribute("ref");
    track->setInstrument(ref);
    if (ref.endsWith(u"-gs") || ref.startsWith(u'2')) { // grand staff
        track->setStaffCount(2);
    }

    if (ref.contains(u"gtr") || ref.contains(u"bass")) {
        //! NOTE Guitar notation is transposed to octave by default (see music grammar)
        track->setTranspose(-12);

        track->setIsGuitar(true);
    }
}

GPTrack::SoundAutomation GP6DomBuilder::readRsePickUp(XmlDomNode& bankChangesNode) const
{
    GPTrack::SoundAutomation result;

    result.bar = bankChangesNode.attribute("barIndex").toInt();
    result.value = bankChangesNode.attribute("bankId");
    result.position = bankChangesNode.attribute("tickOffset").toFloat();

    return result;
}
} // namespace mu::iex::guitarpro
