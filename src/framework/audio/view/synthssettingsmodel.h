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
#ifndef MU_MIDI_SYNTHSSETTINGSMODEL_H
#define MU_MIDI_SYNTHSSETTINGSMODEL_H

#include <QObject>
#include <QMap>
#include <QStringList>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "iinteractive.h"

#include "itracks.h"
#include "iplayback.h"
#include "iaudioconfiguration.h"

namespace mu::audio::synth {
class SynthsSettingsModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(audio, IPlayback, playback)
    INJECT(audio, IAudioConfiguration, configuration)
    INJECT(audio, framework::IInteractive, interactive)

public:
    explicit SynthsSettingsModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void apply();

    Q_INVOKABLE QStringList selectedSoundFonts(const QString& synth) const;
    Q_INVOKABLE QStringList avalaibleSoundFonts(const QString& synth) const;

    Q_INVOKABLE void soundFontUp(int selectedIndex, const QString& synth);
    Q_INVOKABLE void soundFontDown(int selectedIndex, const QString& synth);
    Q_INVOKABLE void removeSoundFont(int selectedIndex, const QString& synth);
    Q_INVOKABLE void addSoundFont(int avalableIndex, const QString& synth);

signals:
    void selectedChanged(const QString& name);
    void avalaibleChanged(const QString& name);

private:

    SynthesizerState m_state;
    QMap<QString, QStringList> m_avalaibleSoundFonts;
};
}

#endif // MU_MIDI_SYNTHSSETTINGSMODEL_H
