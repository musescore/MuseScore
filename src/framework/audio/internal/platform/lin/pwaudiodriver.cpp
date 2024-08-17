/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Pipewire Audio Driver implementation.
 *
 * There are two main objects involved:
 *  - the registry, which is responsible to enumerate devices
 *  - the stream, which is responsible to fetch audio data and send it to the device
 *
 * There are also two additional threads created by this module:
 *  - "pw-driver-loop" thread, which manages the driver event loop (registry callbacks, state changes, etc)
 *  - "pw-data-loop" thread, created by pipewire internals, which deals with low-latency data flow (process function)
 */

#include "pwaudiodriver.h"

#include "containers.h"
#include "log.h"
#include "runtime.h"
#include "translation.h"

#include <pipewire/pipewire.h>
#include <spa/debug/dict.h>
#include <spa/debug/pod.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/latency-utils.h>
#include <spa/support/log.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#define DEBUG_AUDIO
#ifdef DEBUG_AUDIO
#define LOG_AUDIO LOGD
#else
#define LOG_AUDIO LOGN
#endif

using namespace muse;
using namespace muse::audio;

namespace {
#define DEBUG_PW
#ifdef DEBUG_PW
static constexpr enum spa_log_level PW_LOG_LEVEL = SPA_LOG_LEVEL_TRACE;
#else
static constexpr enum spa_log_level PW_LOG_LEVEL = SPA_LOG_LEVEL_INFO;
#endif

static constexpr char PW_THREAD_NAME[] = "pw-driver-loop";
static constexpr char PW_DEFAULT_DEVICE[] = "default"; // symbolic name only, could be anything

class PwLoopLock
{
    struct pw_thread_loop* loop;
public:
    explicit PwLoopLock(pw_thread_loop* loop)
        : loop(loop)
    {
        pw_thread_loop_lock(loop);
    }

    ~PwLoopLock()
    {
        pw_thread_loop_unlock(loop);
    }

    PwLoopLock(const PwLoopLock&) = delete;
    PwLoopLock& operator=(const PwLoopLock&) = delete;
};
}

namespace muse::audio {
// PwRegistry enumerates audio devices.
//
// It uses the same event loop and connection (pw_core) as the data stream.
// It also provides a notification when the list of devices changes.
//
// Pipewire will call the register function several times during intialization phase
// and then issue a "done" event when roundtrip is finished.
class PwRegistry
{
public:
    PwRegistry(pw_thread_loop* loop, pw_core* core);
    ~PwRegistry();

    PwRegistry() = delete;
    PwRegistry(const PwRegistry&) = delete;
    PwRegistry& operator=(const PwRegistry&) = delete;

    // init() method must be done at the same time the thread loop is started.
    void init();

    AudioDeviceList getDevices() const;
    async::Notification availableOutputDevicesChanged() const
    {
        return m_devicesChanged;
    }

private:

    // C-style callbacks and their corresponding C++ methods
    static void c_register_global(void* data, uint32_t id, uint32_t /* perms*/, const char* type, uint32_t /*version*/,
                                  const struct spa_dict* props);
    void registerGlobal(uint32_t id, const char* type, const struct spa_dict* props);

    static void c_unregister_global(void* data, uint32_t id);
    void unregisterGlobal(uint32_t id);

    static void c_roundtrip_done(void* data, uint32_t id, int seq);
    void roundtripDone(uint32_t id, int seq);

    // methods starting with '_' must be called with mutex held
    bool _initDone() const
    {
        return m_pending != 0 || m_roundtripped;
    }

    void _devicesDirty();

    struct SinkNode {
        uint32_t id;
        std::string name;
        std::string nick;
        std::string desc;
        uint32_t deviceId;
    };

    struct Device {
        uint32_t id;
        std::string name;
        std::string nick;
        std::string desc;
    };

    pw_thread_loop* m_loop = nullptr;
    pw_core* m_core = nullptr;
    pw_registry* m_registry = nullptr;

    struct spa_hook m_registryHook = {};  // binds m_registryEvents with this
    struct pw_registry_events m_registryEvents = {};
    struct spa_hook m_roundtripHook = {}; // binds m_roundtripEvents with this
    struct pw_core_events m_roundtripEvents = {};

    std::vector<SinkNode> m_pwNodes;
    std::vector<Device> m_pwDevices;

    async::Notification m_devicesChanged;

    mutable AudioDeviceList m_devices;
    mutable std::mutex m_mutex;
    mutable std::condition_variable m_cond;
    mutable bool m_devicesDirty = true;
    int m_pending = 0;
    bool m_roundtripped = false;
};
}

// Called from main thread
PwRegistry::PwRegistry(pw_thread_loop* loop, pw_core* core)
    : m_loop{loop}, m_core{core}
    , m_registry{pw_core_get_registry(core, PW_VERSION_REGISTRY, 0)}
    , m_registryEvents{PW_VERSION_REGISTRY_EVENTS, c_register_global, c_unregister_global}
    , m_roundtripEvents{}
    , m_devicesDirty{true}
    , m_pending{0}
    , m_roundtripped{false}
{
    m_roundtripEvents.version = PW_VERSION_CORE_EVENTS;
    m_roundtripEvents.done = c_roundtrip_done;

    PwLoopLock lk { m_loop };

    pw_core_add_listener(core, &m_roundtripHook, &m_roundtripEvents, this);
    pw_registry_add_listener(m_registry, &m_registryHook, &m_registryEvents, this);
}

// Called from main thread
PwRegistry::~PwRegistry()
{
    PwLoopLock lk { m_loop };

    pw_proxy_destroy(reinterpret_cast<pw_proxy*>(m_registry));
}

// Called from main thread
void PwRegistry::init()
{
    // core sync method triggers the "done" event after roundtrip
    m_pending = pw_core_sync(m_core, PW_ID_CORE, 0);
}

// Called from main thread
AudioDeviceList PwRegistry::getDevices() const
{
    std::unique_lock<std::mutex> lk(m_mutex);

    if (!m_devicesDirty) {
        LOGD() << "devices up-to-date";
        return m_devices;
    }

    LOGD() << "reconstructing device list";

    m_devices.clear();
    m_devices.push_back({ PW_DEFAULT_DEVICE, muse::trc("audio", "System default") });

    if (!_initDone()) {
        // waiting for m_cond before init would deadlock !! (if not for timeout)
        // because this is the main thread, and the thread loop is not yet started
        return m_devices;
    }

    if (!m_roundtripped) {
        LOGD() << "waiting for devices...";
    }
    m_cond.wait_for(lk, std::chrono::milliseconds(500), [this] { return m_roundtripped; });

    LOGD() << "found " << m_pwNodes.size() + 1 << " devices";
    LOGD() << m_devices[0].id << ": " << m_devices[0].name;

    for (const auto& node : m_pwNodes) {
        LOGD() << "seeking device " << node.deviceId << " for node " << node.name;

        const auto dev = std::find_if(
            m_pwDevices.begin(), m_pwDevices.end(),
            [node](const Device& device) { return device.id == node.deviceId; });

        const auto devFound = dev != m_pwDevices.end();

        if (devFound) {
            LOGD() << "node with device";
        } else {
            LOGD() << "node without device";
        }

        // pipewire identifies node by the "node.name" property
        // (e.g. "alsa_output.pci-0000_04_00.6.analog-stereo")
        // or by the "object.serial" (which we are not using)
        AudioDeviceID id = node.name;

        LOGD() << "id: " << id;

        // priority order for human readable name:
        // - device nickname        (e.g. "HD-Audio Generic")
        // - node nickname          (e.g. "Generic Analog")
        // - device description     (e.g. "Family 17h/19h HD Audio Controller")
        // - node description       (e.g. "Family 17h/19h HD Audio Controller Analog Stereo")
        // - node name              (e.g. "alsa_output.pci-0000_04_00.6.analog-stereo")
        std::string name;
        if (devFound && !dev->nick.empty()) {
            name = dev->nick;
            LOGD() << "device nick: " << name;
        } else if (!node.nick.empty()) {
            name = node.nick;
            LOGD() << "node nick: " << name;
        } else if (devFound && !dev->desc.empty()) {
            name = dev->desc;
            LOGD() << "device desc: " << name;
        } else if (!node.desc.empty()) {
            name = node.desc;
            LOGD() << "node desc: " << name;
        } else {
            name = node.name;
            LOGD() << "node name: " << name;
        }

        LOGD() << id << ": " << name;
        m_devices.push_back({ id, name });
    }

    m_devicesDirty = false;

    return m_devices;
}

// Called from "pw-driver-loop" thread
void PwRegistry::c_roundtrip_done(void* data, uint32_t id, int seq)
{
    auto registry = static_cast<PwRegistry*>(data);
    registry->roundtripDone(id, seq);
}

// Called from "pw-driver-loop" thread
void PwRegistry::roundtripDone(uint32_t id, int seq)
{
    if (id == PW_ID_CORE && seq == m_pending) {
        std::scoped_lock lk { m_mutex };

        m_roundtripped = true;
        m_pending = 0;
        m_cond.notify_all();
        m_devicesChanged.notify();
    }
}

// Called from "pw-driver-loop" thread
void PwRegistry::c_register_global(void* data, uint32_t id, uint32_t /* permissions*/, const char* type, uint32_t /*version*/,
                                   const struct spa_dict* props)
{
    auto registry = static_cast<PwRegistry*>(data);
    registry->registerGlobal(id, type, props);
}

// Called from "pw-driver-loop" thread
void PwRegistry::registerGlobal(uint32_t id, const char* type, const struct spa_dict* props)
{
    // For playback, we are interested in Audio/Sink only.
    // We fetch also Audio/Device, because they arguably have neater nick & description than Audio/Sink.
    // See `getDevices` for details.

    if (spa_streq(type, PW_TYPE_INTERFACE_Device)) {
        auto mediaClass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
        if (spa_streq(mediaClass, "Audio/Device")) {
            auto name = spa_dict_lookup(props, PW_KEY_DEVICE_NAME);
            auto nick = spa_dict_lookup(props, PW_KEY_DEVICE_NICK);
            auto desc = spa_dict_lookup(props, PW_KEY_DEVICE_DESCRIPTION);

            LOGD() << "Registering device: " << name << " (" << id << ")";

            std::scoped_lock<std::mutex> lock(m_mutex);
            m_pwDevices.push_back(Device {
                id,
                name,
                nick ? nick : "",
                desc ? desc : ""
            });
        }
    } else if (spa_streq(type, PW_TYPE_INTERFACE_Node)) {
        auto mediaClass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
        if (spa_streq(mediaClass, "Audio/Sink")) {
            auto name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
            auto nick = spa_dict_lookup(props, PW_KEY_NODE_NICK);
            auto desc = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
            auto deviceIdStr = spa_dict_lookup(props, PW_KEY_DEVICE_ID);
            auto deviceId = static_cast<uint32_t>(deviceIdStr ? std::strtoul(deviceIdStr, nullptr, 10) : 0);

            LOGD() << "Registering sink node: " << name << " (device " << deviceId << ")";

            std::scoped_lock<std::mutex> lock(m_mutex);
            m_pwNodes.push_back(SinkNode {
                id,
                name,
                nick ? nick : "",
                desc ? desc : "",
                deviceId
            });
            _devicesDirty();
        }
    }
}

// Called from "pw-driver-loop" thread
void PwRegistry::c_unregister_global(void* data, uint32_t id)
{
    auto registry = static_cast<PwRegistry*>(data);
    registry->unregisterGlobal(id);
}

// Called from "pw-driver-loop" thread
void PwRegistry::unregisterGlobal(uint32_t id)
{
    std::scoped_lock<std::mutex> lock(m_mutex);

    const auto numN = m_pwNodes.size();

    muse::remove_if(m_pwDevices, [id](const Device& device) {
        return device.id == id;
    });
    muse::remove_if(m_pwNodes, [id](const SinkNode& node) {
        return node.id == id;
    });

    if (numN != m_pwNodes.size()) {
        _devicesDirty();
    }
}

// Called from "pw-driver-loop" thread
// Must be called with mutex locked
void PwRegistry::_devicesDirty()
{
    m_devicesDirty = true;
    if (m_roundtripped) {
        // if already roundtripped, it is probably a device hotplug while application is running.
        m_devicesChanged.notify();
    }
}

namespace muse::audio {
class PwStream
{
public:
    PwStream(pw_thread_loop* loop, pw_core* core, const IAudioDriver::Spec& spec, const std::string& deviceId);
    ~PwStream();

    IAudioDriver::Spec spec() const
    {
        return m_spec;
    }

    void suspend();
    void resume();

private:

    // C-style callbacks and their corresponding C++ methods
    static void c_process(void* userdata);
    void process();

    pw_thread_loop* m_loop = nullptr;
    pw_core* m_core = nullptr;

    spa_hook m_hook;  // binds m_events with this
    pw_stream_events m_events;
    struct pw_stream* m_stream = nullptr;

    IAudioDriver::Spec m_spec;
    size_t m_stride;
};
}

// Called from main thread
PwStream::PwStream(pw_thread_loop* loop, pw_core* core, const IAudioDriver::Spec& spec, const std::string& deviceId)
    : m_loop{loop}, m_core{core}
    , m_events{}
    , m_spec{spec}
{
    m_events.version = PW_VERSION_STREAM_EVENTS;
    m_events.process = c_process;

    auto props = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Playback",
        PW_KEY_MEDIA_ROLE, "Production",
        nullptr);

    if (m_spec.samples) {
        // Request for a specific number of samples.
        // This is done through the "node.latency" property.
        // Note: user system configuration can override this. e.g.:
        //  - PIPEWIRE_QUANTUM environment variable.
        //  - "clock.force-quantum" setting
        std::ostringstream lat;
        lat << m_spec.samples << "/" << m_spec.sampleRate;
        pw_properties_set(props, PW_KEY_NODE_LATENCY, lat.str().c_str());
    }

    if (deviceId != PW_DEFAULT_DEVICE) {
        // Request for a specific output device.
        // This is done through the "target.object" property.
        pw_properties_set(props, PW_KEY_TARGET_OBJECT, deviceId.c_str());
    }

    spa_audio_info_raw formatInfo {};
    formatInfo.rate = m_spec.sampleRate;
    formatInfo.channels = m_spec.channels;
    switch (m_spec.format) {
    case IAudioDriver::Format::AudioF32:
        formatInfo.format = SPA_AUDIO_FORMAT_F32;
        m_stride = m_spec.channels * 4;
        break;
    case IAudioDriver::Format::AudioS16:
        formatInfo.format = SPA_AUDIO_FORMAT_S16;
        m_stride = m_spec.channels * 2;
        break;
    default:
        LOGW() << "Unknow format, falling back to F32";
        formatInfo.format = SPA_AUDIO_FORMAT_F32;
        m_spec.format = IAudioDriver::Format::AudioF32;
        m_stride = m_spec.channels * 4;
        break;
    }

    char buf[1024];
    auto builder = SPA_POD_BUILDER_INIT(buf, sizeof(buf));
    const spa_pod* param = spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &formatInfo);

    PwLoopLock lk { m_loop };

    pw_log_debug("pw_stream_new with props and params");
    spa_debug_dict(1, &props->dict);
    spa_debug_pod(1, NULL, param);

    m_stream = pw_stream_new(m_core, "MuseScore", props);

    pw_stream_add_listener(m_stream, &m_hook, &m_events, this);

    pw_stream_connect(m_stream, PW_DIRECTION_OUTPUT, PW_ID_ANY,
                      static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT
                                                   | PW_STREAM_FLAG_MAP_BUFFERS
                                                   | PW_STREAM_FLAG_RT_PROCESS),
                      &param, 1);
}

// Called from main thread
PwStream::~PwStream()
{
    pw_stream_flush(m_stream, false);

    PwLoopLock lk { m_loop };
    pw_stream_disconnect(m_stream);
    pw_stream_destroy(m_stream);
}

// Called from main thread
void PwStream::suspend()
{
    pw_stream_set_active(m_stream, false);
}

// Called from main thread
void PwStream::resume()
{
    pw_stream_set_active(m_stream, true);
}

// If PW_STREAM_FLAG_RT_PROCESS is set, this is called from "pw-data-loop" thread,
// otherwise it is called from "pw-driver-loop" thread
void PwStream::c_process(void* userdata)
{
    PwStream* stream = static_cast<PwStream*>(userdata);
    stream->process();
}

// If PW_STREAM_FLAG_RT_PROCESS is set, this is called from "pw-data-loop" thread,
// otherwise it is called from "pw-driver-loop" thread
void PwStream::process()
{
    auto b = pw_stream_dequeue_buffer(m_stream);
    if (!b || !b->buffer) {
        LOG_AUDIO() << "out of buffers";
        return;
    }

    auto buf = b->buffer;
    auto dst = static_cast<uint8_t*>(buf->datas[0].data);
    if (!dst) {
        return;
    }

    const auto maxFrames = buf->datas[0].maxsize / m_stride;
    const auto numFrames = b->requested ? SPA_MIN(b->requested, maxFrames) : maxFrames;
    const auto len = numFrames * m_stride;

    m_spec.callback(m_spec.userdata, dst, len);

    buf->datas[0].chunk->offset = 0;
    buf->datas[0].chunk->stride = m_stride;
    buf->datas[0].chunk->size = len;

    pw_stream_queue_buffer(m_stream, b);
}

PwAudioDriver::PwAudioDriver()
{
    m_deviceId = PW_DEFAULT_DEVICE;

    static bool initDone = false;

    if (!initDone) {
        pw_log_set_level(PW_LOG_LEVEL);
        pw_init(nullptr, nullptr);
        initDone = true;
    }

    m_loop = pw_thread_loop_new(PW_THREAD_NAME, nullptr);
    if (!m_loop) {
        return;
    }

    m_context = pw_context_new(pw_thread_loop_get_loop(m_loop), nullptr, 0);
    if (!m_context) {
        return;
    }

    pw_log_set_level(PW_LOG_LEVEL); // log level is reset by pw_context_new

    m_core = pw_context_connect(m_context, nullptr, 0);

    // m_core is valid if we are connected to PipeWire
    if (m_core) {
        m_registry = std::make_unique<PwRegistry>(m_loop, m_core);

        // Actually kick-in the device discovery
        // we do it here rather than in init() in order to have
        // the full list available before the stream is opened
        //
        // If not the stream will first open with "default", and reopen
        // with the preferred device (if not "default")
        //
        // The discovery is effectively done in masked time,
        // while all application modules are initialized.
        m_registry->init();
        pw_thread_loop_start(m_loop);
    }
}

PwAudioDriver::~PwAudioDriver()
{
    if (m_core) {
        m_registry.reset();

        pw_thread_loop_stop(m_loop);
        pw_core_disconnect(m_core);
    }
    if (m_context) {
        pw_context_destroy(m_context);
    }
    if (m_loop) {
        pw_thread_loop_destroy(m_loop);
    }
}

void PwAudioDriver::init()
{
    IF_ASSERT_FAILED_X(m_core, "Not connected to PipeWire. Should fallback to ALSA!") {
        return;
    }

    // Nothing to do
}

std::string PwAudioDriver::name() const { return "MUAUDIO(PipeWire)"; }

bool PwAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    IF_ASSERT_FAILED_X(m_core, "Not connected to PipeWire. Should fallback to ALSA!") {
        return false;
    }

    LOGD() << "Connecting to " << m_deviceId << " / " << spec.sampleRate << "Hz / " << spec.samples << " samples";

    m_stream = std::make_unique<PwStream>(m_loop, m_core, spec, m_deviceId);

    m_formatSpec = m_stream->spec();
    if (activeSpec) {
        *activeSpec = m_stream->spec();
    }

    LOGD() << "Connected to " << outputDevice();
    return true;
}

void PwAudioDriver::close()
{
    m_stream.reset();
}

bool PwAudioDriver::isOpened() const { return m_stream != nullptr; }

const IAudioDriver::Spec& PwAudioDriver::activeSpec() const { return m_formatSpec; }

AudioDeviceID PwAudioDriver::outputDevice() const { return m_deviceId; }

bool PwAudioDriver::selectOutputDevice(const AudioDeviceID& deviceId)
{
    LOGD() << "Selecting output device: " << deviceId;

    if (m_deviceId == deviceId) {
        LOGD() << "Output device already selected: " << deviceId;
        return true;
    }

    auto devices = availableOutputDevices();
    bool reopen = isOpened();

    close();

    auto dev = std::find_if(devices.begin(), devices.end(), [&](const AudioDevice& device) { return device.id == deviceId; });
    if (dev == devices.end()) {
        LOGW() << "Could not find device \"" << m_deviceId << "\". Falling back to \"" << PW_DEFAULT_DEVICE << "\"";
        m_deviceId = PW_DEFAULT_DEVICE;
    } else {
        LOGI() << "Selecting device \"" << dev->name << "\" (" << dev->id << ")";
        m_deviceId = deviceId;
    }

    bool ok = true;
    if (reopen) {
        LOGD() << "Reopening driver";
        ok = open(m_formatSpec, nullptr);
    }

    if (ok) {
        LOGD() << "Notifying device change";
        m_outputDeviceChanged.notify();
    }

    return ok;
}

bool PwAudioDriver::resetToDefaultOutputDevice()
{
    return selectOutputDevice(PW_DEFAULT_DEVICE);
}

async::Notification PwAudioDriver::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList PwAudioDriver::availableOutputDevices() const
{
    IF_ASSERT_FAILED_X(m_core, "Not connected to PipeWire. Should fallback to ALSA!") {
        return AudioDeviceList{};
    }

    return m_registry->getDevices();
}

async::Notification PwAudioDriver::availableOutputDevicesChanged() const
{
    IF_ASSERT_FAILED_X(m_core, "Not connected to PipeWire. Should fallback to ALSA!") {
        return async::Notification();
    }

    return m_registry->availableOutputDevicesChanged();
}

unsigned int PwAudioDriver::outputDeviceBufferSize() const
{
    return m_stream ? m_stream->spec().samples : 0;
}

bool PwAudioDriver::setOutputDeviceBufferSize(unsigned int bufferSize)
{
    if (m_formatSpec.samples == bufferSize) {
        return true;
    }

    bool reopen = isOpened();

    close();
    m_formatSpec.samples = bufferSize;

    bool ok = true;
    if (reopen) {
        ok = open(m_formatSpec, nullptr);
    }

    if (ok) {
        m_bufferSizeChanged.notify();
    }

    return ok;
}

async::Notification PwAudioDriver::outputDeviceBufferSizeChanged() const
{
    return m_bufferSizeChanged;
}

std::vector<unsigned int>
PwAudioDriver::availableOutputDeviceBufferSizes() const
{
    std::vector<unsigned int> result;

    // 0 stands for default buffer size
    result.push_back(0);

    unsigned int n = MAXIMUM_BUFFER_SIZE;
    while (n >= MINIMUM_BUFFER_SIZE) {
        result.push_back(n);
        n /= 2;
    }

    std::sort(result.begin(), result.end());

    return result;
}

unsigned int PwAudioDriver::outputDeviceSampleRate() const
{
    return m_stream ? m_stream->spec().sampleRate : 0;
}

bool PwAudioDriver::setOutputDeviceSampleRate(unsigned int sampleRate)
{
    if (m_formatSpec.sampleRate == sampleRate) {
        return true;
    }

    bool reopen = isOpened();

    close();
    m_formatSpec.sampleRate = sampleRate;

    bool ok = true;
    if (reopen) {
        ok = open(m_formatSpec, nullptr);
    }

    if (ok) {
        m_sampleRateChanged.notify();
    }

    return ok;
}

async::Notification PwAudioDriver::outputDeviceSampleRateChanged() const
{
    return m_sampleRateChanged;
}

std::vector<unsigned int>
PwAudioDriver::availableOutputDeviceSampleRates() const
{
    return {
        44100,
        48000,
        88200,
        96000,
    };
}

void PwAudioDriver::resume()
{
    if (m_stream) {
        m_stream->resume();
    }
}

void PwAudioDriver::suspend()
{
    if (m_stream) {
        m_stream->suspend();
    }
}
