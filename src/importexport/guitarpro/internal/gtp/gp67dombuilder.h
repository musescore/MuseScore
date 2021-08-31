#ifndef GPDOMBUILDER_H
#define GPDOMBUILDER_H

#include <memory>
#include <QDomNode>

#include "igpdombuilder.h"
#include "gpdommodel.h"
#include "inoteproperty.h"

namespace Ms {
class GP67DomBuilder : public IGPDomBuilder
{
public:
    GP67DomBuilder();

    void buildGPDomModel(QDomElement* qdomElem) override;
    std::unique_ptr<GPDomModel> getGPDomModel() override;

protected:

    void buildGPScore(QDomNode* scoreNode);
    void buildGPMasterTracks(QDomNode* masterTrack);
    void buildGPAudioTracks(QDomNode* audioTrack);
    void buildGPTracks(QDomNode* tracksNode);
    void buildGPMasterBars(QDomNode* masterBars);
    void buildGPBars(QDomNode* bars);
    void buildGPVoices(QDomNode* voicesNode);
    void buildGPBeats(QDomNode* beats);
    void buildGPNotes(QDomNode* notesNode);
    void buildGPRhythms(QDomNode* rhythms);

    void breakLyricsOnBeatsIfNeed();
    bool isLyricsOnBeats() const;

    virtual std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(QDomNode* trackNode) = 0;

    std::unique_ptr<GPMasterTracks> createGPMasterTrack(QDomNode* metadata);
    std::unique_ptr<GPAudioTrack> createGPAudioTrack(QDomNode* metadata);
    std::unique_ptr<GPMasterBar> createGPMasterBar(QDomNode* masterBarNode);
    std::pair<int, std::unique_ptr<GPBar> > createGPBar(QDomNode* barNode);
    std::pair<int, std::unique_ptr<GPVoice> > createGPVoice(QDomNode* voiceNode);
    std::pair<int, std::shared_ptr<GPBeat> > createGPBeat(QDomNode* beatNode);
    std::pair<int, std::shared_ptr<GPNote> > createGPNote(QDomNode* noteNode);
    std::pair<int, std::shared_ptr<GPRhytm> > createGPRhythm(QDomNode* rhythmNode);

    void readNoteXProperties(const QDomNode& propertiesNode, GPNote* n);
    void readNoteProperties(QDomNode* propertiesNode, GPNote* n);
    void readBeatXProperties(const QDomNode& propertiesNode, GPBeat* b);
    std::unique_ptr<GPNote::Bend> createBend(QDomNode* propertyNode);
    void readHarmonic(QDomNode* propertyNode, GPNote* note) const;

    std::vector<GPMasterTracks::Automation> readTempoMap(QDomNode* currentNode);
    GPTrack::RSE readTrackRSE(QDomNode* trackChildNode) const;
    GPMasterBar::KeySig readKeySig(QDomNode* keyNode) const;
    GPMasterBar::TimeSig readTimeSig(QDomNode* timeNode) const;
    void readTrackProperties(QDomNode* propertiesNode, GPTrack* track) const;
    void readBeatProperties(const QDomNode& propertiesNode, GPBeat* beat) const;
    void readDiagramm(const QDomNode& items, GPTrack* track) const;
    void readLyrics(const QDomNode& items, GPTrack* track) const;
    std::vector<GPMasterBar::Fermata> readFermatas(QDomNode* fermatasNode) const;
    std::pair<QString, QString> readMasterBarSection(const QDomNode& sectionNode) const;
    GPMasterBar::Repeat readRepeat(QDomNode* repeatNode) const;
    std::vector<int> readEnding(QDomNode* endNode) const;

    std::unordered_map<int, std::shared_ptr<GPNote> > _notes;
    std::unordered_map<int, std::shared_ptr<GPRhytm> > _rhytms;
    std::unordered_map<int, std::shared_ptr<GPBeat> > _beats;
    std::unordered_map<int, std::unique_ptr<GPVoice> > _voices;
    std::unordered_map<int, std::unique_ptr<GPBar> > _bars;

    std::unique_ptr<GPDomModel> _gpDom;
};
} // end Ms namepsace
#endif // GPDOMBUILDER_H
