#ifndef VSTAUDIOCLIENT_H
#define VSTAUDIOCLIENT_H

#include "vsttypes.h"

namespace mu::vst {
class VstAudioClient
{
public:
    VstAudioClient() = default;
    ~VstAudioClient();

    void init(PluginComponentPtr component);

    bool handleEvent(const midi::Event& e);

    void process(float* output, unsigned int samples);

    void setBlockSize(unsigned int samples);
    void setSampleRate(unsigned int sampleRate);

private:
    struct SamplesInfo {
        unsigned int sampleRate = 0;
        unsigned int samplesPerBlock = 0;

        bool isValid()
        {
            return sampleRate > 0 && samplesPerBlock > 0;
        }
    };

    IAudioProcessorPtr pluginProcessor() const;
    void setUpProcessData();
    void updateProcessSetup();
    void fillOutputBuffer(unsigned int samples, float* output);

    bool convertMidiToVst(const mu::midi::Event& in, VstEvent& out) const;

    PluginComponentPtr m_pluginComponent = nullptr;

    SamplesInfo m_samplesInfo;

    VstEventList m_eventList;
    VstProcessData m_processData;
    VstProcessContext m_processContext;
};
}

#endif // VSTAUDIOCLIENT_H
