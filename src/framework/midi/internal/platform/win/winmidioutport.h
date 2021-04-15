/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_MIDI_WINMIDIOUTPORT_H
#define MU_MIDI_WINMIDIOUTPORT_H

#include <memory>
#include "midi/imidioutport.h"

namespace mu {
namespace midi {
class WinMidiOutPort : public IMidiOutPort
{
public:
    WinMidiOutPort();
    ~WinMidiOutPort() override;

    std::vector<MidiDevice> devices() const override;

    Ret connect(const std::string& deviceID) override;
    void disconnect() override;
    bool isConnected() const override;
    std::string deviceID() const override;

    Ret sendEvent(const Event& e) override;

private:

    struct Win;
    std::unique_ptr<Win> m_win;
    std::string m_deviceID;
};
}
}

#endif // MU_MIDI_WINMIDIOUTPORT_H
