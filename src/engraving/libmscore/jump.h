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

#ifndef __JUMP_H__
#define __JUMP_H__

#include "text.h"

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
    String _jumpTo;
    String _playUntil;
    String _continueAt;
    bool _playRepeats;

public:
    enum class Type : char {
        DC,
        DC_AL_FINE,
        DC_AL_CODA,
        DS_AL_CODA,
        DS_AL_FINE,
        DS,
        DC_AL_DBLCODA,
        DS_AL_DBLCODA,
        DSS,
        DSS_AL_CODA,
        DSS_AL_DBLCODA,
        DSS_AL_FINE,
        DCODA,
        DDBLCODA,
        USER
    };

    Jump(Measure* parent);

    void setJumpType(Type t);
    Type jumpType() const;
    String jumpTypeUserName() const;

    Jump* clone() const override { return new Jump(*this); }

    int subtype() const override { return int(jumpType()); }

    Measure* measure() const { return toMeasure(explicitParent()); }

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;

    void layout() override;

    String jumpTo() const { return _jumpTo; }
    String playUntil() const { return _playUntil; }
    String continueAt() const { return _continueAt; }
    void setJumpTo(const String& s) { _jumpTo = s; }
    void setPlayUntil(const String& s) { _playUntil = s; }
    void setContinueAt(const String& s) { _continueAt = s; }
    void undoSetJumpTo(const String& s);
    void undoSetPlayUntil(const String& s);
    void undoSetContinueAt(const String& s);
    bool playRepeats() const { return _playRepeats; }
    void setPlayRepeats(bool val) { _playRepeats = val; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;
};

struct JumpTypeTableItem {
    Jump::Type type;
    AsciiStringView text;
    AsciiStringView jumpTo;
    AsciiStringView playUntil;
    AsciiStringView continueAt;
    const char* userText = nullptr;
};

extern const std::vector<JumpTypeTableItem> jumpTypeTable;
} // namespace mu::engraving

Q_DECLARE_METATYPE(mu::engraving::Jump::Type);

#endif
