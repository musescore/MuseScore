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

#ifndef MU_ENGRAVING_JUMP_H
#define MU_ENGRAVING_JUMP_H

#include "textbase.h"

#include "../types/types.h"

namespace mu::engraving {
//---------------------------------------------------------
//   @@ Jump
///    Jump label
//
//   @P continueAt  string
//   @P jumpTo      string
// not used?
//      jumpType    enum (Jump.DC, .DC_AL_FINE, .DC_AL_CODA, .DS_AL_CODA, .DS_AL_FINE, .DS, USER) (read only)
//   @P playUntil   string
//---------------------------------------------------------

class Jump final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, Jump)
    DECLARE_CLASSOF(ElementType::JUMP)

public:

    Jump(Measure* parent);

    void setJumpType(JumpType t);
    JumpType jumpType() const;
    String jumpTypeUserName() const;

    Jump* clone() const override { return new Jump(*this); }

    int subtype() const override { return int(jumpType()); }
    TranslatableString subtypeUserName() const override;

    Measure* measure() const { return toMeasure(explicitParent()); }

    String jumpTo() const { return m_jumpTo; }
    String playUntil() const { return m_playUntil; }
    String continueAt() const { return m_continueAt; }
    void setJumpTo(const String& s) { m_jumpTo = s; }
    void setPlayUntil(const String& s) { m_playUntil = s; }
    void setContinueAt(const String& s) { m_continueAt = s; }
    void undoSetJumpTo(const String& s);
    void undoSetPlayUntil(const String& s);
    void undoSetContinueAt(const String& s);
    bool playRepeats() const { return m_playRepeats; }
    void setPlayRepeats(bool val) { m_playRepeats = val; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;

    void setLayoutToParentWidth(bool v) { m_layoutToParentWidth = v; }

private:
    String m_jumpTo;
    String m_playUntil;
    String m_continueAt;
    bool m_playRepeats = false;
};

struct JumpTypeTableItem {
    JumpType type;
    AsciiStringView text;
    AsciiStringView jumpTo;
    AsciiStringView playUntil;
    AsciiStringView continueAt;
};

extern const std::vector<JumpTypeTableItem> jumpTypeTable;
} // namespace mu::engraving

#endif
