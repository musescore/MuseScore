#ifndef MU_IMPORTEXPORT_GP67DOMBUILDER_H
#define MU_IMPORTEXPORT_GP67DOMBUILDER_H

#include <memory>

#include "igpdombuilder.h"
#include "gpdommodel.h"
#include "gptrack.h"

namespace mu::iex::guitarpro {
class GP67DomBuilder : public IGPDomBuilder
{
public:
    GP67DomBuilder();

    void buildGPDomModel(muse::XmlDomElement* domElem) override;
    std::unique_ptr<GPDomModel> getGPDomModel() override;

protected:

    void buildGPScore(muse::XmlDomNode* scoreNode);
    void buildGPMasterTracks(muse::XmlDomNode* masterTrack);
    void buildGPAudioTracks(muse::XmlDomNode* audioTrack);
    void buildGPTracks(muse::XmlDomNode* tracksNode, muse::XmlDomNode* versionNode);
    void buildGPMasterBars(muse::XmlDomNode* masterBars);
    void buildGPBars(muse::XmlDomNode* bars);
    void buildGPVoices(muse::XmlDomNode* voicesNode);
    void buildGPBeats(muse::XmlDomNode* beats);
    void buildGPNotes(muse::XmlDomNode* notesNode);
    void buildGPRhythms(muse::XmlDomNode* rhythms);

    void breakLyricsOnBeatsIfNeed();
    bool isLyricsOnBeats() const;

    virtual std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(muse::XmlDomNode* trackNode, muse::XmlDomNode* versionNode) = 0;

    std::unique_ptr<GPMasterTracks> createGPMasterTrack(muse::XmlDomNode* metadata);
    std::unique_ptr<GPMasterBar> createGPMasterBar(muse::XmlDomNode* masterBarNode);
    std::pair<int, std::unique_ptr<GPBar> > createGPBar(muse::XmlDomNode* barNode);
    std::pair<int, std::unique_ptr<GPVoice> > createGPVoice(muse::XmlDomNode* voiceNode);
    std::pair<int, std::shared_ptr<GPBeat> > createGPBeat(muse::XmlDomNode* beatNode);
    std::pair<int, std::shared_ptr<GPNote> > createGPNote(muse::XmlDomNode* noteNode);
    std::pair<int, std::shared_ptr<GPRhythm> > createGPRhythm(muse::XmlDomNode* rhythmNode);

    void readNoteXProperties(const muse::XmlDomNode& propertiesNode, GPNote* n);
    void readNoteProperties(muse::XmlDomNode* propertiesNode, GPNote* n);
    void readBeatXProperties(const muse::XmlDomNode& propertiesNode, GPBeat* b);
    std::unique_ptr<GPNote::Bend> createBend(muse::XmlDomNode* propertyNode);
    void readHarmonic(muse::XmlDomNode* propertyNode, GPNote* note) const;

    std::vector<GPMasterTracks::Automation> readTempoMap(muse::XmlDomNode* currentNode);
    GPTrack::RSE readTrackRSE(muse::XmlDomNode* trackChildNode) const;
    GPMasterBar::KeySig readKeySig(muse::XmlDomNode* keyNode) const;
    bool readUseFlats(muse::XmlDomNode* keyNode) const;
    GPMasterBar::TimeSig readTimeSig(muse::XmlDomNode* timeNode) const;
    void readTrackProperties(muse::XmlDomNode* propertiesNode, GPTrack* track, bool ignoreTuningFlats) const;
    void readBeatProperties(const muse::XmlDomNode& propertiesNode, GPBeat* beat) const;
    void readDiagram(const muse::XmlDomNode& items, GPTrack* track) const;
    void readLyrics(const muse::XmlDomNode& items, GPTrack* track) const;
    std::vector<GPMasterBar::Fermata> readFermatas(muse::XmlDomNode* fermatasNode) const;
    std::vector<GPMasterBar::Direction> readRepeatsJumps(muse::XmlDomNode* repeatsJumpsNode) const;
    std::pair<muse::String, muse::String> readMasterBarSection(const muse::XmlDomNode& sectionNode) const;
    GPMasterBar::Repeat readRepeat(muse::XmlDomNode* repeatNode) const;
    std::vector<int> readEnding(muse::XmlDomNode* endNode) const;

    std::unordered_map<int, std::shared_ptr<GPNote> > _notes;
    std::unordered_map<int, std::shared_ptr<GPRhythm> > _rhythms;
    std::unordered_map<int, std::shared_ptr<GPBeat> > _beats;
    std::unordered_map<int, std::unique_ptr<GPVoice> > _voices;
    std::unordered_map<int, std::unique_ptr<GPBar> > _bars;

    std::unique_ptr<GPDomModel> _gpDom;
};
} // mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GP67DOMBUILDER_H
