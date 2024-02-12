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

#ifndef MU_PLAYBACK_SOUNDFLAGSETTINGSMODEL_H
#define MU_PLAYBACK_SOUNDFLAGSETTINGSMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "notation/view/abstractelementpopupmodel.h"

namespace mu::engraving {
class StaffText;
}

namespace mu::playback {
class SoundFlagSettingsModel : public notation::AbstractElementPopupModel
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)

    enum class SourceType {
        Undefined,
        MuseSounds,
    };
    Q_ENUM(SourceType)

    Q_PROPERTY(SourceType sourceType READ sourceType NOTIFY sourceTypeChanged FINAL)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)

    Q_PROPERTY(QRect iconRect READ iconRect NOTIFY iconRectChanged FINAL)

public:
    explicit SoundFlagSettingsModel(QObject* parent = nullptr);

    Q_INVOKABLE void init() override;

    SourceType sourceType() const;
    void setSourceType(SourceType type);

    QString title() const;
    void setTitle(const QString& title);

    QString text() const;
    void setText(const QString& text);

    QRect iconRect() const;

signals:
    void sourceTypeChanged();
    void titleChanged();
    void showTextChanged();
    void textChanged();

    void iconRectChanged();

private:
    project::IProjectAudioSettingsPtr audioSettings() const;

    audio::AudioInputParams currentAudioInputParams() const;

    void initSourceType();
    void initTitle();

    engraving::StaffText* staffText() const;

    SourceType m_sourceType = SourceType::Undefined;
    QString m_title;
};
}

#endif // MU_PLAYBACK_SOUNDFLAGSETTINGSMODEL_H
