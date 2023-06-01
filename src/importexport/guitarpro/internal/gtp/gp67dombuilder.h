#ifndef MU_IMPORTEXPORT_GP67DOMBUILDER_H
#define MU_IMPORTEXPORT_GP67DOMBUILDER_H

#include <memory>

#include "igpdombuilder.h"
#include "gpdommodel.h"
#include "inoteproperty.h"
#include "gptrack.h"

namespace mu::iex::guitarpro {
class GP67DomBuilder : public IGPDomBuilder
{
public:
    GP67DomBuilder();

    void buildGPDomModel(XmlDomElement* domElem) override;
    std::unique_ptr<GPDomModel> getGPDomModel() override;

protected:

    void buildGPScore(XmlDomNode* scoreNode);
    void buildGPMasterTracks(XmlDomNode* masterTrack);
    void buildGPAudioTracks(XmlDomNode* audioTrack);
    void buildGPTracks(XmlDomNode* tracksNode);
    void buildGPMasterBars(XmlDomNode* masterBars);
    void buildGPBars(XmlDomNode* bars);
    void buildGPVoices(XmlDomNode* voicesNode);
    void buildGPBeats(XmlDomNode* beats);
    void buildGPNotes(XmlDomNode* notesNode);
    void buildGPRhythms(XmlDomNode* rhythms);

    void breakLyricsOnBeatsIfNeed();
    bool isLyricsOnBeats() const;

    virtual std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(XmlDomNode* trackNode) = 0;

    std::unique_ptr<GPMasterTracks> createGPMasterTrack(XmlDomNode* metadata);
    std::unique_ptr<GPMasterBar> createGPMasterBar(XmlDomNode* masterBarNode);
    std::pair<int, std::unique_ptr<GPBar> > createGPBar(XmlDomNode* barNode);
    std::pair<int, std::unique_ptr<GPVoice> > createGPVoice(XmlDomNode* voiceNode);
    std::pair<int, std::shared_ptr<GPBeat> > createGPBeat(XmlDomNode* beatNode);
    std::pair<int, std::shared_ptr<GPNote> > createGPNote(XmlDomNode* noteNode);
    std::pair<int, std::shared_ptr<GPRhythm> > createGPRhythm(XmlDomNode* rhythmNode);

    void readNoteXProperties(const XmlDomNode& propertiesNode, GPNote* n);
    void readNoteProperties(XmlDomNode* propertiesNode, GPNote* n);
    void readBeatXProperties(const XmlDomNode& propertiesNode, GPBeat* b);
    std::unique_ptr<GPNote::Bend> createBend(XmlDomNode* propertyNode);
    void readHarmonic(XmlDomNode* propertyNode, GPNote* note) const;

    std::vector<GPMasterTracks::Automation> readTempoMap(XmlDomNode* currentNode);
    GPTrack::RSE readTrackRSE(XmlDomNode* trackChildNode) const;
    GPMasterBar::KeySig readKeySig(XmlDomNode* keyNode) const;
    bool readUseFlats(XmlDomNode* keyNode) const;
    GPMasterBar::TimeSig readTimeSig(XmlDomNode* timeNode) const;
    void readTrackProperties(XmlDomNode* propertiesNode, GPTrack* track) const;
    void readBeatProperties(const XmlDomNode& propertiesNode, GPBeat* beat) const;
    void readDiagram(const XmlDomNode& items, GPTrack* track) const;
    void readLyrics(const XmlDomNode& items, GPTrack* track) const;
    std::vector<GPMasterBar::Fermata> readFermatas(XmlDomNode* fermatasNode) const;
    std::vector<GPMasterBar::Direction> readRepeatsJumps(XmlDomNode* repeatsJumpsNode) const;
    std::pair<String, String> readMasterBarSection(const XmlDomNode& sectionNode) const;
    GPMasterBar::Repeat readRepeat(XmlDomNode* repeatNode) const;
    std::vector<int> readEnding(XmlDomNode* endNode) const;

    std::unordered_map<int, std::shared_ptr<GPNote> > _notes;
    std::unordered_map<int, std::shared_ptr<GPRhythm> > _rhythms;
    std::unordered_map<int, std::shared_ptr<GPBeat> > _beats;
    std::unordered_map<int, std::unique_ptr<GPVoice> > _voices;
    std::unordered_map<int, std::unique_ptr<GPBar> > _bars;

    std::unique_ptr<GPDomModel> _gpDom;
};
} // mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GP67DOMBUILDER_H
