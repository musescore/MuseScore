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
#ifndef MU_INSPECTOR_ORNAMENTSETTINGSMODEL_H
#define MU_INSPECTOR_ORNAMENTSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class OrnamentSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

    Q_PROPERTY(PropertyItem * intervalAbove READ intervalAbove CONSTANT)
    Q_PROPERTY(bool isIntervalAboveAvailable READ isIntervalAboveAvailable NOTIFY isIntervalAboveAvailableChanged)

    Q_PROPERTY(PropertyItem * intervalBelow READ intervalBelow CONSTANT)
    Q_PROPERTY(bool isIntervalBelowAvailable READ isIntervalBelowAvailable NOTIFY isIntervalBelowAvailableChanged)

    Q_PROPERTY(PropertyItem * intervalStep READ intervalStep CONSTANT)
    Q_PROPERTY(PropertyItem * intervalType READ intervalType CONSTANT)
    Q_PROPERTY(bool isFullIntervalChoiceAvailable READ isFullIntervalChoiceAvailable NOTIFY isFullIntervalChoiceAvailableChanged)
    Q_PROPERTY(bool isPerfectStep READ isPerfectStep NOTIFY isPerfectStepChanged)

    Q_PROPERTY(PropertyItem * showAccidental READ showAccidental CONSTANT)
    Q_PROPERTY(PropertyItem * startOnUpperNote READ startOnUpperNote CONSTANT)

    Q_PROPERTY(PropertyItem * showCueNote READ showCueNote CONSTANT)

public:
    explicit OrnamentSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* placement() const;
    PropertyItem* intervalAbove() const;
    PropertyItem* intervalBelow() const;
    PropertyItem* intervalStep() const;
    PropertyItem* intervalType() const;
    PropertyItem* showAccidental() const;
    PropertyItem* showCueNote() const;
    PropertyItem* startOnUpperNote() const;

    bool isIntervalAboveAvailable() const;
    bool isIntervalBelowAvailable() const;
    bool isFullIntervalChoiceAvailable() const;
    bool isPerfectStep() const;

public slots:
    void setIsIntervalAboveAvailable(bool available);
    void setIsIntervalBelowAvailable(bool available);
    void setIsFullIntervalChoiceAvailable(bool available);
    void setIsPerfectStep(bool isPerfect);

signals:
    void isIntervalAboveAvailableChanged(bool available);
    void isIntervalBelowAvailableChanged(bool available);
    void isFullIntervalChoiceAvailableChanged(bool available);
    void isPerfectStepChanged(bool perfect);

private:
    void updateIsIntervalAboveAvailable();
    void updateIsIntervalBelowAvailable();
    void updateIsFullIntervalChoiceAvailable();
    void updateIsPerfectStep();
    void setIntervalStep(engraving::Pid id, mu::engraving::IntervalStep step);
    void setIntervalType(engraving::Pid id, mu::engraving::IntervalType type);

private:
    PropertyItem* m_placement = nullptr;

    PropertyItem* m_intervalAbove = nullptr;
    bool m_isIntervalAboveAvailable = false;

    PropertyItem* m_intervalBelow = nullptr;
    bool m_isIntervalBelowAvailable = false;

    PropertyItem* m_intervalStep = nullptr;
    PropertyItem* m_intervalType = nullptr;
    bool m_isFullIntervalChoiceAvailable = false;
    bool m_isPerfectStep = false;

    PropertyItem* m_showAccidental = nullptr;
    PropertyItem* m_startOnUpperNote = nullptr;

    PropertyItem* m_showCueNote = nullptr;
};
}

#endif // MU_INSPECTOR_ORNAMENTSETTINGSMODEL_H
