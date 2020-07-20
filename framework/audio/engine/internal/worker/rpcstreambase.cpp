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
#include "rpcstreambase.h"

#include <list>
#include <vector>
#include <sstream>
#include <cstring>
#include <atomic>

#include <soloud.h>
#include "log.h"

using namespace mu::audio::engine;

static constexpr uint16_t READ_SAMPLES_COUNT = SAMPLE_GRANULARITY;

static constexpr uint16_t BUFFER_MIN_SIZE = 250;
static constexpr uint16_t BUFFER_MAX_SIZE = 500;

#define BUF_LOCK std::lock_guard<std::mutex> lock(mutex)

struct RpcStreamBase::Buffer {
    struct Block {
        float timestamp = 0.0;
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

struct RpcStreamBase::SLInstance : public SoLoud::AudioSourceInstance
{
    std::string m_name;
    RpcStreamBase* m_rpc = nullptr;
    Buffer::Block* m_requestedBlock = nullptr;
    RpcStreamBase::Buffer::Block m_silent;
    RpcStreamBase::Buffer m_buf;
    std::atomic<bool> m_seeking{false};
    std::atomic<bool> m_sourceHasEnded{false};

    SLInstance(RpcStreamBase* rs)
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
        m_rpc->channel()->unregisterStream(m_rpc->m_id);
        m_rpc->call(CallMethod::InstanceDestroy, {});

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
        seek_frame(mStreamPosition);
    }

    float* doGetBuffer(uint32_t /*samples*/, float time)
    {
//        if (xtz::audio::isTimestampEnded(time)) {
//            LOG_SLI() << "audio source is end, id: " << _rs->_id;
//            _sourceHasEnded.store(true);
//        } else {
//            _sourceHasEnded.store(false);
//        }

        IF_ASSERT_FAILED(!m_requestedBlock) {
            m_requestedBlock = nullptr;
        }

        m_requestedBlock = m_buf.takeFreeBlock();
        m_requestedBlock->timestamp = time;

        return &m_requestedBlock->buf[0];
    }

    void doOnRequestFinished()
    {
        if (!m_requestedBlock) {
            return;
        }

        IF_ASSERT_FAILED(m_requestedBlock) {
            return;
        }

        bool isEnd{ true };
        size_t writed_count{ 0 };
        {
            if (!m_seeking.load() /* && !xtz::audio::isTimestampEnded(_requestedBlock->timestamp)*/) {
                m_buf.pushWrited(m_requestedBlock);
                isEnd = false;
            }

            m_requestedBlock = nullptr;
            writed_count = m_buf.sizeWrited();
        }

        if (!isEnd && writed_count < BUFFER_MAX_SIZE) {
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

        auto GetBuffer = [this](uint32_t samples, float time) -> float* { return doGetBuffer(samples, time); };
        auto onRequestFinished = [this]() { doOnRequestFinished(); };
        m_rpc->channel()->registerStream(m_rpc->m_id, READ_SAMPLES_COUNT, mChannels, GetBuffer, onRequestFinished);
    }

    void readNextBlock()
    {
        m_rpc->channel()->requestAudio(m_rpc->m_id);
    }

    SoLoud::result seek_frame(double sec) override
    {
        m_seeking.store(true);
        m_sourceHasEnded.store(false);
        mStreamPosition = sec;
        m_rpc->call(CallMethod::InstanceSeekFrame, Args::make_arg1<float>(sec));
        m_buf.truncate();
        return SoLoud::SO_NO_ERROR;
    }

    unsigned int getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/) override
    {
        //LOGI() "[" << _name << "] buf.size_writed: " << _buf.size_writed();

        float timestamp = 0.0; //TIMESTAMP_SILENT;
        std::pair<Buffer::Block*, size_t /*writed count*/> p = m_buf.takeAndMoveWritedToFree();
        if (p.first) {
            std::memcpy(aBuffer, &p.first->buf[0], m_buf.blockSizeInBytes);
            timestamp = p.first->timestamp;
        } else {
            std::memcpy(aBuffer, &m_silent.buf[0], m_buf.blockSizeInBytes);
        }

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

struct RpcStreamBase::SL : public SoLoud::AudioSource
{

    RpcStreamBase* m_rpc = nullptr;

    SL(RpcStreamBase* rpc)
        : m_rpc(rpc) {}

    SoLoud::AudioSourceInstance* createInstance() override
    {
        return new RpcStreamBase::SLInstance(m_rpc);
    }
};

RpcStreamBase::RpcStreamBase(CallType type, const std::string& name)
    : m_name(name), m_type(type)
{
    m_id = channel()->newID();

    m_sl = new RpcStreamBase::SL(this);
    m_sl->mChannels = 2;

    call(CallMethod::Create, Args::make_arg1<std::string>(name));
}

RpcStreamBase::~RpcStreamBase()
{
    call(CallMethod::Destroy, {});
    channel()->unlisten(m_id);

    delete m_sl; // @NOTE: important to delete this object BEFORE the instance of the RpcStreamBase died
}

void RpcStreamBase::call(CallMethod method, const Args& args)
{
    CallID callid = callID(m_type, method);
    channel()->send(m_id, callid, args);
}

void RpcStreamBase::listen(const std::function<void(CallID callid, const Args& args)> &func)
{
    channel()->listen(m_id, [this, func](CallID callid, const Args& args) {
        if (callid == callID(m_type, CallMethod::InstaneOnSeek)) {
            std::lock_guard<std::mutex> lock(m_instanceMutex);
            if (m_instance) {
                m_instance->m_seeking.store(false);
            }
        } else {
            func(callid, args);
        }
    });
}

void RpcStreamBase::setSampleRate(float samplerate)
{
    m_sl->mBaseSamplerate = samplerate;
    call(CallMethod::SetSamplerate, Args::make_arg1<float>(samplerate));
}

void RpcStreamBase::truncate()
{
    std::lock_guard<std::mutex> lock(m_instanceMutex);
    if (m_instance) {
        m_instance->truncate();
    }
}

void RpcStreamBase::setLoopRegion(const LoopRegion& loop)
{
    call(CallMethod::SetLoopRegion, Args::make_arg1<LoopRegion>(loop));

    truncate();
}

SoLoud::AudioSource* RpcStreamBase::source()
{
    return m_sl;
}
