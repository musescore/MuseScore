#include "gp67dombuilder.h"

#include <set>

#include "global/log.h"
#include "types/constants.h"

namespace Ms {
GP67DomBuilder::GP67DomBuilder()
{
    _gpDom = std::make_unique<GPDomModel>();
}

void GP67DomBuilder::buildGPDomModel(QDomElement* qdomElem)
{
    QDomNode revision;
    // Score node
    QDomNode scoreNode,        masterTrack,      eachTrack,
             masterBars,           bars,             voices,
             beats,                notes,                  rhythms;

    // Currently ignored
    QDomNode gpversion, encoding;

    std::map<QString, QDomNode*> nodeMap =
    {
        { "GPRevision",    &revision },
        { "Score",               &scoreNode },
        { "MasterTrack",   &masterTrack },
        { "Tracks",        &eachTrack },
        { "MasterBars",    &masterBars },
        { "Bars",                &bars },
        { "Voices",        &voices },
        { "Beats",               &beats },
        { "Notes",               &notes },
        { "Rhythms",       &rhythms },
        { "GPVersion",   &gpversion },
        { "Encoding",    &encoding }
    };

    auto assignMap = [&nodeMap](QDomNode node)
    {
        auto iter = nodeMap.find(node.nodeName());
        if (iter != nodeMap.end()) {
            *(iter->second) = node;
        } else {
            QString nodeName = node.nodeName();
            LOGW() << "unknown node " << nodeName << "\n";
        }
    };

    for (QDomNode current = qdomElem->firstChild(); !current.isNull(); current = current.nextSibling()) {
        assignMap(current);
    }

    std::unordered_map<int, std::shared_ptr<GPRhytm> > rhytms;
    buildGPRhythms(&rhythms);
    buildGPNotes(&notes);
    buildGPBeats(&beats);
    buildGPVoices(&voices);
    buildGPBars(&bars);
    buildGPMasterBars(&masterBars);

    buildGPScore(&scoreNode);
    buildGPMasterTracks(&masterTrack);
    buildGPAudioTracks(&eachTrack);
    buildGPTracks(&eachTrack);
}

std::unique_ptr<GPDomModel> GP67DomBuilder::getGPDomModel()
{
    return std::move(_gpDom);
}

void GP67DomBuilder::buildGPScore(QDomNode* scoreNode)
{
    // Contains list of unused info
    static const std::set<QString> sUnusedNodes = {
        "FirstPageFooter", "FirstPageHeader",
        "PageFooter", "PageHeader",
        "ScoreSystemsDefaultLayout", "ScoreSystemsLayout", "PageSetup",
        "MultiVoice"
    };

    std::unique_ptr<GPScore> score = std::make_unique<GPScore>();
    QDomNode currentNode = scoreNode->firstChild();
    while (!currentNode.isNull()) {
        QString nodeName = currentNode.nodeName();
        if (!nodeName.compare("Title")) {
            score->setTitle(currentNode.toElement().text());
        } else if (!nodeName.compare("Subtitle") || !nodeName.compare("SubTitle")) {
            score->setSubTitle(currentNode.toElement().text());
        } else if (!nodeName.compare("Artist")) {
            score->setArtist(currentNode.toElement().text());
        } else if (!nodeName.compare("Album")) {
            score->setAlbum(currentNode.toElement().text());
        } else if (!nodeName.compare("Words")) {
            score->setPoet(currentNode.toElement().text());
        } else if (!nodeName.compare("Music")) {
            score->setComposer(currentNode.toElement().text());
        } else if (!nodeName.compare("Copyright")) {
            // Currently we ignore Copyright info
        } else if (!nodeName.compare("Tabber")) {
            // Currently we ignore Tabber info
        } else if (!nodeName.compare("Instructions") || !nodeName.compare("Notices")) {
            // Currently we ignore score unrelated texts
        } else if (sUnusedNodes.find(nodeName) != sUnusedNodes.end()) {
            // Ignored nodes, which specify unused specifics (e.g. default layout, footers e.t.c.)
        } else {
            LOGW() << "unknown GP score info tag: " << nodeName << "\n";
        }
        currentNode = currentNode.nextSibling();
    }
    _gpDom->addGPScore(std::move(score));
}

void GP67DomBuilder::buildGPMasterTracks(QDomNode* masterTrack)
{
    std::unique_ptr<GPMasterTracks> masterTracks = std::make_unique<GPMasterTracks>();

    QDomNode currentNode = masterTrack->firstChild();
    while (!currentNode.isNull()) {
        QString nodeName = currentNode.nodeName();
        if (!nodeName.compare("Automations")) {
            masterTracks->setTempoMap(readTempoMap(&currentNode));
        } else if (!nodeName.compare("RSE")) {
            //! TODO volume and pan(balance) of mixer are set here
        } else if (!nodeName.compare("Tracks")) {
            auto tracks = currentNode.toElement().text();
            size_t tracksCount = tracks.split(" ").count();
            masterTracks->setTracksCount(tracksCount);
        } else {
            LOGW() << "unknown GP MasterTracks tag: " << nodeName << "\n";
        }
        currentNode = currentNode.nextSibling();
    }

    _gpDom->addGPMasterTracks(std::move(masterTracks));
}

void GP67DomBuilder::buildGPAudioTracks(QDomNode* audioTrack)
{
    QDomNode currentNode = audioTrack->firstChild();
    while (!currentNode.isNull()) {
        currentNode = currentNode.nextSibling();
    }
}

void GP67DomBuilder::buildGPTracks(QDomNode* tracksNode)
{
    std::map<int, std::unique_ptr<GPTrack> > tracks;
    QDomNode currentNode = tracksNode->firstChild();
    while (!currentNode.isNull()) {
        tracks.insert(createGPTrack(&currentNode));
        currentNode = currentNode.nextSibling();
    }

    _gpDom->addGPTracks(std::move(tracks));
}

void GP67DomBuilder::buildGPMasterBars(QDomNode* masterBarsNode)
{
    std::vector<std::unique_ptr<GPMasterBar> > masterBars;

    QDomNode innerNode = masterBarsNode->firstChild();
    int masterBarIdx = 0;
    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();

        if (nodeName == "MasterBar") {
            masterBars.push_back(createGPMasterBar(&innerNode));
            masterBars.back()->setId(masterBarIdx);
            masterBarIdx++;
        }

        innerNode = innerNode.nextSibling();
    }

    _gpDom->addGPMasterBars(std::move(masterBars));
}

void GP67DomBuilder::buildGPBars(QDomNode* barsNode)
{
    std::unordered_map<int, std::unique_ptr<GPBar> > bars;

    QDomNode innerNode = barsNode->firstChild();
    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();
        if (nodeName == "Bar") {
            bars.insert(createGPBar(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _bars.swap(bars);
}

void GP67DomBuilder::buildGPVoices(QDomNode* voicesNode)
{
    std::unordered_map<int, std::unique_ptr<GPVoice> > voices;

    QDomNode innerNode = voicesNode->firstChild();
    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();
        if (nodeName == "Voice") {
            voices.insert(createGPVoice(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _voices.swap(voices);
}

void GP67DomBuilder::buildGPBeats(QDomNode* beatsNode)
{
    std::unordered_map<int, std::shared_ptr<GPBeat> > beats;

    QDomNode innerNode = beatsNode->firstChild();
    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();
        if (nodeName == "Beat") {
            beats.insert(createGPBeat(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _beats.swap(beats);
}

void GP67DomBuilder::buildGPNotes(QDomNode* notesNode)
{
    std::unordered_map<int, std::shared_ptr<GPNote> > notes;

    QDomNode innerNode = notesNode->firstChild();

    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();
        if (nodeName == "Note") {
            notes.insert(createGPNote(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _notes.swap(notes);
}

void GP67DomBuilder::buildGPRhythms(QDomNode* rhythmsNode)
{
    std::unordered_map<int, std::shared_ptr<GPRhytm> > rhytms;

    QDomNode innerNode = rhythmsNode->firstChild();
    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();
        if (nodeName == "Rhythm") {
            rhytms.insert(createGPRhythm(&innerNode));
        }

        innerNode = innerNode.nextSibling();
    }

    _rhytms.swap(rhytms);
}

std::vector<GPMasterTracks::Automation> GP67DomBuilder::readTempoMap(QDomNode* currentNode)
{
    std::vector<GPMasterTracks::Automation> tempoMap;
    QDomNode currentAutomation = currentNode->firstChild();
    while (!currentAutomation.isNull()) {
        if (!currentAutomation.nodeName().compare("Automation")) {
            auto ln = currentAutomation.firstChildElement("Linear");
            auto first_name = currentAutomation.firstChild().nodeName();
            if (first_name == "Type") {
                first_name = currentAutomation.firstChild().toElement().text();
            }
            if (!first_name.compare("Tempo")) {
                GPMasterTracks::Automation tempo;
                tempo.type = GPMasterTracks::Automation::Type::tempo;
                auto str = currentAutomation.firstChildElement("Value").toElement().text();
                auto tempoValue = str.split(" ");
                tempo.value = tempoValue[0].toInt();
                tempo.tempoUnit = tempoValue.size() > 1 ? tempoValue[1].toInt() : 0;
                tempo.bar = currentAutomation.firstChildElement("Bar").text().toInt();
                tempo.position = currentAutomation.firstChildElement("Position").text().toFloat();
                tempo.linear = (ln.toElement().text() == "true");
                tempoMap.push_back(tempo);
            }
        }
        currentAutomation = currentAutomation.nextSibling();
    }

    return tempoMap;
}

std::unique_ptr<GPMasterTracks> GP67DomBuilder::createGPMasterTrack(QDomNode* metadata)
{
    UNUSED(metadata);
    return std::make_unique<GPMasterTracks>();
}

std::unique_ptr<GPAudioTrack> GP67DomBuilder::createGPAudioTrack(QDomNode* metadata)
{
    UNUSED(metadata);
    return nullptr;
}

std::unique_ptr<GPMasterBar> GP67DomBuilder::createGPMasterBar(QDomNode* masterBarNode)
{
    static const std::set<QString> sUnused = {
        "XProperties"
    };

    auto tripletFeelType = [](auto&& str) {
        if (str == "Triplet8th") {
            return GPMasterBar::TripletFeelType::Triplet8th;
        }
        if (str == "Triplet16th") {
            return GPMasterBar::TripletFeelType::Triplet16th;
        }
        if (str == "Dotted8th") {
            return GPMasterBar::TripletFeelType::Dotted8th;
        }
        if (str == "Dotted16th") {
            return GPMasterBar::TripletFeelType::Dotted16th;
        }
        if (str == "Scottish8th") {
            return GPMasterBar::TripletFeelType::Scottish8th;
        }
        if (str == "Scottish16th") {
            return GPMasterBar::TripletFeelType::Scottish16th;
        }
        return GPMasterBar::TripletFeelType::None;
    };

    std::unique_ptr<GPMasterBar> masterBar = std::make_unique<GPMasterBar>();

    auto innerNode = masterBarNode->firstChild();

    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();

        if (nodeName == "Time") {
            masterBar->setTimeSig(readTimeSig(&innerNode));
        } else if (nodeName == "Repeat") {
            masterBar->setRepeat(readRepeat(&innerNode));
        } else if (nodeName == "AlternateEndings") {
            masterBar->setAlternativeEnding(readEnding(&innerNode));
        } else if (nodeName == "Key") {
            masterBar->setKeySig(readKeySig(&innerNode));
        } else if (nodeName == "Bars") {
            const auto& barsElemet = innerNode.toElement().text();
            const auto& bars = barsElemet.split(" ");
            for (const auto& barIdx : bars) {
                int idx = barIdx.toInt();
                std::unique_ptr<GPBar> bar;
                bar = std::move(_bars.at(idx));
                _bars.erase(idx);
                masterBar->addGPBar(std::move(bar));
            }
        } else if (nodeName == "TripletFeel") {
            masterBar->setTripletFeel(tripletFeelType(innerNode.toElement().text()));
        } else if (nodeName == "Fermatas") {
            masterBar->setFermatas(readFermatas(&innerNode));
        } else if (nodeName == "Section") {
            masterBar->setSection(readMasterBarSection(innerNode));
        } else if (nodeName == "Directions") {
            if (innerNode.firstChild().nodeName() == "Jump") {
                masterBar->setDirectionJump(innerNode.firstChild().toElement().text());
            } else {
                masterBar->setDirectionTarget(innerNode.firstChild().toElement().text());
            }
        } else if (nodeName == "DoubleBar") {
            masterBar->setBarlineType(GPMasterBar::BarlineType::DOUBLE);
        } else if (nodeName == "FreeTime") {
            masterBar->setFreeTime(true);
        } else if (sUnused.find(nodeName) != sUnused.end()) {
            // Ignored
        } else {
            LOGW() << "unknown GP MasterBar tag: " << nodeName << "\n";
        }

        innerNode = innerNode.nextSibling();
    }

    return masterBar;
}

std::pair<int, std::unique_ptr<GPBar> > GP67DomBuilder::createGPBar(QDomNode* barNode)
{
    static const std::set<QString> sUnused = {
        "XProperties"
    };

    auto clefType = [](const QString& clef) {
        if (clef == "C4") {
            return GPBar::ClefType::C4;
        } else if (clef == "C3") {
            return GPBar::ClefType::C3;
        } else if (clef == "F4") {
            return GPBar::ClefType::F4;
        } else if (clef == "G2") {
            return GPBar::ClefType::G2;
        } else {
            return GPBar::ClefType::Neutral;
        }
    };

    auto ottaviaType = [](const QString& ott) {
        if (ott == "8va") {
            return GPBar::OttaviaType::va8;
        } else if (ott == "15ma") {
            return GPBar::OttaviaType::ma15;
        } else if (ott == "8vb") {
            return GPBar::OttaviaType::vb8;
        } else if (ott == "15mb") {
            return GPBar::OttaviaType::mb15;
        } else {
            return GPBar::OttaviaType::Regular;
        }
    };
    auto simileMarkType = [](const auto& str) {
        if (str == "Simple") {
            return GPBar::SimileMark::Simple;
        } else if (str == "FirstOfDouble") {
            return GPBar::SimileMark::FirstOfDouble;
        } else if (str == "SecondOfDouble") {
            return GPBar::SimileMark::SecondOfDouble;
        } else {
            return GPBar::SimileMark::None;
        }
    };

    std::unique_ptr<GPBar> bar = std::make_unique<GPBar>();

    auto innerNode = barNode->firstChild();
    int barIdx = barNode->attributes().namedItem("id").toAttr().value().toInt();
    bar->setId(barIdx);

    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();

        if (nodeName == "Clef") {
            bar->setClefType(clefType(innerNode.toElement().text()));
        } else if (nodeName == "Ottavia") {
            bar->setOttaviaType(ottaviaType(innerNode.toElement().text()));
        } else if (nodeName == "SimileMark") {
            bar->setSimileMark(simileMarkType(innerNode.toElement().text()));
        } else if (nodeName == "Voices") {
            auto voicesElemet = innerNode.toElement().text();
            auto voices = voicesElemet.split(" ");
            for (const auto& voiceIdx : voices) {
                int idx = voiceIdx.toInt();
                if (idx == -1) {
                    continue;
                }
                std::unique_ptr<GPVoice> voice;
                voice = std::move(_voices.at(idx));
                _voices.erase(idx);
                bar->addGPVoice(std::move(voice));
            }
        } else if (sUnused.find(nodeName) != sUnused.end()) {
            // Ignored
        } else {
            LOGW() << "unknown GP Bar tag: " << nodeName << "\n";
        }
        innerNode = innerNode.nextSibling();
    }

    return std::make_pair(barIdx, std::move(bar));
}

std::pair<int, std::unique_ptr<GPVoice> > GP67DomBuilder::createGPVoice(QDomNode* voiceNode)
{
    std::unique_ptr<GPVoice> voice = std::make_unique<GPVoice>();

    auto innerNode = voiceNode->firstChild();
    int voiceIdx = voiceNode->attributes().namedItem("id").toAttr().value().toInt();
    voice->setId(voiceIdx);

    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();
        if (nodeName == "Beats") {
            auto beatsElemet = innerNode.toElement().text();
            auto beats = beatsElemet.split(" ");
            for (const auto& beatIdx : beats) {
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

std::pair<int, std::shared_ptr<GPBeat> > GP67DomBuilder::createGPBeat(QDomNode* beatNode)
{
    static const std::set<QString> sUnused = {
        "Bank",
        "StemOrientation", "ConcertPitchStemOrientation", "TransposedPitchStemOrientation",
        "Ottavia"
    };

    auto dynamicType = [](auto&& str) -> GPBeat::DynamicType {
        if (str == "FFF") {
            return GPBeat::DynamicType::FFF;
        } else if (str == "FF") {
            return GPBeat::DynamicType::FF;
        } else if (str == "F") {
            return GPBeat::DynamicType::F;
        } else if (str == "MF") {
            return GPBeat::DynamicType::MF;
        } else if (str == "MP") {
            return GPBeat::DynamicType::MP;
        } else if (str == "P") {
            return GPBeat::DynamicType::P;
        } else if (str == "PP") {
            return GPBeat::DynamicType::PP;
        } else {
            return GPBeat::DynamicType::PPP;
        }
    };
    auto legatoType = [](const auto& origin, const auto& destination) {
        if (origin == "true" && destination == "false") {
            return GPBeat::LegatoType::Start;
        } else if (origin == "true" && destination == "true") {
            return GPBeat::LegatoType::Mediate;
        } else if (origin == "false" && destination == "true") {
            return GPBeat::LegatoType::End;
        } else {
            return GPBeat::LegatoType::None;
        }
    };
    auto arpeggioType = [](const auto& arp) {
        if (arp == "Up") {
            return GPBeat::Arpeggio::Up;
        } else if (arp == "Down") {
            return GPBeat::Arpeggio::Down;
        }
        return GPBeat::Arpeggio::None;
    };
    auto graceNotes = [](const auto& gn) {
        if (gn == "OnBeat") {
            return GPBeat::GraceNotes::OnBeat;
        } else if (gn == "BeforeBeat") {
            return GPBeat::GraceNotes::BeforeBeat;
        }
        return GPBeat::GraceNotes::None;
    };
    auto faddingType = [](const auto& str) {
        if (str == "FadeIn") {
            return GPBeat::Fadding::FadeIn;
        } else if (str == "FadeOut") {
            return GPBeat::Fadding::FadeOut;
        } else if (str == "VolumeSwell") {
            return GPBeat::Fadding::VolumeSwell;
        }
        return GPBeat::Fadding::None;
    };
    auto hairpinType = [](const auto& str) {
        if (str == "Crescendo") {
            return GPBeat::Hairpin::Crescendo;
        } else if (str == "Decrescendo") {
            return GPBeat::Hairpin::Decrescendo;
        }
        return GPBeat::Hairpin::None;
    };
    auto wahType = [](const auto& str) {
        if (str == "Open") {
            return GPBeat::Wah::Open;
        } else if (str == "Closed") {
            return GPBeat::Wah::Closed;
        } else {
            return GPBeat::Wah::None;
        }
    };

    auto ottavaType = [](const QString& ott) {
        if (ott == "8va") {
            return GPBeat::OttavaType::va8;
        } else if (ott == "15ma") {
            return GPBeat::OttavaType::ma15;
        } else if (ott == "8vb") {
            return GPBeat::OttavaType::vb8;
        } else if (ott == "15mb") {
            return GPBeat::OttavaType::mb15;
        }
        LOGE() << "wrong ottava type: " << ott;
        return GPBeat::OttavaType::None;
    };

    std::shared_ptr<GPBeat> beat = std::make_shared<GPBeat>();

    auto innerNode = beatNode->firstChild();
    int beatIdx = beatNode->attributes().namedItem("id").toAttr().value().toInt();
    beat->setId(beatIdx);

    while (!innerNode.isNull()) {
        auto nodeName = innerNode.nodeName();

        if (nodeName == "Dynamic") {
            GPBeat::DynamicType dynamic = dynamicType(innerNode.toElement().text());
            beat->setDynamic(dynamic);
        } else if (nodeName == "Legato") {
            auto origin = innerNode.attributes().namedItem("origin").toAttr().value();
            auto destination = innerNode.attributes().namedItem("destination").toAttr().value();
            GPBeat::LegatoType legato = legatoType(origin, destination);
            beat->setLegatoType(legato);
        } else if (nodeName == "Rhythm") {
            auto rIdx = innerNode.attributes().namedItem("ref").toAttr().value().toInt();
            beat->addGPRhytm(_rhytms.at(rIdx));
        } else if (nodeName == "Notes") {
            auto notesStr = innerNode.toElement().text();
            auto strList = notesStr.split(" ");
            for (const auto& strIdx : strList) {
                int idx = strIdx.toInt();
                std::shared_ptr<GPNote> note;
                note = _notes.at(idx);
                beat->addGPNote(note);
            }
        } else if (nodeName == "GraceNotes") {
            beat->setGraceNotes(graceNotes(innerNode.toElement().text()));
        } else if (nodeName == "Arpeggio") {
            beat->setArpeggio(arpeggioType(innerNode.toElement().text()));
        } else if (nodeName == "Properties") {
            readBeatProperties(innerNode, beat.get());
        } else if (nodeName == "Chord") {
            beat->setDiagramIdx(innerNode.toElement().text().toInt());
        } else if (nodeName == "Timer") {
            beat->setTime(innerNode.toElement().text().toInt());
        } else if (nodeName == "FreeText") {
            beat->setFreeText(innerNode.toElement().text());
        } else if (nodeName == "Fadding") {
            beat->setFadding(faddingType(innerNode.toElement().text()));
        } else if (nodeName == "Hairpin") {
            beat->setHairpin(hairpinType(innerNode.toElement().text()));
        } else if (nodeName == "Tremolo") {
            GPBeat::Tremolo tr;
            tr.enumerator = innerNode.toElement().text().split("/").at(0).toInt();
            tr.denumerator = innerNode.toElement().text().split("/").at(1).toInt();
            beat->setTremolo(tr);
        } else if (nodeName == "Wah") {
            beat->setWah(wahType(innerNode.toElement().text()));
        } else if (nodeName == "Lyrics") {
            // this code is almost a copy-paste from android_improvement.
            // it reads lyrics for the beat (only one line).

            QDomElement lyrNode = innerNode.firstChildElement("Line");
            QString str = lyrNode.toElement().text();
            beat->setLyrics(str.toStdString());
        } else if (nodeName == "Ottavia") {
            beat->setOttavaType(ottavaType(innerNode.toElement().text()));
        } else if (nodeName == "XProperties") {
            readBeatXProperties(innerNode, beat.get());
        } else if (sUnused.find(nodeName) != sUnused.end()) {
            // Ignored nodes
        } else {
            LOGW() << "unknown GP Beat Tag " << nodeName << "\n";
        }

        innerNode = innerNode.nextSibling();
    }

    return std::make_pair(beatIdx, std::move(beat));
}

std::pair<int, std::shared_ptr<GPNote> > GP67DomBuilder::createGPNote(QDomNode* noteNode)
{
    auto tieType = [](const QString& origin, const QString& destination) ->GPNote::TieType {
        if (origin == "true" && destination == "false") {
            return GPNote::TieType::Start;
        } else if (origin == "true" && destination == "true") {
            return GPNote::TieType::Mediate;
        } else if (origin == "false" && destination == "true") {
            return GPNote::TieType::End;
        } else {
            return GPNote::TieType::None;
        }
    };
    auto vibratoType = [](const auto& str) {
        if (str == "Wide") {
            return GPNote::VibratoType::Wide;
        } else if (str == "Slight") {
            return GPNote::VibratoType::Slight;
        } else {
            return GPNote::VibratoType::None;
        }
    };
    auto ornametType = [](const auto& str) {
        if (str == "UpperMordent") {
            return GPNote::Ornament::UpperMordent;
        } else if (str == "LowerMordent") {
            return GPNote::Ornament::LowerMordent;
        } else if (str == "InvertedTurn") {
            return GPNote::Ornament::InvertedTurn;
        } else if (str == "Turn") {
            return GPNote::Ornament::Turn;
        } else {
            return GPNote::Ornament::None;
        }
    };

    auto note = std::make_shared<GPNote>();
    int noteIdx = noteNode->attributes().namedItem("id").toAttr().value().toInt();
    note->setId(noteIdx);

    auto innerNode = noteNode->firstChild();
    while (!innerNode.isNull()) {
        QString nodeName = innerNode.nodeName();
        if (nodeName == "InstrumentArticulation") {
        }
        if (nodeName == "Properties") {
            readNoteProperties(&innerNode, note.get());
        }
        if (nodeName == "XProperties") {
            readNoteXProperties(innerNode, note.get());
        }
        if (nodeName == "Tie") {
            auto origin = innerNode.attributes().namedItem("origin").toAttr().value();
            auto destination = innerNode.attributes().namedItem("destination").toAttr().value();
            GPNote::TieType tie = tieType(origin, destination);
            note->setTieType(tie);
        }
        if (nodeName == "LetRing") {
            note->setLetRing(true);
        } else if (nodeName == "AntiAccent") {
            note->setGhostNote(innerNode.toElement().text() == "Normal");
        } else if (nodeName == "Accent") {
            note->setAccent(innerNode.toElement().text().toUInt());
        } else if (nodeName == "LeftFingering") {
            QString finger = innerNode.toElement().text();
            if (finger == "Open") {
                finger = "0";
            } else if (finger == "P") {
                finger = "t";
            } else if (finger == "I") {
                finger = "1";
            } else if (finger == "M") {
                finger = "2";
            } else if (finger == "A") {
                finger = "3";
            } else if (finger == "C") {
                finger = "4";
            }
            note->setLeftFingering(finger);
        } else if (nodeName == "RightFingering") {
            note->setRightFingering(innerNode.toElement().text().toLower());
        } else if (nodeName == "Vibrato") {
            note->setVibratoType(vibratoType(innerNode.toElement().text()));
        } else if (nodeName == "Trill") {
            note->setTrillFret(innerNode.toElement().text().toInt());
        } else if (nodeName == "Ornament") {
            note->setOrnament(ornametType(innerNode.toElement().text()));
        } else {
            LOGD() << "unknown GP Note Tag" << nodeName << "\n";
        }

        innerNode = innerNode.nextSibling();
    }

    return std::make_pair(noteIdx, std::move(note));
}

std::pair<int, std::shared_ptr<GPRhytm> > GP67DomBuilder::createGPRhythm(QDomNode* rhythmNode)
{
    auto rhythmType = [](const QString& str) -> GPRhytm::RhytmType {
        if (str == "Whole") {
            return GPRhytm::RhytmType::Whole;
        } else if (str == "Half") {
            return GPRhytm::RhytmType::Half;
        } else if (str == "Quarter") {
            return GPRhytm::RhytmType::Quarter;
        } else if (str == "Eighth") {
            return GPRhytm::RhytmType::Eighth;
        } else if (str == "16th") {
            return GPRhytm::RhytmType::Sixteenth;
        } else if (str == "32nd") {
            return GPRhytm::RhytmType::ThirtySecond;
        } else {
            return GPRhytm::RhytmType::SixtyFourth;
        }
    };

    std::shared_ptr<GPRhytm> rhythm = std::make_shared<GPRhytm>();

    auto innerNode = rhythmNode->firstChild();
    int rhythmIdx = rhythmNode->attributes().namedItem("id").toAttr().value().toInt();
    while (!innerNode.isNull()) {
        QString nodeName = innerNode.nodeName();
        if (nodeName == "NoteValue") {
            auto rhType = rhythmType(innerNode.toElement().text());
            rhythm->setRhytm(rhType);
        }
        if (nodeName == "AugmentationDot") {
            rhythm->setDotCount(innerNode.attributes().namedItem("count").toAttr().value().toInt());
        } else if (nodeName == "PrimaryTuplet") {
            int num = innerNode.attributes().namedItem("num").toAttr().value().toInt();
            int denum = innerNode.attributes().namedItem("den").toAttr().value().toInt();
            rhythm->setTuplet({ num, denum });
        } else {
            LOGD() << "unknown GP Rhytms tag" << nodeName << "\n";
        }

        innerNode = innerNode.nextSibling();
    }

    return std::make_pair(rhythmIdx, std::move(rhythm));
}

GPTrack::RSE GP67DomBuilder::readTrackRSE(QDomNode* trackChildNode) const
{
    auto innerNode = trackChildNode->firstChild();
    auto nodeName = innerNode.nodeName();

    if (!innerNode.isNull() && nodeName.compare("ChannelStrip") == 0) {
        innerNode = innerNode.firstChildElement("Parameters");
        if (!innerNode.isNull()) {
            auto str = innerNode.toElement().text();
            auto strList = str.split(" ");
            GPTrack::RSE rse;
            rse.pan = strList[11].toFloat();
            rse.volume = strList[12].toFloat();
            return rse;
        }
    }

    return GPTrack::RSE();
}

GPMasterBar::KeySig GP67DomBuilder::readKeySig(QDomNode* keyNode) const
{
    const auto& accidentalCount = keyNode->firstChildElement("AccidentalCount");
    int keyCount = accidentalCount.toElement().text().toInt();

    return GPMasterBar::KeySig(keyCount);
}

GPMasterBar::TimeSig GP67DomBuilder::readTimeSig(QDomNode* timeNode) const
{
    const auto& time = timeNode->toElement().text();
    const auto& timeSig = time.split("/");
    GPMasterBar::TimeSig sig { timeSig.at(0).toInt(), timeSig.at(1).toInt() };
    return sig;
}

void GP67DomBuilder::readNoteXProperties(const QDomNode& propertiesNode, GPNote* note)
{
    auto propertyNode = propertiesNode.firstChild();

    while (!propertyNode.isNull()) {
        int propertyId = propertyNode.attributes().namedItem("id").toAttr().value().toInt();

        if (propertyId == 688062467) {
            note->setTrillSpeed(propertyNode.firstChild().toElement().text().toInt());
        }

        propertyNode = propertyNode.nextSibling();
    }
}

void GP67DomBuilder::readNoteProperties(QDomNode* propertiesNode, GPNote* note)
{
    std::unordered_set<std::unique_ptr<INoteProperty> > properties;

    auto propetryNode = propertiesNode->firstChild();

    while (!propetryNode.isNull()) {
        auto propertyName = propetryNode.attributes().namedItem("name").toAttr().value();

        if (propertyName == "Midi") {
            int midi = propetryNode.firstChild().toElement().text().toInt();
            note->setMidi(midi);
        }
        if (propertyName == "Variation") {
            note->setVariation(propetryNode.firstChild().toElement().text().toInt());
        }
        if (propertyName == "Element") {
            note->setElement(propetryNode.firstChild().toElement().text().toInt());
        } else if (propertyName == "String") {
            int string = propetryNode.firstChild().toElement().text().toInt();
            note->setString(string);
        } else if (propertyName == "Fret") {
            int fret = propetryNode.firstChild().toElement().text().toInt();
            note->setFret(fret);
        } else if (propertyName == "Octave") {
            int octave = propetryNode.firstChild().toElement().text().toInt();
            note->setOctave(octave);
        } else if (propertyName == "Tone") {
            int tone = propetryNode.firstChild().toElement().text().toInt();
            note->setTone(tone);
        } else if (propertyName == "Bended") {
            if (propetryNode.firstChild().nodeName() == "Enable") {
                note->setBend(createBend(&propetryNode));
            }
        } else if (propertyName == "Harmonic"
                   || propertyName == "HarmonicFret"
                   || propertyName == "HarmonicType") {
            readHarmonic(&propetryNode, note);
        } else if (propertyName == "PalmMuted") {
            if (propetryNode.firstChild().nodeName() == "Enable") {
                note->setPalmMute(true);
            }
        } else if (propertyName == "Muted") {
            //! property muted in GP means dead note
            if (propetryNode.firstChild().nodeName() == "Enable") {
                note->setMute(true);
            }
        } else if (propertyName == "Slide") {
            note->setSlides(propetryNode.firstChild().toElement().text().toUInt());
        } else if (propertyName == "HopoOrigin") {
            note->setHammerOn(GPNote::HammerOn::Start);
        } else if (propertyName == "HopoDestination") {
            note->setHammerOn(GPNote::HammerOn::End);
        } else if (propertyName == "Tapped") {
            if (propetryNode.firstChild().nodeName() == "Enable") {
                note->setTapping(true);
            }
        } else if (propertyName == "LeftHandTapped") {
            if (propetryNode.firstChild().nodeName() == "Enable") {
                note->setLeftHandTapped(true);
            }
        } else {
            LOGD() << "unknown GP Note Property tag" << propertyName << "\n";
        }

        propetryNode = propetryNode.nextSibling();
    }

    note->addProperties(std::move(properties));
    return;
}

void GP67DomBuilder::readBeatXProperties(const QDomNode& propertiesNode, GPBeat* beat)
{
    auto propertyNode = propertiesNode.firstChild();

    while (!propertyNode.isNull()) {
        int propertyId = propertyNode.attributes().namedItem("id").toAttr().value().toInt();

        if (propertyId == 687931393 || propertyId == 687935489) {
            // arpeggio/brush ticks
            beat->setArpeggioStretch(propertyNode.firstChild().toElement().text().toDouble() / Ms::Constant::division);
        }

        propertyNode = propertyNode.nextSibling();
    }
}

std::unique_ptr<GPNote::Bend> GP67DomBuilder::createBend(QDomNode* propertyNode)
{
    std::unique_ptr<GPNote::Bend> bend = std::make_unique<GPNote::Bend>();

    auto currentNode = propertyNode->nextSibling();
    while (!currentNode.isNull()) {
        auto propertyName = currentNode.attributes().namedItem("name").toAttr().value();
        if (propertyName == "BendDestinationOffset") {
            bend->destinationOffset = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == "BendDestinationValue") {
            bend->destinationValue = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == "BendMiddleOffset1") {
            bend->middleOffset1 = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == "BendMiddleOffset2") {
            bend->middleOffset2 = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == "BendMiddleValue") {
            bend->middleValue = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == "BendOriginOffset") {
            bend->originOffset = currentNode.firstChild().toElement().text().toFloat();
        } else if (propertyName == "BendOriginValue") {
            bend->originValue = currentNode.firstChild().toElement().text().toFloat();
        }

        currentNode = currentNode.nextSibling();
    }

    return bend;
}

void GP67DomBuilder::readHarmonic(QDomNode* propertyNode, GPNote* note) const
{
    auto harmonicType = [](const auto& str) {
        if (str == "Artificial") {
            return GPNote::Harmonic::Type::Artificial;
        } else if (str == "Semi") {
            return GPNote::Harmonic::Type::Semi;
        } else if (str == "Pinch") {
            return GPNote::Harmonic::Type::Pinch;
        } else if (str == "Feedback") {
            return GPNote::Harmonic::Type::FeedBack;
        } else if (str == "Natural") {
            return GPNote::Harmonic::Type::Natural;
        } else {
            return GPNote::Harmonic::Type::Tap;
        }
    };

    auto propertyName = propertyNode->attributes().namedItem("name").toAttr().value();
    if (propertyName == "HarmonicFret") {
        note->setHarmonicFret(propertyNode->firstChild().toElement().text().toFloat());
    } else if (propertyName == "HarmonicType") {
        note->setHarmonicType(harmonicType(propertyNode->firstChild().toElement().text()));
    }
}

void GP67DomBuilder::readBeatProperties(const QDomNode& propertiesNode, GPBeat* beat) const
{
    auto brushType = [](const auto& brush) {
        if (brush == "Down") {
            return GPBeat::Brush::Down;
        }
        return GPBeat::Brush::Up;
    };
    auto vibratoType = [](const auto& str) {
        if (str == "Wide") {
            return GPBeat::VibratoWTremBar::Wide;
        } else if (str == "Slight") {
            return GPBeat::VibratoWTremBar::Slight;
        } else {
            return GPBeat::VibratoWTremBar::None;
        }
    };
    auto rasgueadoType = [](const auto& str) {
        if (str == "ii_1") {
            return GPBeat::Rasgueado::II_1;
        } else if (str == "mi_1") {
            return GPBeat::Rasgueado::MII_1;
        } else if (str == "mii_1") {
            return GPBeat::Rasgueado::MII_1;
        } else if (str == "mii_2") {
            return GPBeat::Rasgueado::MII_2;
        } else if (str == "ami_1") {
            return GPBeat::Rasgueado::AMI_1;
        } else if (str == "ami_2") {
            return GPBeat::Rasgueado::AMI_2;
        } else if (str == "pai_1") {
            return GPBeat::Rasgueado::PAI_1;
        } else if (str == "pai_2") {
            return GPBeat::Rasgueado::PAI_2;
        } else if (str == "pei_1") {
            return GPBeat::Rasgueado::PEI_1;
        } else if (str == "pei_2") {
            return GPBeat::Rasgueado::PEI_2;
        } else if (str == "pmp_2") {
            return GPBeat::Rasgueado::PMP_2;
        } else if (str == "ppp_1") {
            return GPBeat::Rasgueado::PPP_1;
        } else if (str == "amii_1") {
            return GPBeat::Rasgueado::AMII_1;
        } else if (str == "amip_1") {
            return GPBeat::Rasgueado::AMIP_1;
        } else if (str == "eami_1") {
            return GPBeat::Rasgueado::EAMI_1;
        } else if (str == "eamii_1") {
            return GPBeat::Rasgueado::EAMII_1;
        } else if (str == "peami_1") {
            return GPBeat::Rasgueado::PEAMI_1;
        } else {
            return GPBeat::Rasgueado::None;
        }
    };
    auto pickStrokeType = [](const auto& str) {
        if (str == "Up") {
            return GPBeat::PickStroke::Up;
        } else if (str == "Down") {
            return GPBeat::PickStroke::Down;
        }
        return GPBeat::PickStroke::None;
    };

    auto propertyNode = propertiesNode.firstChild();

    while (!propertyNode.isNull()) {
        auto propertyName = propertyNode.attributes().namedItem("name").toAttr().value();

        if (propertyName == "Popped") {
            if (propertyNode.firstChild().nodeName() == "Enable") {
                beat->setPopped(true);
            }
        } else if (propertyName == "Slapped") {
            if (propertyNode.firstChild().nodeName() == "Enable") {
                beat->setSlapped(true);
            }
        } else if (propertyName == "Brush") {
            beat->setBrush(brushType(propertyNode.firstChild().toElement().text()));
        } else if (propertyName == "VibratoWTremBar") {
            beat->setVibrato(vibratoType(propertyNode.firstChild().toElement().text()));
        } else if (propertyName == "Rasgueado") {
            beat->setRasgueado(rasgueadoType(propertyNode.firstChild().toElement().text()));
        } else if (propertyName == "PickStroke") {
            beat->setPickStroke(pickStrokeType(propertyNode.firstChild().toElement().text()));
        } else if (propertyName == "BarreFret") {
            beat->setBarreFret(propertyNode.firstChild().toElement().text().toInt());
        } else if (propertyName == "BarreString") {
            beat->setBarreString(propertyNode.firstChild().toElement().text().toInt());
        } else {
            LOGD() << "unknown GP Beat property info tag: " << propertyName << "\n";
        }

        propertyNode = propertyNode.nextSibling();
    }
}

void GP67DomBuilder::readTrackProperties(QDomNode* propertiesNode, GPTrack* track) const
{
    GPTrack::StaffProperty property;

    auto propertyNode = propertiesNode->firstChild();

    while (!propertyNode.isNull()) {
        auto propertyName = propertyNode.attributes().namedItem("name").toAttr().value();

        if (propertyName == "CapoFret") {
            property.capoFret = propertyNode.firstChild().toElement().text().toInt();
        } else if (propertyName == "FretCount") {
            property.fretCount = propertyNode.firstChild().toElement().text().toInt();
        } else if (propertyName == "Tuning") {
            auto tunningStr = propertyNode.firstChildElement("Pitches").text();
            std::vector<int> tunning;
            tunning.reserve(6);
            for (const auto& val : tunningStr.split(" ")) {
                tunning.push_back(val.toInt());
            }
            property.tunning.swap(tunning);
        } else if (propertyName == "DiagramCollection" || propertyName == "DiagramWorkingSet") {
            readDiagramm(propertyNode.firstChild(), track);
        } else {
            LOGD() << "unknown GP trackProperty info tag: " << propertyName << "\n";
        }

        propertyNode = propertyNode.nextSibling();
    }

    track->addStaffProperty(property);
}

void GP67DomBuilder::readDiagramm(const QDomNode& items, GPTrack* track) const
{
    auto item = items.firstChild();

    while (!item.isNull()) {
        GPTrack::Diagram diagram;

        diagram.id = item.attributes().namedItem("id").toAttr().value().toInt();
        diagram.name = item.attributes().namedItem("name").toAttr().value();

        auto diagrammNode = item.firstChild();
        diagram.stringCount = diagrammNode.attributes().namedItem("stringCount").toAttr().value().toInt();
        diagram.fretCount = diagrammNode.attributes().namedItem("fretCount").toAttr().value().toInt();
        diagram.baseFret = diagrammNode.attributes().namedItem("baseFret").toAttr().value().toInt();

        auto fretNode = diagrammNode.firstChild();
        while (!fretNode.isNull()) {
            if (fretNode.nodeName() == "Fret") {
                int string = fretNode.attributes().namedItem("string").toAttr().value().toInt();
                int fret = fretNode.attributes().namedItem("fret").toAttr().value().toInt();
                diagram.frets[string] = fret;
            }

            fretNode = fretNode.nextSibling();
        }

        track->addDiagram(std::make_pair(diagram.id, diagram));

        item = item.nextSibling();
    }
}

void GP67DomBuilder::readLyrics(const QDomNode& items, GPTrack* track) const
{
    QDomNode lineNode = items.firstChildElement("Line");

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

std::vector<int> GP67DomBuilder::readEnding(QDomNode* endNode) const
{
    auto str = endNode->toElement().text();
    auto strList = str.split(" ");
    std::vector<int> ending;
    ending.reserve(strList.count());
    for (const auto& val : strList) {
        ending.push_back(val.toInt());
    }

    return ending;
}

GPMasterBar::Repeat GP67DomBuilder::readRepeat(QDomNode* repeatNode) const
{
    auto repeatType = [](const QString& start, const QString& end) {
        if (start == "true" && end == "false") {
            return GPMasterBar::Repeat::Type::Start;
        } else if (start == "false" && end == "true") {
            return GPMasterBar::Repeat::Type::End;
        } else if (start == "true" && end == "true") {
            return GPMasterBar::Repeat::Type::StartEnd;
        }
        return GPMasterBar::Repeat::Type::None;
    };

    auto start = repeatNode->attributes().namedItem("start").toAttr().value();
    auto end = repeatNode->attributes().namedItem("end").toAttr().value();
    int count = repeatNode->attributes().namedItem("count").toAttr().value().toInt();

    GPMasterBar::Repeat repeat{ repeatType(start, end), count };
    return repeat;
}

std::pair<QString, QString>
GP67DomBuilder::readMasterBarSection(const QDomNode& sectionNode) const
{
    std::pair<QString, QString> section;

    auto node = sectionNode.firstChild();

    while (!node.isNull()) {
        auto nodeName = node.nodeName();

        if (nodeName == "Letter") {
            section.first = node.toElement().text();
        } else if (nodeName == "Text") {
            section.second = node.toElement().text();
        }

        node = node.nextSibling();
    }

    return section;
}

std::vector<GPMasterBar::Fermata> GP67DomBuilder::readFermatas(QDomNode* fermatasNode) const
{
    auto fermataType = [](const QString& str) {
        if (str == "Short") {
            return GPMasterBar::Fermata::Type::Short;
        } else if (str == "Medium") {
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
            auto nodeName = fermataProperty.nodeName();
            if (nodeName == "Type") {
                fermata.type = fermataType(fermataProperty.toElement().text());
            } else if (nodeName == "Offset") {
                auto str = fermataProperty.toElement().text();
                auto numbers = str.split("/");
                fermata.offsetEnum = numbers[0].toInt();
                fermata.offsetDenum = numbers[1].toInt();
            } else if (nodeName == "Length") {
                fermata.lenght = fermataProperty.toElement().text().toFloat();
            }

            fermataProperty = fermataProperty.nextSibling();
        }

        fermatas.push_back(fermata);
        fermataNode = fermataNode.nextSibling();
    }

    return fermatas;
}
} //end Ms namespace
