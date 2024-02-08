/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#ifndef MU_PLAYBACK_MUSESOUNDSPARAMSMODEL_H
#define MU_PLAYBACK_MUSESOUNDSPARAMSMODEL_H

#include <QObject>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "playback/iplaybackcontroller.h"
#include "playback/iplaybackconfiguration.h"

#include "notation/notationtypes.h"

namespace mu::playback {
class MuseSoundsParamsModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)
    INJECT(IPlaybackController, playbackController)
    INJECT(IPlaybackConfiguration, playbackConfiguration)

    Q_PROPERTY(QVariantList availablePresets READ availablePresets NOTIFY availablePresetsChanged FINAL)
    Q_PROPERTY(QStringList presetCodes READ presetCodes NOTIFY presetCodesChanged FINAL)

public:
    MuseSoundsParamsModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();
    Q_INVOKABLE void togglePreset(const QString& presetCode, bool forceMultiSelection);

    QVariantList availablePresets() const;
    void setAvailablePresets(const audio::SoundPresetList& presets);

    QStringList presetCodes() const;

signals:
    void availablePresetsChanged();
    void presetCodesChanged();

private:
    notation::INotationSelectionPtr selection() const;
    notation::INotationUndoStackPtr undoStack() const;

    engraving::EngravingItem* m_item = nullptr;
    QVariantList m_availablePresets;
};
}

#endif // MU_PLAYBACK_MUSESOUNDSPARAMSMODEL_H
