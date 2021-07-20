#include "synthfactory.h"

#include "log.h"
#include "async/async.h"

#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"

using namespace mu::async;
using namespace mu::audio::synth;

void SynthFactory::init(const SynthUri& defaultUri)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(defaultUri.isValid()) {
        return;
    }

    m_defaultUri = defaultUri;
}

ISynthesizerPtr SynthFactory::createNew(const SynthUri& uri) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(uri.isValid()) {
        return nullptr;
    }

    auto search = m_creators.find(uri.type);

    if (search == m_creators.end()) {
        return nullptr;
    }

    return search->second->create(uri);
}

ISynthesizerPtr SynthFactory::createDefault() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_defaultUri.isValid()) {
        return nullptr;
    }

    return createNew(m_defaultUri);
}

void SynthFactory::registerCreator(const SynthType type, ISynthCreatorPtr creator)
{
    Async::call(this, [this, type, creator]() {
        ONLY_AUDIO_WORKER_THREAD;

        m_creators.insert_or_assign(type, std::move(creator));
    }, AudioThread::ID);
}
