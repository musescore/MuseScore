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
#ifndef MU_INSPECTOR_FRETDIAGRAMSETTINGSMODEL_H
#define MU_INSPECTOR_FRETDIAGRAMSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

#include "types/fretdiagramtypes.h"

namespace mu::engraving {
class FretDiagram;
}

namespace mu::inspector {
class FretDiagramSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * scale READ scale CONSTANT)
    Q_PROPERTY(PropertyItem * stringsCount READ stringsCount CONSTANT)
    Q_PROPERTY(PropertyItem * fretsCount READ fretsCount CONSTANT)
    Q_PROPERTY(PropertyItem * fretNumber READ fretNumber CONSTANT)
    Q_PROPERTY(PropertyItem * isNutVisible READ isNutVisible CONSTANT)
    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)
    Q_PROPERTY(PropertyItem * orientation READ orientation CONSTANT)

    Q_PROPERTY(bool isBarreModeOn READ isBarreModeOn WRITE setIsBarreModeOn NOTIFY isBarreModeOnChanged)
    Q_PROPERTY(bool isMultipleDotsModeOn READ isMultipleDotsModeOn WRITE setIsMultipleDotsModeOn NOTIFY isMultipleDotsModeOnChanged)
    Q_PROPERTY(int currentFretDotType READ currentFretDotType WRITE setCurrentFretDotType NOTIFY currentFretDotTypeChanged)

    Q_PROPERTY(bool areSettingsAvailable READ areSettingsAvailable NOTIFY areSettingsAvailableChanged)

    Q_PROPERTY(QVariant fretDiagram READ fretDiagram NOTIFY fretDiagramChanged)
    Q_PROPERTY(PropertyItem * showFingerings READ showFingerings CONSTANT)
    Q_PROPERTY(QStringList fingerings READ fingerings NOTIFY fingeringsChanged)

public:
    explicit FretDiagramSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* scale() const;
    PropertyItem* stringsCount() const;
    PropertyItem* fretsCount() const;
    PropertyItem* fretNumber() const;
    PropertyItem* isNutVisible() const;
    PropertyItem* placement() const;
    PropertyItem* orientation() const;
    PropertyItem* showFingerings() const;
    QStringList fingerings() const;

    Q_INVOKABLE void setFingering(int string, int finger);
    Q_INVOKABLE void resetFingerings();

    QVariant fretDiagram() const;

    bool isBarreModeOn() const;
    bool isMultipleDotsModeOn() const;
    int currentFretDotType() const;

    bool areSettingsAvailable() const;

public slots:
    void setIsBarreModeOn(bool isBarreModeOn);
    void setIsMultipleDotsModeOn(bool isMultipleDotsModeOn);
    void setCurrentFretDotType(int currentFretDotType);

signals:
    void fretDiagramChanged(QVariant fretDiagram);

    void isBarreModeOnChanged(bool isBarreModeOn);
    void isMultipleDotsModeOnChanged(bool isMultipleDotsModeOn);
    void currentFretDotTypeChanged(int currentFretDotType);

    void areSettingsAvailableChanged(bool areSettingsAvailable);
    void fingeringsChanged(QStringList fingerings);

private:
    PropertyItem* m_scale = nullptr;
    PropertyItem* m_stringsCount = nullptr;
    PropertyItem* m_fretsCount = nullptr;
    PropertyItem* m_fretNumber = nullptr;
    PropertyItem* m_isNutVisible = nullptr;
    PropertyItem* m_placement = nullptr;
    PropertyItem* m_orientation = nullptr;
    PropertyItem* m_showFingerings = nullptr;
    PropertyItem* m_fingerings = nullptr;

    mu::engraving::FretDiagram* m_fretDiagram = nullptr;

    bool m_isBarreModeOn = false;
    bool m_isMultipleDotsModeOn = false;
    FretDiagramTypes::FretDot m_currentFretDotType = FretDiagramTypes::FretDot::DOT_NORMAL;
};
}

#endif // MU_INSPECTOR_FRETDIAGRAMSETTINGSMODEL_H
