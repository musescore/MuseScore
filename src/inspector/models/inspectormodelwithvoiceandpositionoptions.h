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
#ifndef MU_INSPECTOR_INSPECTORMODELWITHVOICEANDPOSITIONOPTIONS_H
#define MU_INSPECTOR_INSPECTORMODELWITHVOICEANDPOSITIONOPTIONS_H

#include "abstractinspectormodel.h"

using namespace mu::engraving;

namespace mu::inspector {
class InspectorModelWithVoiceAndPositionOptions : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * voiceBasedPosition READ voiceBasedPosition CONSTANT)
    Q_PROPERTY(PropertyItem * voiceAssignment READ voiceAssignment CONSTANT)
    Q_PROPERTY(PropertyItem * voice READ voice CONSTANT)
    Q_PROPERTY(PropertyItem * centerBetweenStaves READ centerBetweenStaves CONSTANT)
    Q_PROPERTY(bool isMultiStaffInstrument READ isMultiStaffInstrument WRITE setIsMultiStaffInstrument NOTIFY isMultiStaffInstrumentChanged)
    Q_PROPERTY(
        bool isStaveCenteringAvailable READ isStaveCenteringAvailable WRITE setIsStaveCenteringAvailable NOTIFY isStaveCenteringAvailableChanged)

public:
    explicit InspectorModelWithVoiceAndPositionOptions(QObject* parent, IElementRepositoryService* repository,
                                                       ElementType elementType = ElementType::INVALID);

    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const PropertyIdSet&, const StyleIdSet&) override;

    PropertyItem* voiceBasedPosition() const;
    PropertyItem* voiceAssignment() const;
    PropertyItem* voice() const;
    PropertyItem* centerBetweenStaves() const;
    bool isMultiStaffInstrument() const;
    bool isStaveCenteringAvailable() const;

    Q_INVOKABLE void changeVoice(int voice);

public slots:
    void setIsMultiStaffInstrument(bool v);
    void setIsStaveCenteringAvailable(bool v);

signals:
    void isMultiStaffInstrumentChanged(bool isMultiStaffInstrument);
    void isStaveCenteringAvailableChanged(bool isStaveCenteringAvailable);

private:
    void updateIsMultiStaffInstrument();
    void updateIsStaveCenteringAvailable();

private:
    PropertyItem* m_voiceBasedPosition = nullptr;
    PropertyItem* m_voiceAssignment = nullptr;
    PropertyItem* m_voice = nullptr;
    PropertyItem* m_centerBetweenStaves = nullptr;
    bool m_isMultiStaffInstrument = false;
    bool m_isStaveCenteringAvailable = false;
};
}

#endif // MU_INSPECTOR_INSPECTORMODELWITHVOICEANDPOSITIONOPTIONS_H
