/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_MUSESAMPLER_LIBHANDLER_H
#define MU_MUSESAMPLER_LIBHANDLER_H

#include <memory>
#include <cstring>

#ifdef USE_MUSESAMPLER_SRC
#include "Api/musesampler_api.h"
#else
#include "apitypes.h"
#endif

namespace mu::musesampler {
#ifdef USE_MUSESAMPLER_SRC
struct MuseSamplerLibHandler
{
    ms_Result initLib() { return ms_init(); }
    ms_InstrumentList getInstrumentList() { return ms_get_instrument_list(); }
    ms_InstrumentInfo getNextInstrument(ms_InstrumentList instrument_list) { return ms_InstrumentList_get_next(instrument_list); }
    int getInstrumentId(ms_InstrumentInfo instrument) { return ms_Instrument_get_id(instrument); }
    const char* getInstrumentName(ms_InstrumentInfo instrument) { return ms_Instrument_get_name(instrument); }
    const char* getMusicXmlSoundId(ms_InstrumentInfo instrument) { return ms_Instrument_get_musicxml_sound(instrument); }

    bool containsInstrument(const char* musicXmlSoundId)
    {
        auto instrumentList = getInstrumentList();
        if (instrumentList == nullptr) {
            return false;
        }

        while (auto instrument = getNextInstrument(instrumentList))
        {
            if (std::strcmp(getMusicXmlSoundId(instrument), musicXmlSoundId) == 0) {
                return true;
            }
        }

        return false;
    }

    ms_MuseSampler create() { return ms_MuseSampler_create(); }
    void destroy(ms_MuseSampler sampler) { ms_MuseSampler_destroy(sampler); }
    ms_Result initSampler(ms_MuseSampler ms, double sample_rate, int block_size, int channel_count)
    {
        return ms_MuseSampler_init(ms, sample_rate, block_size, channel_count);
    }

    ms_Result clearScore(ms_MuseSampler ms) { return ms_MuseSampler_clear_score(ms); }
    ms_Result finalizeScore(ms_MuseSampler ms) { return ms_MuseSampler_finalize_score(ms); }
    ms_Track addTrack(ms_MuseSampler ms, int instrument_id) { return ms_MuseSampler_add_track(ms, instrument_id); }
    ms_Result clearTrack(ms_MuseSampler ms, ms_Track track) { return ms_MuseSampler_clear_track(ms, track); }

    ms_Result addDynamicsEvent(ms_MuseSampler ms, ms_DynamicsEvent evt) { return ms_MuseSampler_add_dynamics_event(ms, evt); }
    ms_Result addTrackEvent(ms_MuseSampler ms, ms_Track track, ms_Event evt) { return ms_MuseSampler_add_track_event(ms, track, evt); }
    ms_Result process(ms_MuseSampler ms, ms_OutputBuffer buff, long long micros) { return ms_MuseSampler_process(ms, buff, micros); }

    MuseSamplerLibHandler(const char* /*path*/)
    {
        initLib();
    }

    bool isValid() const
    {
        return true;
    }
};
#else
struct MuseSamplerLibHandler
{
    ms_init initLib = nullptr;
    ms_get_instrument_list getInstrumentList = nullptr;
    ms_InstrumentList_get_next getNextInstrument = nullptr;
    ms_Instrument_get_id getInstrumentId = nullptr;
    ms_Instrument_get_name getInstrumentName = nullptr;
    ms_Instrument_get_musicxml_sound getMusicXmlSoundId = nullptr;

    ms_MuseSampler_create create = nullptr;
    ms_MuseSampler_destroy destroy = nullptr;
    ms_MuseSampler_init initSampler = nullptr;

    ms_MuseSampler_clear_score clearScore = nullptr;
    ms_MuseSampler_finalize_score finalizeScore = nullptr;
    ms_MuseSampler_add_track addTrack = nullptr;
    ms_MuseSampler_clear_track clearTrack = nullptr;

    ms_MuseSampler_add_dynamics_event addDynamicsEvent = nullptr;
    ms_MuseSampler_add_track_event addTrackEvent = nullptr;
    ms_MuseSampler_process process = nullptr;

    bool containsInstrument(const char* musicXmlSoundId)
    {
        if (!isValid()) {
            return false;
        }

        auto instrumentList = getInstrumentList();
        if (instrumentList == nullptr) {
            return false;
        }

        while (auto instrument = getNextInstrument(instrumentList))
        {
            if (std::strcmp(getMusicXmlSoundId(instrument), musicXmlSoundId) == 0) {
                return true;
            }
        }

        return false;
    }

    MuseSamplerLibHandler(const char* path)
    {
        m_lib = dlopen(path, RTLD_LAZY);

        if (!m_lib) {
            LOGE() << "Unable to open MuseSampler library, path: " << path;
            return;
        }

        initLib = (ms_init)dlsym(m_lib, "ms_init");
        getInstrumentList = (ms_get_instrument_list)dlsym(m_lib, "ms_get_instrument_list");
        getNextInstrument = (ms_InstrumentList_get_next)dlsym(m_lib, "ms_InstrumentList_get_next");
        getInstrumentId = (ms_Instrument_get_id)dlsym(m_lib, "ms_Instrument_get_id");
        getInstrumentName = (ms_Instrument_get_name)dlsym(m_lib, "ms_Instrument_get_name");
        getMusicXmlSoundId = (ms_Instrument_get_musicxml_sound)dlsym(m_lib, "ms_Instrument_get_musicxml_sound");

        create = (ms_MuseSampler_create)dlsym(m_lib, "ms_MuseSampler_create");
        destroy = (ms_MuseSampler_destroy)dlsym(m_lib, "ms_MuseSampler_destroy");
        initSampler = (ms_MuseSampler_init)dlsym(m_lib, "ms_MuseSampler_init");

        clearScore = (ms_MuseSampler_clear_score)dlsym(m_lib, "ms_MuseSampler_clear_score");
        finalizeScore = (ms_MuseSampler_finalize_score)dlsym(m_lib, "ms_MuseSampler_finalize_score");
        addTrack = (ms_MuseSampler_add_track)dlsym(m_lib, "ms_MuseSampler_add_track");
        clearTrack = (ms_MuseSampler_clear_track)dlsym(m_lib, "ms_MuseSampler_clear_track");

        addDynamicsEvent = (ms_MuseSampler_add_dynamics_event)dlsym(m_lib, "ms_MuseSampler_add_dynamics_event");
        addTrackEvent = (ms_MuseSampler_add_track_event)dlsym(m_lib, "ms_MuseSampler_add_track_event");

        process = (ms_MuseSampler_process)dlsym(m_lib, "ms_MuseSampler_process");

        initLib();
    }

    ~MuseSamplerLibHandler()
    {
        if (!m_lib) {
            return;
        }

        dlclose(m_lib);
    }

    bool isValid() const
    {
        return m_lib
               && initLib
               && getInstrumentList
               && getNextInstrument
               && getInstrumentId
               && getInstrumentName
               && getMusicXmlSoundId
               && create
               && destroy
               && initSampler
               && clearScore
               && finalizeScore
               && addTrack
               && clearTrack
               && addDynamicsEvent
               && addTrackEvent
               && process;
    }

private:
    MuseSamplerLib m_lib = nullptr;
};
#endif

using MuseSamplerLibHandlerPtr = std::shared_ptr<MuseSamplerLibHandler>;
}

#endif // MU_MUSESAMPLER_LIBHANDLER_H
