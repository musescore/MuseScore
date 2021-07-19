#include "synthfactory.h"

#include "log.h"
#include "async/async.h"

#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"

using namespace mu::async;
using namespace mu::audio::synth;

void SynthFactory::init(const SynthType defaultType, ISynthCreatorPtr defaultCreator, SoundFontPath defaultSoundFontPath)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(defaultCreator || !defaultSoundFontPath.empty() || defaultType == SynthType::Undefined) {
        return;
    }

    m_creators.emplace(defaultType, std::move(defaultCreator));
    m_defaultSynthType = defaultType;
    m_defaultSoundFontPath = std::move(defaultSoundFontPath);
}

ISynthesizerPtr SynthFactory::createNew(const SynthType type, const SoundFontPath& sfPath) const
{
    ONLY_AUDIO_WORKER_THREAD;

    auto search = m_creators.find(type);

    if (search == m_creators.cend()) {
        return nullptr;
    }

    return search->second->create(sfPath);
}

ISynthesizerPtr SynthFactory::createDefault() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_defaultSynthType != SynthType::Undefined || !m_defaultSoundFontPath.empty()) {
        return nullptr;
    }

    return createNew(m_defaultSynthType, m_defaultSoundFontPath);
}

void SynthFactory::registerCreator(const SynthType type, ISynthCreatorPtr creator)
{
    Async::call(this, [this, type, creator]() {
        ONLY_AUDIO_WORKER_THREAD;

        m_creators.insert_or_assign(type, std::move(creator));
    }, AudioThread::ID);
}
