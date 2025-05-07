#include "gp67dombuilder.h"

#include <set>

#include "global/log.h"

#include "engraving/types/constants.h"

using namespace muse;

namespace mu::iex::guitarpro {
GP67DomBuilder::GP67DomBuilder()
{
    _gpDom = std::make_unique<GPDomModel>();
}

void GP67DomBuilder::buildGPDomModel(XmlDomElement* domElem)
{
    XmlDomNode revision;
    // Score node
    XmlDomNode scoreNode,        masterTrack,      audioTracks, eachTrack,
               masterBars,       bars,             voices,
               beats,            notes,            rhythms;

    XmlDomNode gpversion, encoding;

    std::map<String, XmlDomNode*> nodeMap =
    {
        { u"GPRevision",    &revision },
        { u"Score",         &scoreNode },
        { u"MasterTrack",   &masterTrack },
        { u"AudioTracks",   &audioTracks },
        { u"Tracks",        &eachTrack },
        { u"MasterBars",    &masterBars },
        { u"Bars",          &bars },
        { u"Voices",        &voices },
        { u"Beats",         &beats },
        { u"Notes",         &notes },
        { u"Rhythms",       &rhythms },
        { u"GPVersion",     &gpversion },
        { u"Encoding",      &encoding }
    };

    auto assignMap = [&nodeMap](XmlDomNode node)
    {
        auto iter = nodeMap.find(node.nodeName());
        if (iter != nodeMap.end()) {
            *(iter->second) = node;
        }
    };

    for (XmlDomNode current = domElem->firstChild(); !current.isNull(); current = current.nextSibling()) {
        assignMap(current);
    }

    buildGPRhythms(&rhythms);
    buildGPNotes(&notes);
    buildGPBeats(&beats);
    buildGPVoices(&voices);
    buildGPBars(&bars);
    buildGPMasterBars(&masterBars);

    buildGPScore(&scoreNode);
    buildGPMasterTracks(&masterTrack);
    buildGPAudioTracks(&audioTracks);
    buildGPTracks(&eachTrack, &gpversion);
}

std::unique_ptr<GPDomModel> GP67DomBuilder::getGPDomModel()
{
    return std::move(_gpDom);
}

void GP67DomBuilder::buildGPScore(XmlDomNode* scoreNode)
{
    // Contains list of unused info
    static const std::set<String> sUnusedNodes = {
        u"FirstPageFooter", u"FirstPageHeader",
        u"PageFooter", u"PageHeader",
        u"ScoreSystemsDefaultLayout", u"ScoreSystemsLayout", u"PageSetup"
    };

    std::unique_ptr<GPScore> score = std::make_unique<GPScore>();
    XmlDomNode currentNode = scoreNode->firstChild();
    while (!currentNode.isNull()) {
        String nodeName = currentNode.nodeName();
        if (nodeName == u"Title") {
            score->setTitle(currentNode.toElement().text());
        } else if (nodeName == u"Subtitle" || nodeName == u"SubTitle") {
            score->setSubTitle(currentNode.toElement().text());
        } else if (nodeName == u"Artist") {
            score->setArtist(currentNode.toElement().text());
        } else if (nodeName == u"Album") {
            score->setAlbum(currentNode.toElement().text());
        } else if (nodeName == u"Words") {
            score->setPoet(currentNode.toElement().text());
        } else if (nodeName == u"Music") {
            score->setComposer(currentNode.toElement().text());
        } else if (nodeName == u"Copyright") {
            // Currently we ignore Copyright info
        } else if (nodeName == u"Tabber") {
            // Currently we ignore Tabber info
        } else if (nodeName == u"Instructions" || nodeName == u"Notices") {
            // Currently we ignore score unrelated texts
        } else if (nodeName == u"MultiVoice") {
            /// gp saves the value "1>" instead of "1"
            String multiVoiceString = currentNode.toElement().text();
            if (!multiVoiceString.empty()) {
                score->setMultiVoice(String(multiVoiceString[0]).toInt());
            }
        } else if (sUnusedNodes.find(nodeName) != sUnusedNodes.end()) {
            // Ignored nodes, which specify unused specifics (e.g. default layout, footers e.t.c.)
        }

        currentNode = currentNode.nextSibling();
    }
    _gpDom->addGPScore(std::move(score));
}

void GP67DomBuilder::buildGPMasterTracks(XmlDomNode* masterTrack)
{
    std::unique_ptr<GPMasterTracks> masterTracks = std::make_unique<GPMasterTracks>();

    XmlDomNode currentNode = masterTrack->firstChild();
    while (!currentNode.isNull()) {
        String nodeName = currentNode.nodeName();
        if (nodeName == u"Automations") {
            masterTracks->setTempoMap(readTempoMap(&currentNode));
        } else if (nodeName == u"RSE") {
            //! TODO volume and pan(balance) of mixer are set here
        } else if (nodeName == u"Tracks") {
            String tracks = currentNode.toElement().text();
            size_t tracksCount = tracks.split(u' ').size();
            masterTracks->setTracksCount(tracksCount);
        }

        currentNode = currentNode.nextSibling();
    }

    _gpDom->addGPMasterTracks(std::move(masterTracks));
}

void GP67DomBuilder::buildGPAudioTracks(XmlDomNode* audioTrack)
{
    XmlDomNode currentNode = audioTrack->firstChild();
    while (!currentNode.isNull()) {
        currentNode = currentNode.nextSibling();
    }
}

void GP67DomBuilder::buildGPTracks(XmlDomNode* tracksNode, XmlDomNode* versionNode)
{
    std::map<int, std::unique_ptr<GPTrack> > tracks;
    XmlDomNode currentNode = tracksNode->firstChild();
    while (!currentNode.isNull()) {
        tracks.insert(createGPTrack(&currentNode, versionNode));
        currentNode = currentNode.nextSibling();
    }

    _gpDom->addGPTracks(std::move(tracks));
}

void GP67DomBuilder::buildGPMasterBars(XmlDomNode* masterBarsNode)
{
    std::vector<std::unique_ptr<GPMasterBar> > masterBars;

    XmlDomNode innerNode = masterBarsNode->firstChild();
    int masterBarIdx = 0;
    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();

        if (nodeName == u"MasterBar") {
            masterBars.push_back(createGPMasterBar(&innerNode));
            masterBars.back()->setId(masterBarIdx);
            masterBarIdx++;
        }

        innerNode = innerNode.nextSibling();
    }

    _gpDom->addGPMasterBars(std::move(masterBars));
}

void GP67DomBuilder::buildGPBars(XmlDomNode* barsNode)
{
    std::unordered_map<int, std::unique_ptr<GPBar> > bars;

    XmlDomNode innerNode = barsNode->firstChild();
    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();
        if (nodeName == u"Bar") {
            bars.insert(createGPBar(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _bars.swap(bars);
}

void GP67DomBuilder::buildGPVoices(XmlDomNode* voicesNode)
{
    std::unordered_map<int, std::unique_ptr<GPVoice> > voices;

    XmlDomNode innerNode = voicesNode->firstChild();
    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();
        if (nodeName == u"Voice") {
            voices.insert(createGPVoice(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _voices.swap(voices);
}

void GP67DomBuilder::buildGPBeats(XmlDomNode* beatsNode)
{
    std::unordered_map<int, std::shared_ptr<GPBeat> > beats;

    XmlDomNode innerNode = beatsNode->firstChild();
    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();
        if (nodeName == u"Beat") {
            beats.insert(createGPBeat(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _beats.swap(beats);
}

void GP67DomBuilder::buildGPNotes(XmlDomNode* notesNode)
{
    std::unordered_map<int, std::shared_ptr<GPNote> > notes;

    XmlDomNode innerNode = notesNode->firstChild();

    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();
        if (nodeName == u"Note") {
            notes.insert(createGPNote(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _notes.swap(notes);
}

void GP67DomBuilder::buildGPRhythms(XmlDomNode* rhythmsNode)
{
    std::unordered_map<int, std::shared_ptr<GPRhythm> > rhythms;

    XmlDomNode innerNode = rhythmsNode->firstChild();
    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();
        if (nodeName == u"Rhythm") {
            rhythms.insert(createGPRhythm(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _rhythms.swap(rhythms);
}

std::vector<GPMasterTracks::Automation> GP67DomBuilder::readTempoMap(XmlDomNode* currentNode)
{
    std::vector<GPMasterTracks::Automation> tempoMap;
    XmlDomNode currentAutomation = currentNode->firstChild();
    while (!currentAutomation.isNull()) {
        if (currentAutomation.nodeName() == u"Automation") {
            XmlDomElement ln = currentAutomation.firstChildElement("Linear");
            String first_name = currentAutomation.firstChild().nodeName();
            if (first_name == u"Type") {
                first_name = currentAutomation.firstChild().toElement().text();
            }
            if (first_name == u"Tempo") {
                GPMasterTracks::Automation tempo;
                tempo.type = GPMasterTracks::Automation::Type::tempo;
                String str = currentAutomation.firstChildElement("Value").toElement().text();
                StringList tempoValue = str.split(u' ');
                tempo.value = static_cast<int>(tempoValue[0].toDouble());
                tempo.tempoUnit = tempoValue.size() > 1 ? static_cast<int>(tempoValue[1].toDouble()) : 0;
                tempo.bar = currentAutomation.firstChildElement("Bar").text().toInt();
                tempo.position = currentAutomation.firstChildElement("Position").text().toFloat();
                tempo.linear = (ln.toElement().text() == u"true");
                XmlDomElement labelElem = currentAutomation.firstChildElement("Text");
                if (labelElem.hasChildNodes()) {
                    tempo.text = labelElem.toElement().text();
                }

                tempoMap.push_back(tempo);
            }
        }
        currentAutomation = currentAutomation.nextSibling();
    }

    return tempoMap;
}

std::unique_ptr<GPMasterTracks> GP67DomBuilder::createGPMasterTrack(XmlDomNode* metadata)
{
    UNUSED(metadata);
    return std::make_unique<GPMasterTracks>();
}

std::unique_ptr<GPMasterBar> GP67DomBuilder::createGPMasterBar(XmlDomNode* masterBarNode)
{
    static const std::set<String> sUnused = {
        u"XProperties"
    };

    auto tripletFeelType = [](String&& str) {
        if (str == u"Triplet8th") {
            return GPMasterBar::TripletFeelType::Triplet8th;
        }
        if (str == u"Triplet16th") {
            return GPMasterBar::TripletFeelType::Triplet16th;
        }
        if (str == u"Dotted8th") {
            return GPMasterBar::TripletFeelType::Dotted8th;
        }
        if (str == u"Dotted16th") {
            return GPMasterBar::TripletFeelType::Dotted16th;
        }
        if (str == u"Scottish8th") {
            return GPMasterBar::TripletFeelType::Scottish8th;
        }
        if (str == u"Scottish16th") {
            return GPMasterBar::TripletFeelType::Scottish16th;
        }
        return GPMasterBar::TripletFeelType::None;
    };

    std::unique_ptr<GPMasterBar> masterBar = std::make_unique<GPMasterBar>();

    auto innerNode = masterBarNode->firstChild();

    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();

        if (nodeName == u"Time") {
            masterBar->setTimeSig(readTimeSig(&innerNode));
        } else if (nodeName == u"Repeat") {
            masterBar->setRepeat(readRepeat(&innerNode));
        } else if (nodeName == u"AlternateEndings") {
            masterBar->setAlternativeEnding(readEnding(&innerNode));
        } else if (nodeName == u"Key") {
            masterBar->setKeySig(readKeySig(&innerNode), readUseFlats(&innerNode));
        } else if (nodeName == u"Bars") {
            const String& barsElement = innerNode.toElement().text();
            const StringList& bars = barsElement.split(u' ');
            for (const String& barIdx : bars) {
                int idx = barIdx.toInt();
                std::unique_ptr<GPBar> bar;
                bar = std::move(_bars.at(idx));
                _bars.erase(idx);
                masterBar->addGPBar(std::move(bar));
            }
        } else if (nodeName == u"TripletFeel") {
            masterBar->setTripletFeel(tripletFeelType(innerNode.toElement().text()));
        } else if (nodeName == u"Fermatas") {
            masterBar->setFermatas(readFermatas(&innerNode));
        } else if (nodeName == u"Section") {
            masterBar->setSection(readMasterBarSection(innerNode));
        } else if (nodeName == u"Directions") {
            masterBar->setDirections(readRepeatsJumps(&innerNode));
        } else if (nodeName == u"DoubleBar") {
            masterBar->setBarlineType(GPMasterBar::BarlineType::DOUBLE);
        } else if (nodeName == u"FreeTime") {
            masterBar->setFreeTime(true);
        } else if (sUnused.find(nodeName) != sUnused.end()) {
            // Ignored
        }

        innerNode = innerNode.nextSibling();
    }

    return masterBar;
}

std::pair<int, std::unique_ptr<GPBar> > GP67DomBuilder::createGPBar(XmlDomNode* barNode)
{
    auto clefType = [](const String& clef) {
        if (clef == u"C4") {
            return GPBar::ClefType::C4;
        } else if (clef == u"C3") {
            return GPBar::ClefType::C3;
        } else if (clef == u"F4") {
            return GPBar::ClefType::F4;
        } else if (clef == u"G2") {
            return GPBar::ClefType::G2;
        } else {
            return GPBar::ClefType::Neutral;
        }
    };

    auto ottaviaType = [](const String& ott) {
        if (ott == u"8va") {
            return GPBar::OttaviaType::va8;
        } else if (ott == u"15ma") {
            return GPBar::OttaviaType::ma15;
        } else if (ott == u"8vb") {
            return GPBar::OttaviaType::vb8;
        } else if (ott == u"15mb") {
            return GPBar::OttaviaType::mb15;
        } else {
            return GPBar::OttaviaType::Regular;
        }
    };
    auto simileMarkType = [](const String& str) {
        if (str == u"Simple") {
            return GPBar::SimileMark::Simple;
        } else if (str == u"FirstOfDouble") {
            return GPBar::SimileMark::FirstOfDouble;
        } else if (str == u"SecondOfDouble") {
            return GPBar::SimileMark::SecondOfDouble;
        } else {
            return GPBar::SimileMark::None;
        }
    };

    std::unique_ptr<GPBar> bar = std::make_unique<GPBar>();

    auto innerNode = barNode->firstChild();
    int barIdx = barNode->toElement().attribute("id").value().toInt();
    bar->setId(barIdx);

    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();

        if (nodeName == u"Clef") {
            bar->setClefType(clefType(innerNode.toElement().text()));
        } else if (nodeName == u"Ottavia") {
            bar->setOttaviaType(ottaviaType(innerNode.toElement().text()));
        } else if (nodeName == u"SimileMark") {
            bar->setSimileMark(simileMarkType(innerNode.toElement().text()));
        } else if (nodeName == u"Voices") {
            String voicesElement = innerNode.toElement().text();
            StringList voices = voicesElement.split(u' ');
            int currentPosition = -1;
            for (const String& voiceIdx : voices) {
                currentPosition++;
                int idx = voiceIdx.toInt();
                if (idx == -1) {
                    continue;
                }

                std::unique_ptr<GPVoice> voice;
                voice = std::move(_voices.at(idx));
                voice->setPosition(currentPosition);
                _voices.erase(idx);
                bar->addGPVoice(std::move(voice));
            }
        }

        innerNode = innerNode.nextSibling();
    }

    return std::make_pair(barIdx, std::move(bar));
}

std::pair<int, std::unique_ptr<GPVoice> > GP67DomBuilder::createGPVoice(XmlDomNode* voiceNode)
{
    std::unique_ptr<GPVoice> voice = std::make_unique<GPVoice>();

    XmlDomNode innerNode = voiceNode->firstChild();
    int voiceIdx = voiceNode->toElement().attribute("id").value().toInt();
    voice->setId(voiceIdx);

    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();
        if (nodeName == u"Beats") {
            String beatsElement = innerNode.toElement().text();
            StringList beats = beatsElement.split(u' ');
            for (const String& beatIdx : beats) {
                int idx = beatIdx.toInt();
                std::shared_ptr<GPBeat> beat;
                beat = _beats.at(idx);
                voice->addGPBeat(beat);
            }
        }

        innerNode = innerNode.nextSibling();
    }

    return std::make_pair(voiceIdx, std::move(voice));
}

std::pair<int, std::shared_ptr<GPBeat> > GP67DomBuilder::createGPBeat(XmlDomNode* beatNode)
{
    static const std::set<String> sUnused = {
        u"Bank",
        u"StemOrientation", u"ConcertPitchStemOrientation",
        u"Ottavia"
    };

    auto dynamicType = [](String&& str) -> GPBeat::DynamicType {
        if (str == u"FFF") {
            return GPBeat::DynamicType::FFF;
        } else if (str == u"FF") {
            return GPBeat::DynamicType::FF;
        } else if (str == u"F") {
            return GPBeat::DynamicType::F;
        } else if (str == u"MF") {
            return GPBeat::DynamicType::MF;
        } else if (str == u"MP") {
            return GPBeat::DynamicType::MP;
        } else if (str == u"P") {
            return GPBeat::DynamicType::P;
        } else if (str == u"PP") {
            return GPBeat::DynamicType::PP;
        } else {
            return GPBeat::DynamicType::PPP;
        }
    };
    auto legatoType = [](const String& origin, const String& destination) {
        if (origin == u"true" && destination == u"false") {
            return GPBeat::LegatoType::Start;
        } else if (origin == u"true" && destination == u"true") {
            return GPBeat::LegatoType::Mediate;
        } else if (origin == u"false" && destination == u"true") {
            return GPBeat::LegatoType::End;
        } else {
            return GPBeat::LegatoType::None;
        }
    };
    auto arpeggioType = [](const String& arp) {
        if (arp == u"Up") {
            return GPBeat::Arpeggio::Up;
        } else if (arp == u"Down") {
            return GPBeat::Arpeggio::Down;
        }
        return GPBeat::Arpeggio::None;
    };
    auto graceNotes = [](const String& gn) {
        if (gn == u"OnBeat") {
            return GPBeat::GraceNotes::OnBeat;
        } else if (gn == u"BeforeBeat") {
            return GPBeat::GraceNotes::BeforeBeat;
        }
        return GPBeat::GraceNotes::None;
    };
    auto faddingType = [](const String& str) {
        if (str == u"FadeIn") {
            return GPBeat::Fadding::FadeIn;
        } else if (str == u"FadeOut") {
            return GPBeat::Fadding::FadeOut;
        } else if (str == u"VolumeSwell") {
            return GPBeat::Fadding::VolumeSwell;
        }
        return GPBeat::Fadding::None;
    };
    auto hairpinType = [](const String& str) {
        if (str == u"Crescendo") {
            return GPBeat::Hairpin::Crescendo;
        } else if (str == u"Decrescendo") {
            return GPBeat::Hairpin::Decrescendo;
        }
        return GPBeat::Hairpin::None;
    };
    auto wahType = [](const String& str) {
        if (str == u"Open") {
            return GPBeat::Wah::Open;
        } else if (str == u"Closed") {
            return GPBeat::Wah::Closed;
        } else {
            return GPBeat::Wah::None;
        }
    };

    auto golpeType = [](const String& str) {
        if (str == u"Finger") {
            return GPBeat::Golpe::Finger;
        } else if (str == u"Thumb") {
            return GPBeat::Golpe::Thumb;
        }

        return GPBeat::Golpe::None;
    };

    auto ottavaType = [](const String& ott) {
        if (ott == u"8va") {
            return GPBeat::OttavaType::va8;
        } else if (ott == u"15ma") {
            return GPBeat::OttavaType::ma15;
        } else if (ott == u"8vb") {
            return GPBeat::OttavaType::vb8;
        } else if (ott == u"15mb") {
            return GPBeat::OttavaType::mb15;
        }
        LOGE() << "wrong ottava type: " << ott;
        return GPBeat::OttavaType::None;
    };

    std::shared_ptr<GPBeat> beat = std::make_shared<GPBeat>();

    auto innerNode = beatNode->firstChild();
    int beatIdx = beatNode->toElement().attribute("id").value().toInt();
    beat->setId(beatIdx);

    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();

        if (nodeName == u"Dynamic") {
            GPBeat::DynamicType dynamic = dynamicType(innerNode.toElement().text());
            beat->setDynamic(dynamic);
        } else if (nodeName == u"Legato") {
            String origin = innerNode.toElement().attribute("origin").value();
            String destination = innerNode.toElement().attribute("destination").value();
            GPBeat::LegatoType legato = legatoType(origin, destination);
            beat->setLegatoType(legato);
        } else if (nodeName == u"Rhythm") {
            int rIdx = innerNode.toElement().attribute("ref").value().toInt();
            beat->addGPRhythm(_rhythms.at(rIdx));
        } else if (nodeName == u"Notes") {
            String notesStr = innerNode.toElement().text();
            StringList strList = notesStr.split(u' ');
            for (const auto& strIdx : strList) {
                int idx = strIdx.toInt();
                std::shared_ptr<GPNote> note;
                note = _notes.at(idx);
                beat->addGPNote(note);
            }
            beat->sortGPNotes();
        } else if (nodeName == u"GraceNotes") {
            beat->setGraceNotes(graceNotes(innerNode.toElement().text()));
        } else if (nodeName == u"Arpeggio") {
            beat->setArpeggio(arpeggioType(innerNode.toElement().text()));
        } else if (nodeName == u"Properties") {
            readBeatProperties(innerNode, beat.get());
        } else if (nodeName == u"Chord") {
            beat->setDiagramIdx(innerNode.toElement().text().toInt());
        } else if (nodeName == u"Timer") {
            beat->setTime(innerNode.toElement().text().toInt());
        } else if (nodeName == u"FreeText") {
            beat->setFreeText(innerNode.toElement().text());
        } else if (nodeName == u"Fadding") {
            beat->setFadding(faddingType(innerNode.toElement().text()));
        } else if (nodeName == u"Hairpin") {
            beat->setHairpin(hairpinType(innerNode.toElement().text()));
        } else if (nodeName == u"Tremolo") {
            GPBeat::Tremolo tr;
            String trStr = innerNode.toElement().text();
            StringList trList = trStr.split(u'/');
            tr.numerator = trList.at(0).toInt();
            tr.denominator = trList.at(1).toInt();
            beat->setTremolo(tr);
        } else if (nodeName == u"Wah") {
            beat->setWah(wahType(innerNode.toElement().text()));
        } else if (nodeName == u"Golpe") {
            beat->setGolpe(golpeType(innerNode.toElement().text()));
        } else if (nodeName == u"Lyrics") {
            // this code reads lyrics for the beat (only one line).

            XmlDomElement lyrNode = innerNode.firstChildElement("Line");
            String str = lyrNode.toElement().text();
            beat->setLyrics(str.toStdString());
        } else if (nodeName == u"Ottavia") {
            beat->setOttavaType(ottavaType(innerNode.toElement().text()));
        } else if (nodeName == u"Whammy" || nodeName == u"WhammyExtend") {
            // TODO-gp: implement dives
            beat->setDive(true);
        } else if (nodeName == u"DeadSlapped") {
            beat->setDeadSlapped(true);
        } else if (nodeName == u"TransposedPitchStemOrientation") {
            beat->setStemOrientationUp(innerNode.toElement().text() == u"Upward");
        } else if (nodeName == u"TransposedPitchStemOrientationUserDefined") {
            beat->setStemOrientationUserDefined(true);
        } else if (nodeName == u"XProperties") {
            readBeatXProperties(innerNode, beat.get());
        } else if (sUnused.find(nodeName) != sUnused.end()) {
            // Ignored nodes
        }

        innerNode = innerNode.nextSibling();
    }

    return std::make_pair(beatIdx, std::move(beat));
}

std::pair<int, std::shared_ptr<GPNote> > GP67DomBuilder::createGPNote(XmlDomNode* noteNode)
{
    auto tieType = [](const String& origin, const String& destination) ->GPNote::TieType {
        if (origin == u"true" && destination == u"false") {
            return GPNote::TieType::Start;
        } else if (origin == u"true" && destination == u"true") {
            return GPNote::TieType::Mediate;
        } else if (origin == u"false" && destination == u"true") {
            return GPNote::TieType::End;
        } else {
            return GPNote::TieType::None;
        }
    };
    auto vibratoType = [](const String& str) {
        if (str == u"Wide") {
            return GPNote::VibratoType::Wide;
        } else if (str == u"Slight") {
            return GPNote::VibratoType::Slight;
        } else {
            return GPNote::VibratoType::None;
        }
    };
    auto ornamentType = [](const String& str) {
        if (str == u"UpperMordent") {
            return GPNote::Ornament::UpperMordent;
        } else if (str == u"LowerMordent") {
            return GPNote::Ornament::LowerMordent;
        } else if (str == u"InvertedTurn") {
            return GPNote::Ornament::InvertedTurn;
        } else if (str == u"Turn") {
            return GPNote::Ornament::Turn;
        } else {
            return GPNote::Ornament::None;
        }
    };

    auto note = std::make_shared<GPNote>();
    int noteIdx = noteNode->toElement().attribute("id").value().toInt();
    note->setId(noteIdx);

    auto innerNode = noteNode->firstChild();
    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();

        if (nodeName == u"Accidental") {
            std::map<String, int> accidentals = {
                { u"DoubleFlat", -2 },
                { u"Flat", -1 },
                { u"Natural", 0 },
                { u"Sharp", +1 },
                { u"DoubleSharp", +2 }
            };

            String accidentalName = innerNode.toElement().text();

            if (accidentals.find(accidentalName) != accidentals.end()) {
                note->setAccidental(accidentals[accidentalName]);
            }
        }
        if (nodeName == u"InstrumentArticulation") {
        }
        if (nodeName == u"Properties") {
            readNoteProperties(&innerNode, note.get());
        }
        if (nodeName == u"XProperties") {
            readNoteXProperties(innerNode, note.get());
        }
        if (nodeName == u"Tie") {
            String origin = innerNode.toElement().attribute("origin").value();
            String destination = innerNode.toElement().attribute("destination").value();
            GPNote::TieType tie = tieType(origin, destination);
            note->setTieType(tie);
        }
        if (nodeName == u"LetRing") {
            note->setLetRing(true);
        } else if (nodeName == u"AntiAccent") {
            note->setGhostNote(innerNode.toElement().text() == u"Normal");
        } else if (nodeName == u"Accent") {
            note->setAccent(innerNode.toElement().text().toUInt());
        } else if (nodeName == u"LeftFingering") {
            String finger = innerNode.toElement().text();
            if (finger == u"Open") {
                finger = u"0";
            } else if (finger == u"P") {
                finger = u"t";
            } else if (finger == u"I") {
                finger = u"1";
            } else if (finger == u"M") {
                finger = u"2";
            } else if (finger == u"A") {
                finger = u"3";
            } else if (finger == u"C") {
                finger = u"4";
            }
            note->setLeftFingering(finger);
        } else if (nodeName == "RightFingering") {
            note->setRightFingering(innerNode.toElement().text().toLower());
        } else if (nodeName == "Vibrato") {
            note->setVibratoType(vibratoType(innerNode.toElement().text()));
        } else if (nodeName == "Trill") {
            note->setTrillFret(innerNode.toElement().text().toInt());
        } else if (nodeName == "Ornament") {
            note->setOrnament(ornamentType(innerNode.toElement().text()));
        }

        innerNode = innerNode.nextSibling();
    }

    return std::make_pair(noteIdx, std::move(note));
}

std::pair<int, std::shared_ptr<GPRhythm> > GP67DomBuilder::createGPRhythm(XmlDomNode* rhythmNode)
{
    auto rhythmType = [](const String& str) -> GPRhythm::RhytmType {
        if (str == u"Whole") {
            return GPRhythm::RhytmType::Whole;
        } else if (str == u"Half") {
            return GPRhythm::RhytmType::Half;
        } else if (str == u"Quarter") {
            return GPRhythm::RhytmType::Quarter;
        } else if (str == u"Eighth") {
            return GPRhythm::RhytmType::Eighth;
        } else if (str == u"16th") {
            return GPRhythm::RhytmType::Sixteenth;
        } else if (str == u"32nd") {
            return GPRhythm::RhytmType::ThirtySecond;
        } else {
            return GPRhythm::RhytmType::SixtyFourth;
        }
    };

    std::shared_ptr<GPRhythm> rhythm = std::make_shared<GPRhythm>();

    auto innerNode = rhythmNode->firstChild();
    int rhythmIdx = rhythmNode->toElement().attribute("id").value().toInt();
    while (!innerNode.isNull()) {
        String nodeName = innerNode.nodeName();
        if (nodeName == u"NoteValue") {
            auto rhType = rhythmType(innerNode.toElement().text());
            rhythm->setRhytm(rhType);
        }
        if (nodeName == u"AugmentationDot") {
            rhythm->setDotCount(innerNode.toElement().attribute("count").value().toInt());
        } else if (nodeName == u"PrimaryTuplet") {
            int num = innerNode.toElement().attribute("num").value().toInt();
            int denom = innerNode.toElement().attribute("den").value().toInt();
            rhythm->setTuplet({ num, denom });
        }

        innerNode = innerNode.nextSibling();
    }

    return std::make_pair(rhythmIdx, std::move(rhythm));
}

GPTrack::RSE GP67DomBuilder::readTrackRSE(XmlDomNode* trackChildNode) const
{
    XmlDomNode innerNode = trackChildNode->firstChild();
    String nodeName = innerNode.nodeName();

    if (!innerNode.isNull() && nodeName == u"ChannelStrip") {
        innerNode = innerNode.firstChildElement("Parameters");
        if (!innerNode.isNull()) {
            String str = innerNode.toElement().text();
            StringList strList = str.split(u' ');
            GPTrack::RSE rse;
            rse.pan = strList.at(11).toFloat();
            rse.volume = strList.at(12).toFloat();
            return rse;
        }
    }

    return GPTrack::RSE();
}

GPMasterBar::KeySig GP67DomBuilder::readKeySig(XmlDomNode* keyNode) const
{
    const auto& accidentalCount = keyNode->firstChildElement("AccidentalCount");
    const auto& modeNode = keyNode->firstChildElement("Mode");

    String modeName = modeNode.toElement().text();

    GPMasterBar::KeySig::Mode mode = GPMasterBar::KeySig::Mode::Major;
    if (modeName == "Minor") {
        mode = GPMasterBar::KeySig::Mode::Minor;
    }

    int keyCount = accidentalCount.toElement().text().toInt();

    return GPMasterBar::KeySig{ GPMasterBar::KeySig::Accidentals(keyCount), mode };
}

bool GP67DomBuilder::readUseFlats(XmlDomNode* keyNode) const
{
    const auto& transposeAs = keyNode->firstChildElement("TransposeAs");
    if (transposeAs.isNull()) {
        return false;
    }
    return transposeAs.toElement().text() == "Flats";
}

GPMasterBar::TimeSig GP67DomBuilder::readTimeSig(XmlDomNode* timeNode) const
{
    const String time = timeNode->toElement().text();
    const StringList timeSig = time.split(u'/');
    GPMasterBar::TimeSig sig { timeSig.at(0).toInt(), timeSig.at(1).toInt() };
    return sig;
}

void GP67DomBuilder::readNoteXProperties(const XmlDomNode& propertiesNode, GPNote* note)
{
    auto propertyNode = propertiesNode.firstChild();

    while (!propertyNode.isNull()) {
        int propertyId = propertyNode.toElement().attribute("id").value().toInt();

        if (propertyId == 688062467) {
            note->setTrillSpeed(propertyNode.firstChild().toElement().text().toInt());
        }

        propertyNode = propertyNode.nextSibling();
    }
}

void GP67DomBuilder::readNoteProperties(XmlDomNode* propertiesNode, GPNote* note)
{
    std::unordered_set<std::unique_ptr<INoteProperty> > properties;

    auto propertyNode = propertiesNode->firstChild();

    while (!propertyNode.isNull()) {
        auto propertyName = propertyNode.toElement().attribute("name").value();

        if (propertyName == u"Midi") {
            int midi = propertyNode.firstChild().toElement().text().toInt();
            note->setMidi(midi);
        }
        if (propertyName == u"Variation") {
            note->setVariation(propertyNode.firstChild().toElement().text().toInt());
        }
        if (propertyName == u"Element") {
            note->setElement(propertyNode.firstChild().toElement().text().toInt());
        } else if (propertyName == u"String") {
            int string = propertyNode.firstChild().toElement().text().toInt();
            note->setString(string);
        } else if (propertyName == u"Fret") {
            int fret = propertyNode.firstChild().toElement().text().toInt();
            note->setFret(fret);
        } else if (propertyName == u"Octave") {
            int octave = propertyNode.firstChild().toElement().text().toInt();
            note->setOctave(octave);
        } else if (propertyName == u"ConcertPitch") {
            auto pitchNode = propertyNode.firstChild();
            auto innerNode = pitchNode.firstChild();
            while (!innerNode.isNull()) {
                String nodeName = innerNode.nodeName();
                if (nodeName == u"Accidental") {
                    std::map<String, int> accidentals = {
                        { u"bb", -2 },
                        { u"b",  -1 },
                        { u"#",  +1 },
                        { u"x",  +2 },
                    };

                    String accidentalName = innerNode.toElement().text();
                    if (!accidentalName.isEmpty() && accidentals.find(accidentalName) != accidentals.end()) {
                        note->setAccidental(accidentals[accidentalName]);
                    } else {
                        note->setAccidental(0);
                    }
                }

                innerNode = innerNode.nextSibling();
            }
        } else if (propertyName == u"Tone") {
            int tone = propertyNode.firstChild().toElement().text().toInt();
            note->setTone(tone);
        } else if (propertyName == u"Bended") {
            if (propertyNode.firstChild().nodeName() == "Enable") {
                note->setBend(createBend(&propertyNode));
            }
        } else if (propertyName == u"Harmonic"
                   || propertyName == u"HarmonicFret"
                   || propertyName == u"HarmonicType") {
            readHarmonic(&propertyNode, note);
        } else if (propertyName == u"PalmMuted") {
            if (propertyNode.firstChild().nodeName() == "Enable") {
                note->setPalmMute(true);
            }
        } else if (propertyName == u"Muted") {
            //! property muted in GP means dead note
            if (propertyNode.firstChild().nodeName() == "Enable") {
                note->setMute(true);
            }
        } else if (propertyName == u"Slide") {
            int slideInfo = propertyNode.firstChild().toElement().text().toUInt();
            switch (slideInfo) {
            case 64:
                note->setPickScrape(GPNote::PickScrape::Down);
                break;
            case 128:
                note->setPickScrape(GPNote::PickScrape::Up);
                break;
            default:
                note->setSlides(slideInfo);
                break;
            }
        } else if (propertyName == u"HopoOrigin") {
            note->setHammerOn(GPNote::HammerOn::Start);
        } else if (propertyName == u"Tapped") {
            if (propertyNode.firstChild().nodeName() == "Enable") {
                note->setTapping(true);
            }
        } else if (propertyName == u"LeftHandTapped") {
            if (propertyNode.firstChild().nodeName() == "Enable") {
                note->setLeftHandTapped(true);
            }
        } else if (propertyName == "ShowStringNumber") {
            note->setShowStringNumber(true);
        }

        propertyNode = propertyNode.nextSibling();
    }

    note->addProperties(std::move(properties));
    return;
}

void GP67DomBuilder::readBeatXProperties(const XmlDomNode& propertiesNode, GPBeat* beat)
{
    auto propertyNode = propertiesNode.firstChild();

    bool brokenBeams = false;
    bool brokenSecondaryBeams = false;
    bool joinedBeams = false;

    while (!propertyNode.isNull()) {
        int propertyId = propertyNode.toElement().attribute("id").value().toInt();

        if (propertyId == 687931393 || propertyId == 687935489) {
            // arpeggio/brush ticks
            beat->setArpeggioStretch(propertyNode.firstChild().toElement().text().toDouble() / mu::engraving::Constants::DIVISION);
        } else if (propertyId == 1124204546) {
            int beamData = propertyNode.firstChild().toElement().text().toInt();

            if (beamData == 1) {
                joinedBeams = true;
            } else if (beamData == 2) {
                brokenBeams = true;
            }
        } else if (propertyId == 1124204552) {
            int beamData = propertyNode.firstChild().toElement().text().toInt();
            if (beamData == 1) {
                brokenSecondaryBeams = true;
            }
        }

        propertyNode = propertyNode.nextSibling();
    }

    if (brokenBeams) {
        beat->setBeamMode(GPBeat::BeamMode::BROKEN);
    } else if (brokenSecondaryBeams) {
        beat->setBeamMode(joinedBeams ? GPBeat::BeamMode::BROKEN2_JOINED : GPBeat::BeamMode::BROKEN2);
    } else if (joinedBeams) {
        beat->setBeamMode(GPBeat::BeamMode::JOINED);
    }
}

std::unique_ptr<GPNote::Bend> GP67DomBuilder::createBend(XmlDomNode* propertyNode)
{
    std::unique_ptr<GPNote::Bend> bend = std::make_unique<GPNote::Bend>();

    auto currentNode = propertyNode->nextSibling();
    while (!currentNode.isNull()) {
        String propertyName = currentNode.toElement().attribute("name").value();
        if (propertyName == u"BendDestinationOffset") {
            bend->destinationOffset = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == u"BendDestinationValue") {
            bend->destinationValue = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == u"BendMiddleOffset1") {
            bend->middleOffset1 = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == u"BendMiddleOffset2") {
            bend->middleOffset2 = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == u"BendMiddleValue") {
            bend->middleValue = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == u"BendOriginOffset") {
            bend->originOffset = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == u"BendOriginValue") {
            bend->originValue = currentNode.firstChild().toElement().text().toFloat();
        }

        currentNode = currentNode.nextSibling();
    }

    return bend;
}

void GP67DomBuilder::readHarmonic(XmlDomNode* propertyNode, GPNote* note) const
{
    auto harmonicType = [](const String& str) {
        if (str == u"Artificial") {
            return GPNote::Harmonic::Type::Artificial;
        } else if (str == u"Semi") {
            return GPNote::Harmonic::Type::Semi;
        } else if (str == u"Pinch") {
            return GPNote::Harmonic::Type::Pinch;
        } else if (str == u"Feedback") {
            return GPNote::Harmonic::Type::FeedBack;
        } else if (str == u"Natural") {
            return GPNote::Harmonic::Type::Natural;
        } else {
            return GPNote::Harmonic::Type::Tap;
        }
    };

    String propertyName = propertyNode->toElement().attribute("name").value();
    if (propertyName == u"HarmonicFret") {
        note->setHarmonicFret(propertyNode->firstChild().toElement().text().toFloat());
    } else if (propertyName == u"HarmonicType") {
        note->setHarmonicType(harmonicType(propertyNode->firstChild().toElement().text()));
    }
}

void GP67DomBuilder::readBeatProperties(const XmlDomNode& propertiesNode, GPBeat* beat) const
{
    auto brushType = [](const String& brush) {
        if (brush == u"Down") {
            return GPBeat::Brush::Down;
        }
        return GPBeat::Brush::Up;
    };
    auto vibratoType = [](const String& str) {
        if (str == u"Wide") {
            return GPBeat::VibratoWTremBar::Wide;
        } else if (str == u"Slight") {
            return GPBeat::VibratoWTremBar::Slight;
        } else {
            return GPBeat::VibratoWTremBar::None;
        }
    };
    auto rasgueadoType = [](const String& str) {
        if (str == u"ii_1") {
            return GPBeat::Rasgueado::II_1;
        } else if (str == u"mi_1") {
            return GPBeat::Rasgueado::MII_1;
        } else if (str == u"mii_1") {
            return GPBeat::Rasgueado::MII_1;
        } else if (str == u"mii_2") {
            return GPBeat::Rasgueado::MII_2;
        } else if (str == u"ami_1") {
            return GPBeat::Rasgueado::AMI_1;
        } else if (str == u"ami_2") {
            return GPBeat::Rasgueado::AMI_2;
        } else if (str == u"pai_1") {
            return GPBeat::Rasgueado::PAI_1;
        } else if (str == u"pai_2") {
            return GPBeat::Rasgueado::PAI_2;
        } else if (str == u"pei_1") {
            return GPBeat::Rasgueado::PEI_1;
        } else if (str == u"pei_2") {
            return GPBeat::Rasgueado::PEI_2;
        } else if (str == u"pmp_1") {
            return GPBeat::Rasgueado::PMP_1;
        } else if (str == u"pmp_2") {
            return GPBeat::Rasgueado::PMP_2;
        } else if (str == u"ppp_1") {
            return GPBeat::Rasgueado::PPP_1;
        } else if (str == u"amii_1") {
            return GPBeat::Rasgueado::AMII_1;
        } else if (str == u"amip_1") {
            return GPBeat::Rasgueado::AMIP_1;
        } else if (str == u"eami_1") {
            return GPBeat::Rasgueado::EAMI_1;
        } else if (str == u"eamii_1") {
            return GPBeat::Rasgueado::EAMII_1;
        } else if (str == u"peami_1") {
            return GPBeat::Rasgueado::PEAMI_1;
        } else {
            return GPBeat::Rasgueado::None;
        }
    };
    auto pickStrokeType = [](const String& str) {
        if (str == u"Up") {
            return GPBeat::PickStroke::Up;
        } else if (str == u"Down") {
            return GPBeat::PickStroke::Down;
        }
        return GPBeat::PickStroke::None;
    };

    auto propertyNode = propertiesNode.firstChild();

    while (!propertyNode.isNull()) {
        String propertyName = propertyNode.toElement().attribute("name").value();

        if (propertyName == u"Popped") {
            if (propertyNode.firstChild().nodeName() == "Enable") {
                beat->setPopped(true);
            }
        } else if (propertyName == u"Slapped") {
            if (propertyNode.firstChild().nodeName() == "Enable") {
                beat->setSlapped(true);
            }
        } else if (propertyName == u"Brush") {
            beat->setBrush(brushType(propertyNode.firstChild().toElement().text()));
        } else if (propertyName == u"VibratoWTremBar") {
            beat->setVibratoWTremBar(vibratoType(propertyNode.firstChild().toElement().text()));
        } else if (propertyName == u"Rasgueado") {
            beat->setRasgueado(rasgueadoType(propertyNode.firstChild().toElement().text()));
        } else if (propertyName == u"PickStroke") {
            beat->setPickStroke(pickStrokeType(propertyNode.firstChild().toElement().text()));
        } else if (propertyName == u"BarreFret") {
            beat->setBarreFret(propertyNode.firstChild().toElement().text().toInt());
        } else if (propertyName == u"BarreString") {
            beat->setBarreString(propertyNode.firstChild().toElement().text().toInt());
        } else if (propertyName == u"WhammyBar") {
            beat->setDive(true);
        }
        /// TODO: implement dive
//        else if (propertyName == u"WhammyBarDestinationOffset") {
//        } else if (propertyName == u"WhammyBarDestinationValue") {
//        } else if (propertyName == u"WhammyBarMiddleOffset1") {
//        } else if (propertyName == u"WhammyBarMiddleOffset2") {
//        } else if (propertyName == u"WhammyBarMiddleValue") {
//        } else if (propertyName == u"WhammyBarOriginValue") {
//        }

        propertyNode = propertyNode.nextSibling();
    }
}

void GP67DomBuilder::readTrackProperties(XmlDomNode* propertiesNode, GPTrack* track, bool ignoreTuningFlats) const
{
    GPTrack::StaffProperty property;
    property.ignoreFlats = ignoreTuningFlats;

    auto propertyNode = propertiesNode->firstChild();

    while (!propertyNode.isNull()) {
        String propertyName = propertyNode.toElement().attribute("name").value();

        if (propertyName == u"CapoFret") {
            property.capoFret = propertyNode.firstChild().toElement().text().toInt();
        } else if (propertyName == u"FretCount") {
            property.fretCount = propertyNode.firstChild().toElement().text().toInt();
        } else if (propertyName == "Tuning") {
            String tunningStr = propertyNode.firstChildElement("Pitches").text();
            std::vector<int> tunning;
            tunning.reserve(6);
            for (const String& val : tunningStr.split(u' ')) {
                tunning.push_back(val.toInt());
            }
            property.tunning.swap(tunning);
            property.useFlats = !propertyNode.firstChildElement("Flat").isNull();
        } else if (propertyName == u"TuningFlat") {
            property.useFlats = !propertyNode.firstChildElement("Enable").isNull();
        } else if (propertyName == u"DiagramCollection" || propertyName == u"DiagramWorkingSet") {
            readDiagram(propertyNode.firstChild(), track);
        }

        propertyNode = propertyNode.nextSibling();
    }

    track->addStaffProperty(property);
}

void GP67DomBuilder::readDiagram(const XmlDomNode& items, GPTrack* track) const
{
    auto item = items.firstChild();

    while (!item.isNull()) {
        GPTrack::Diagram diagram;

        diagram.id = item.toElement().attribute("id").value().toInt();
        diagram.name = item.toElement().attribute("name").value();

        auto diagramNode = item.firstChild();
        diagram.stringCount = diagramNode.toElement().attribute("stringCount").value().toInt();
        diagram.fretCount = diagramNode.toElement().attribute("fretCount").value().toInt();
        diagram.baseFret = diagramNode.toElement().attribute("baseFret").value().toInt();

        auto fretNode = diagramNode.firstChild();
        while (!fretNode.isNull()) {
            if (fretNode.nodeName() == "Fret") {
                int string = fretNode.toElement().attribute("string").value().toInt();
                int fret = fretNode.toElement().attribute("fret").value().toInt();
                diagram.frets[string] = fret;
            }

            fretNode = fretNode.nextSibling();
        }

        track->addDiagram(std::make_pair(diagram.id, diagram));

        item = item.nextSibling();
    }
}

void GP67DomBuilder::readLyrics(const XmlDomNode& items, GPTrack* track) const
{
    XmlDomNode lineNode = items.firstChildElement("Line");

    // This code doesn't support multiple lines of lyrics.
    if (!lineNode.isNull()) {
        auto textNode = lineNode.firstChildElement("Text");
        if (!textNode.isNull()) {
            track->setLyrics(textNode.toElement().text().toStdString());
        }

        auto offsetNode = lineNode.firstChildElement("Offset");
        if (!offsetNode.isNull()) {
            track->setLyricsOffset(offsetNode.toElement().text().toInt());
        }
    }
}

std::vector<int> GP67DomBuilder::readEnding(XmlDomNode* endNode) const
{
    String str = endNode->toElement().text();
    StringList strList = str.split(u' ');
    std::vector<int> ending;
    ending.reserve(strList.size());
    for (const String& val : strList) {
        ending.push_back(val.toInt());
    }

    return ending;
}

GPMasterBar::Repeat GP67DomBuilder::readRepeat(XmlDomNode* repeatNode) const
{
    auto repeatType = [](const String& start, const String& end) {
        if (start == u"true" && end == u"false") {
            return GPMasterBar::Repeat::Type::Start;
        } else if (start == u"false" && end == u"true") {
            return GPMasterBar::Repeat::Type::End;
        } else if (start == u"true" && end == u"true") {
            return GPMasterBar::Repeat::Type::StartEnd;
        }
        return GPMasterBar::Repeat::Type::None;
    };

    String start = repeatNode->toElement().attribute("start").value();
    String end = repeatNode->toElement().attribute("end").value();
    int count = repeatNode->toElement().attribute("count").value().toInt();

    GPMasterBar::Repeat repeat{ repeatType(start, end), count };
    return repeat;
}

std::pair<String, String>
GP67DomBuilder::readMasterBarSection(const XmlDomNode& sectionNode) const
{
    std::pair<String, String> section;

    auto node = sectionNode.firstChild();

    while (!node.isNull()) {
        String nodeName = node.nodeName();

        if (nodeName == u"Letter") {
            section.first = node.toElement().text();
        } else if (nodeName == u"Text") {
            section.second = node.toElement().text();
        }

        node = node.nextSibling();
    }

    return section;
}

std::vector<GPMasterBar::Fermata> GP67DomBuilder::readFermatas(XmlDomNode* fermatasNode) const
{
    auto fermataType = [](const String& str) {
        if (str == u"Short") {
            return GPMasterBar::Fermata::Type::Short;
        } else if (str == u"Medium") {
            return GPMasterBar::Fermata::Type::Medium;
        }
        return GPMasterBar::Fermata::Type::Long;
    };

    std::vector<GPMasterBar::Fermata> fermatas;

    auto fermataNode = fermatasNode->firstChild();

    while (!fermataNode.isNull()) {
        GPMasterBar::Fermata fermata;

        auto fermataProperty = fermataNode.firstChild();
        while (!fermataProperty.isNull()) {
            String nodeName = fermataProperty.nodeName();
            if (nodeName == u"Type") {
                fermata.type = fermataType(fermataProperty.toElement().text());
            } else if (nodeName == u"Offset") {
                String str = fermataProperty.toElement().text();
                StringList numbers = str.split(u'/');
                fermata.offsetNum = numbers.at(0).toInt();
                fermata.offsetDenom = numbers.at(1).toInt();
            } else if (nodeName == u"Length") {
                fermata.length = fermataProperty.toElement().text().toFloat();
            }

            fermataProperty = fermataProperty.nextSibling();
        }

        fermatas.push_back(fermata);
        fermataNode = fermataNode.nextSibling();
    }

    return fermatas;
}

std::vector<GPMasterBar::Direction> GP67DomBuilder::readRepeatsJumps(XmlDomNode* repeatsJumpsNode) const
{
    std::vector<GPMasterBar::Direction> repeatsJumps;

    auto innerNode = repeatsJumpsNode->firstChild();

    while (!innerNode.isNull()) {
        GPMasterBar::Direction repeatJump;
        repeatJump.type = (innerNode.nodeName() == "Jump" ? GPMasterBar::Direction::Type::Jump : GPMasterBar::Direction::Type::Repeat);
        repeatJump.name = innerNode.toElement().text();

        // GP encodes "To Coda" instructions as Jumps, but MuseScore uses Markers for that
        if ((repeatJump.name == u"DaCoda") || (repeatJump.name == u"DaDoubleCoda")) {
            repeatJump.type = GPMasterBar::Direction::Type::Marker;
        }

        repeatsJumps.push_back(repeatJump);
        innerNode = innerNode.nextSibling();
    }

    return repeatsJumps;
}
} // namespace mu::iex::guitarpro
