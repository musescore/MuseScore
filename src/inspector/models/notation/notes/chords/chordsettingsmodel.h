/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_INSPECTOR_CHORDSETTINGSMODEL_H
#define MU_INSPECTOR_CHORDSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class ChordSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isStemless READ isStemless CONSTANT)
    Q_PROPERTY(PropertyItem * showStemSlash READ showStemSlash CONSTANT)
    Q_PROPERTY(PropertyItem * combineVoice READ combineVoice CONSTANT)

    Q_PROPERTY(bool showStemSlashVisible READ showStemSlashVisible NOTIFY showStemSlashVisibleChanged)
    Q_PROPERTY(bool showStemSlashEnabled READ showStemSlashEnabled NOTIFY showStemSlashEnabledChanged)

public:
    explicit ChordSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* isStemless() const;
    PropertyItem* showStemSlash() const;
    PropertyItem* combineVoice() const;

    bool showStemSlashVisible() const;  //  chord is grace
    bool showStemSlashEnabled() const;  //  chord is not stemless

public slots:
    void setShowStemSlashVisible(bool showStemSlashVisible);
    void setShowStemSlashEnabled(bool showStemSlashEnabled);

signals:
    void showStemSlashVisibleChanged(bool showStemSlashVisible);
    void showStemSlashEnabledChanged(bool showStemSlashEnabled);

private:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    void updateShowStemSlashVisible();
    void updateShowStemSlashEnabled();

    PropertyItem* m_isStemless = nullptr;
    PropertyItem* m_showStemSlash = nullptr;
    PropertyItem* m_combineVoice = nullptr;

    bool m_showStemSlashVisible = false;
    bool m_showStemSlashEnabled = false;
};
}

#endif // MU_INSPECTOR_CHORDSETTINGSMODEL_H
