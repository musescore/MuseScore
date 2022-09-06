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
#ifndef MU_INSPECTOR_GENERALSETTINGSMODEL_H
#define MU_INSPECTOR_GENERALSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "playback/playbackproxymodel.h"
#include "appearance/appearancesettingsmodel.h"

namespace mu::inspector {
class GeneralSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isVisible READ isVisible CONSTANT)
    Q_PROPERTY(PropertyItem * isAutoPlaceAllowed READ isAutoPlaceAllowed CONSTANT)
    Q_PROPERTY(PropertyItem * isPlayable READ isPlayable CONSTANT)
    Q_PROPERTY(PropertyItem * isSmall READ isSmall CONSTANT)

    Q_PROPERTY(QObject * playbackProxyModel READ playbackProxyModel NOTIFY playbackProxyModelChanged)
    Q_PROPERTY(QObject * appearanceSettingsModel READ appearanceSettingsModel NOTIFY appearanceSettingsModelChanged)

public:
    explicit GeneralSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* isVisible() const;
    PropertyItem* isAutoPlaceAllowed() const;
    PropertyItem* isPlayable() const;
    PropertyItem* isSmall() const;

    QObject* playbackProxyModel() const;
    QObject* appearanceSettingsModel() const;

public slots:
    void setPlaybackProxyModel(mu::inspector::PlaybackProxyModel* playbackProxyModel);
    void setAppearanceSettingsModel(mu::inspector::AppearanceSettingsModel* appearanceSettingsModel);

signals:
    void playbackProxyModelChanged(QObject* playbackProxyModel);
    void appearanceSettingsModelChanged(QObject* appearanceSettingsModel);

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void updatePropertiesOnNotationChanged() override;

    PropertyItem* m_isVisible = nullptr;
    PropertyItem* m_isAutoPlaceAllowed = nullptr;
    PropertyItem* m_isPlayable = nullptr;
    PropertyItem* m_isSmall = nullptr;

    PlaybackProxyModel* m_playbackProxyModel = nullptr;
    AppearanceSettingsModel* m_appearanceSettingsModel = nullptr;
};
}

#endif // MU_INSPECTOR_GENERALSETTINGSMODEL_H
