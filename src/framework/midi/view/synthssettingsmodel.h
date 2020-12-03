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
#ifndef MU_MIDI_SYNTHSSETTINGSMODEL_H
#define MU_MIDI_SYNTHSSETTINGSMODEL_H

#include <QObject>
#include <QMap>
#include <QStringList>

#include "modularity/ioc.h"
#include "midi/isoundfontsprovider.h"
#include "midi/isynthesizersregister.h"
#include "midi/imidiconfiguration.h"
#include "iinteractive.h"

namespace mu {
namespace midi {
class SynthsSettingsModel : public QObject
{
    Q_OBJECT

    INJECT(midi, ISoundFontsProvider, sfprovider)
    INJECT(midi, ISynthesizersRegister, synthRegister)
    INJECT(midi, IMidiConfiguration, configuration)
    INJECT(midi, framework::IInteractive, interactive)

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
}

#endif // MU_MIDI_SYNTHSSETTINGSMODEL_H
