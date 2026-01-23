/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include "audio/common/audiotypes.h"

namespace muse::audio {
static constexpr volume_dbfs_t MINIMUM_OPERABLE_DBFS_LEVEL = volume_dbfs_t::make(-100.f);

struct AudioSignalsNotifier
{
    void updateSignalValue(const audioch_t audioChNumber, const float newPeak)
    {
        volume_dbfs_t newPressure = (newPeak > 0.f) ? volume_dbfs_t(muse::linear_to_db(newPeak)) : MINIMUM_OPERABLE_DBFS_LEVEL;
        newPressure = std::max(newPressure, MINIMUM_OPERABLE_DBFS_LEVEL);

        AudioSignalVal& signalVal = m_signalValuesMap[audioChNumber];

        if (muse::is_equal(signalVal.pressure, newPressure)) {
            return;
        }

        if (std::abs(signalVal.pressure - newPressure) < PRESSURE_MINIMAL_VALUABLE_DIFF) {
            return;
        }

        signalVal.pressure = newPressure;
        m_shouldNotifyAboutChanges = true;
    }

    void notifyAboutChanges()
    {
        if (m_shouldNotifyAboutChanges) {
            audioSignalChanges.send(m_signalValuesMap);
            m_shouldNotifyAboutChanges = false;
        }
    }

    //! NOTE It would be nice if the driver callback was called in one thread.
    //! But some drivers, for example PipeWire, use queues
    //! And then the callback can be called in different threads.
    //! If a score is open, we will change the audio API (change the driver)
    //! then the number of threads used may increase...
    //! Channels allow 10 threads by default. Here we're increasing that to the maximum...
    //! If this is not enough, then we need to make sure that the callback is called in one thread,
    //! or use something else here instead of channels, some kind of queues.
    AudioSignalChanges audioSignalChanges = AudioSignalChanges(async::makeOpt()
                                                               .threads(100)
                                                               .disableWaitPendingsOnSend());

private:
    static constexpr volume_dbfs_t PRESSURE_MINIMAL_VALUABLE_DIFF = volume_dbfs_t::make(2.5f);

    AudioSignalValuesMap m_signalValuesMap;
    bool m_shouldNotifyAboutChanges = false;
};
}
