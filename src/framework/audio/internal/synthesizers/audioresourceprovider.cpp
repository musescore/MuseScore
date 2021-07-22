#include "audioresourceprovider.h"

#include "async/async.h"
#include "log.h"

#include "audioerrors.h"
#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"

using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::async;

Promise<AudioResourceIdList> AudioResourceProvider::resourceIdList(const AudioSourceType type) const
{
    return Promise<AudioResourceIdList>([this, type](Promise<AudioResourceIdList>::Resolve resolve,
                                                     Promise<AudioResourceIdList>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        auto search = m_resolvers.find(type);

        if (search != m_resolvers.end()) {
            resolve(search->second->resolve());
        } else {
            reject(static_cast<int>(Err::UknownSynthType), "Unable to find resources resolver for audio type");
        }
    }, AudioThread::ID);
}

void AudioResourceProvider::registerResolver(const AudioSourceType type, IResourceResolverPtr resolver)
{
    Async::call(this, [this, type, resolver]() {
        ONLY_AUDIO_WORKER_THREAD;

        IF_ASSERT_FAILED(resolver) {
            return;
        }

        resolver->refresh();
        m_resolvers.insert_or_assign(type, std::move(resolver));
    }, AudioThread::ID);
}

void AudioResourceProvider::refreshResolvers()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;

        for (const auto& pair : m_resolvers) {
            pair.second->refresh();
        }
    }, AudioThread::ID);
}
