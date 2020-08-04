//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "rpcsourcebase.h"

#include <list>
#include <vector>
#include <sstream>
#include <cstring>
#include <atomic>

#include <soloud.h>
#include "log.h"

#include "audio/audiotypes.h"

using namespace mu::audio;
using namespace mu::audio::worker;

static constexpr uint16_t READ_SAMPLES_COUNT = SAMPLE_GRANULARITY;

static constexpr uint16_t BUFFER_MIN_SIZE = 250;
static constexpr uint16_t BUFFER_MAX_SIZE = 500;

#define BUF_LOCK std::lock_guard<std::mutex> lock(mutex)

struct RpcSourceBase::Buffer {
    struct Block {
        Context ctx;
        std::vector<float> buf;
    };

    mutable std::mutex mutex;
    uint32_t blockSizeInSamples{ 0 };
    uint32_t blockSizeInBytes{ 0 };
    std::list<Block*> freeBlocks;
    std::list<Block*> writedBlocks;

    Buffer() {}
    ~Buffer() {}

    void print_dump()
    {
        std::stringstream ss;
        ss << "free count: " << freeBlocks.size() << " ";
        ss << "writed count: " << writedBlocks.size() << " ";
        ss << "total count: " << (freeBlocks.size() + writedBlocks.size()) << "\n";

        LOGI() << ss.str();
    }

    void allocate()
    {
        BUF_LOCK;
        for (size_t i = 0; i < BUFFER_MAX_SIZE; ++i) {
            freeBlocks.push_back(new Block()); //! Block allocation but with empty buffers!!
        }
    }

    void deallocate()
    {
        BUF_LOCK;
        for (Block* b : freeBlocks) {
            delete b;
        }
        freeBlocks.clear();

        for (Block* b : writedBlocks) {
            delete b;
        }
        writedBlocks.clear();
    }

    // writed
    size_t sizeWrited() const
    {
        size_t s{ 0 };
        {
            BUF_LOCK;
            s = writedBlocks.size();
        }
        return s;
    }

    void pushWrited(Block* b)
    {
        BUF_LOCK;
        writedBlocks.push_back(b);
    }

    Block* takeWritedBlock()
    {
        Block* b = nullptr;
        {
            BUF_LOCK;
            b = takeWritedBlockUnsafe();
        }
        return b;
    }

    Block* takeWritedBlockUnsafe()
    {
        Block* b = nullptr;
        if (!writedBlocks.empty()) {
            b = *writedBlocks.begin();
            writedBlocks.erase(writedBlocks.begin());
        }
        return b;
    }

    std::pair<Block*, size_t /*writed count*/> takeAndMoveWritedToFree()
    {
        std::pair<Block*, size_t /*write count*/> p;
        {
            BUF_LOCK;
            p.first = takeWritedBlockUnsafe();
            if (p.first) {
                freeBlocks.push_back(p.first);
            }
            p.second = writedBlocks.size();
        }
        return p;
    }

    // free
    Block* takeFreeBlock()
    {
        Block* b = nullptr;
        {
            BUF_LOCK;
            if (freeBlocks.empty()) {
                b = new Block();
            } else {
                b = *freeBlocks.begin();
                freeBlocks.erase(freeBlocks.begin());
            }

            if (b->buf.empty()) {
                b->buf.resize(blockSizeInSamples);
            }
        }
        return b;
    }

    void pushFree(Block* b)
    {
        BUF_LOCK;
        freeBlocks.push_back(b);
    }

    void truncate()
    {
        BUF_LOCK;
        for (Block* b : writedBlocks) {
            freeBlocks.push_back(b);
        }
        writedBlocks.clear();
    }
};

struct RpcSourceBase::SLInstance : public SoLoud::AudioSourceInstance
{
    std::string m_name;
    RpcSourceBase* m_rpc = nullptr;
    Buffer::Block* m_requestedBlock = nullptr;
    RpcSourceBase::Buffer::Block m_silent;
    RpcSourceBase::Buffer m_buf;
    double m_positionFromCtx = 0.0;
    std::atomic<bool> m_seeking{ false };
    std::atomic<bool> m_sourceHasEnded{ false };

    SLInstance(RpcSourceBase* rs)
        : m_rpc(rs)
    {
        m_name = m_rpc->m_name;

        {
            std::lock_guard<std::mutex> lock(m_rpc->m_instanceMutex);
            m_rpc->m_instance = this;
        }
        m_rpc->call(CallMethod::InstanceCreate, {});

        m_buf.allocate();
    }

    ~SLInstance() override
    {
        m_rpc->channel()->unregisterStream(m_rpc->m_streamID);
        m_rpc->call(CallMethod::InstanceDestroy, {});

        Context ctx;
        ctx.set<bool>(CtxKey::InstanceDestroyed, true);
        m_rpc->audioEngine()->swapPlayContext(handle, ctx);

        {
            std::lock_guard<std::mutex> lock(m_rpc->m_instanceMutex);
            m_rpc->m_instance = nullptr;
        }

        m_buf.deallocate();
        delete m_requestedBlock;
        m_requestedBlock = nullptr;
    }

    void truncate()
    {
        if (m_seeking.load() || m_buf.sizeWrited() == 0) {
            //! NOTE There is no point in doing truncate, or it is already being done, or the written buffer is empty
            return;
        }

        //! NOTE seek_frame truncate the buffer itself
        seekFrame(m_positionFromCtx);
    }

    float* doGetBuffer(uint32_t /*samples*/, Context& ctx)
    {
        if (ctx.hasVal(CtxKey::HasEnded)) {
            m_sourceHasEnded.store(true);
        } else {
            m_sourceHasEnded.store(false);
        }

        IF_ASSERT_FAILED(!m_requestedBlock) {
            m_requestedBlock = nullptr;
        }

        m_requestedBlock = m_buf.takeFreeBlock();
        m_requestedBlock->ctx.swap(ctx);

        return &m_requestedBlock->buf[0];
    }

    void doOnRequestFinished()
    {
        //LOGI() << "doOnRequestFinished";
        if (!m_requestedBlock) {
            return;
        }

        bool isEnd = true;
        size_t writedCount = 0;
        {
            if (!m_seeking.load() && !m_sourceHasEnded.load()) {
                m_buf.pushWrited(m_requestedBlock);
                isEnd = false;
            }

            m_requestedBlock = nullptr;
            writedCount = m_buf.sizeWrited();
        }

        if (!isEnd && writedCount < BUFFER_MAX_SIZE) {
            readNextBlock();
        }
    }

    void init(SoLoud::AudioSource& aSource, int aPlayIndex) override
    {
        SoLoud::AudioSourceInstance::init(aSource, aPlayIndex);

        Args args;
        args.setArg<float>(0, mSamplerate);
        args.setArg<unsigned int>(1, mChannels);
        args.setArg<double>(2, mStreamTime);
        args.setArg<double>(3, mStreamPosition);
        m_rpc->call(CallMethod::InstanceInit, args);

        m_silent.buf.resize(READ_SAMPLES_COUNT * mChannels);
        for (size_t i = 0; i < m_silent.buf.size(); ++i) {
            m_silent.buf[i] = 0;
        }

        m_buf.blockSizeInSamples = READ_SAMPLES_COUNT * mChannels;
        m_buf.blockSizeInBytes = m_buf.blockSizeInSamples * sizeof(float);

        auto GetBuffer = [this](uint32_t samples, Context& ctx) -> float* { return doGetBuffer(samples, ctx); };
        auto onRequestFinished = [this]() { doOnRequestFinished(); };
        m_rpc->channel()->registerStream(m_rpc->m_streamID, READ_SAMPLES_COUNT, mChannels, GetBuffer,
                                         onRequestFinished);
    }

    void readNextBlock()
    {
        m_rpc->channel()->requestAudio(m_rpc->m_streamID);
    }

    SoLoud::result seekFrame(double sec) override
    {
        m_seeking.store(true);
        m_sourceHasEnded.store(false);
        mStreamPosition = sec;
        m_positionFromCtx = sec;
        m_rpc->call(CallMethod::InstanceSeekFrame, Args::make_arg1<float>(sec));
        m_buf.truncate();
        return SoLoud::SO_NO_ERROR;
    }

    unsigned int getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/) override
    {
        std::pair<Buffer::Block*, size_t /*writed count*/> p = m_buf.takeAndMoveWritedToFree();
        if (p.first) {
            std::memcpy(aBuffer, &p.first->buf[0], m_buf.blockSizeInBytes);
            m_positionFromCtx = p.first->ctx.get<double>(CtxKey::Position, m_positionFromCtx);
            m_rpc->onGetAudio(p.first->ctx);
            m_rpc->audioEngine()->swapPlayContext(handle, p.first->ctx);
        } else {
            std::memcpy(aBuffer, &m_silent.buf[0], m_buf.blockSizeInBytes);

            Context ctx;
            ctx.set<bool>(CtxKey::Silent, true);
            m_rpc->onGetAudio(ctx);
            m_rpc->audioEngine()->swapPlayContext(handle, ctx);
        }

        mStreamPosition = m_positionFromCtx;

        bool isNeedReadBlock =  p.second < BUFFER_MIN_SIZE && !m_sourceHasEnded.load();
        if (isNeedReadBlock) {
            readNextBlock();
        }

        return aSamplesToRead;
    }

    bool hasEnded() override
    {
        return m_sourceHasEnded.load() && m_buf.sizeWrited() == 0;
    }
};

struct RpcSourceBase::SL : public SoLoud::AudioSource
{
    RpcSourceBase* m_rpc = nullptr;

    SL(RpcSourceBase* rpc)
        : m_rpc(rpc) {}

    SoLoud::AudioSourceInstance* createInstance() override
    {
        return new RpcSourceBase::SLInstance(m_rpc);
    }
};

RpcSourceBase::RpcSourceBase(CallType type, const std::string& name)
    : m_name(name), m_type(type)
{
    m_streamID = channel()->newID();

    m_sl = new RpcSourceBase::SL(this);
    m_sl->mChannels = 2;

    call(CallMethod::Create, Args::make_arg1<std::string>(name));
}

RpcSourceBase::~RpcSourceBase()
{
    call(CallMethod::Destroy, {});
    channel()->unlisten(m_streamID);

    delete m_sl; //! NOTE important to delete this object BEFORE the instance of the RpcStreamBase died
}

void RpcSourceBase::call(CallMethod method, const Args& args)
{
    CallID callid = callID(m_type, method);
    channel()->send(m_streamID, callid, args);
}

void RpcSourceBase::listen(const std::function<void(CallMethod callid, const Args& args)>& func)
{
    channel()->listen(m_streamID, [this, func](CallID callid, const Args& args) {
        if (callType(callid) != m_type) {
            return;
        }

        CallMethod method = callMethod(callid);
        if (method == CallMethod::InstaneOnSeek) {
            std::lock_guard<std::mutex> lock(m_instanceMutex);
            if (m_instance) {
                m_instance->m_seeking.store(false);
            }
        } else {
            func(method, args);
        }
    });
}

void RpcSourceBase::setSampleRate(float samplerate)
{
    m_sl->mBaseSamplerate = samplerate;
    call(CallMethod::SetSamplerate, Args::make_arg1<float>(samplerate));
}

void RpcSourceBase::truncate()
{
    std::lock_guard<std::mutex> lock(m_instanceMutex);
    if (m_instance) {
        m_instance->truncate();
    }
}

void RpcSourceBase::setLoopRegion(const LoopRegion& loop)
{
    call(CallMethod::SetLoopRegion, Args::make_arg1<LoopRegion>(loop));

    truncate();
}

SoLoud::AudioSource* RpcSourceBase::source()
{
    return m_sl;
}

void RpcSourceBase::onGetAudio(const Context&)
{
}
