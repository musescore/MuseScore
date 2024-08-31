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

#ifndef MU_ENGRAVING_GRADUALTEMPOCHANGE_H
#define MU_ENGRAVING_GRADUALTEMPOCHANGE_H

#include <optional>

#include "textlinebase.h"
#include "../types/types.h"

namespace mu::engraving {
class GradualTempoChangeSegment;
class GradualTempoChange : public TextLineBase
{
    OBJECT_ALLOCATOR(engraving, GradualTempoChange)
    DECLARE_CLASSOF(ElementType::GRADUAL_TEMPO_CHANGE)

public:
    GradualTempoChange(EngravingItem* parent);

    GradualTempoChange* clone() const override;

    LineSegment* createLineSegment(System* parent) override;

    GradualTempoChangeType tempoChangeType() const;
    ChangeMethod easingMethod() const;
    void setTempoChangeType(const GradualTempoChangeType type);

    double tempoChangeFactor() const;

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid id, const PropertyValue& val) override;
    PropertyValue propertyDefault(Pid propertyId) const override;
    Sid getPropertyStyle(Pid id) const override;

    bool snapToItemAfter() const { return m_snapToItemAfter; }
    void setSnapToItemAfter(bool v) { m_snapToItemAfter = v; }

protected:
    void added() override;
    void removed() override;

private:
    void requestToRebuildTempo();

    GradualTempoChangeType m_tempoChangeType = GradualTempoChangeType::Undefined;
    ChangeMethod m_tempoEasingMethod = ChangeMethod::NORMAL;
    std::optional<float> m_tempoChangeFactor;

    bool m_snapToItemAfter = true;

    friend class GradualTempoChangeSegment;
};

class GradualTempoChangeSegment : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, GradualTempoChangeSegment)
    DECLARE_CLASSOF(ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT)

public:
    GradualTempoChangeSegment(GradualTempoChange* annotation, System* parent);

    GradualTempoChangeSegment* clone() const override;

    GradualTempoChange* tempoChange() const;

    void endEdit(EditData& editData) override;
    void added() override;
    void removed() override;
    Sid getPropertyStyle(Pid id) const override;

    GradualTempoChangeSegment* findElementToSnapBefore() const;
    TempoText* findElementToSnapAfter() const;

    friend class GradualTempoChange;
};
}

#endif // MU_ENGRAVING_GRADUALTEMPOCHANGE_H
