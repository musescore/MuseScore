#include "synthfactory.h"

#include "log.h"
#include "async/async.h"

#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"

using namespace mu::async;
using namespace mu::audio::synth;

void SynthFactory::init(const AudioInputParams& defaultInputParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(defaultInputParams.isValid()) {
        return;
    }

    m_defaultInputParams = defaultInputParams;
}

ISynthesizerPtr SynthFactory::createNew(const AudioInputParams& params) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(params.isValid()) {
        return nullptr;
    }

    auto search = m_creators.find(params.type);

    if (search == m_creators.end()) {
        return nullptr;
    }

    return search->second->create(params);
}

ISynthesizerPtr SynthFactory::createDefault() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_defaultInputParams.isValid()) {
        return nullptr;
    }

    return createNew(m_defaultInputParams);
}

void SynthFactory::registerCreator(const AudioSourceType type, ISynthCreatorPtr creator)
{
    Async::call(this, [this, type, creator]() {
        ONLY_AUDIO_WORKER_THREAD;

        m_creators.insert_or_assign(type, std::move(creator));
    }, AudioThread::ID);
}
