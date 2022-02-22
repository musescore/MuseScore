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

namespace Ms {
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
    QString _jumpTo;
    QString _playUntil;
    QString _continueAt;
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
    QString jumpTypeUserName() const;

    Jump* clone() const override { return new Jump(*this); }

    int subtype() const override { return int(jumpType()); }

    Measure* measure() const { return toMeasure(explicitParent()); }

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;

    void layout() override;

    QString jumpTo() const { return _jumpTo; }
    QString playUntil() const { return _playUntil; }
    QString continueAt() const { return _continueAt; }
    void setJumpTo(const QString& s) { _jumpTo = s; }
    void setPlayUntil(const QString& s) { _playUntil = s; }
    void setContinueAt(const QString& s) { _continueAt = s; }
    void undoSetJumpTo(const QString& s);
    void undoSetPlayUntil(const QString& s);
    void undoSetContinueAt(const QString& s);
    bool playRepeats() const { return _playRepeats; }
    void setPlayRepeats(bool val) { _playRepeats = val; }

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    QString accessibleInfo() const override;
};

//---------------------------------------------------------
//   JumpTypeTable
//---------------------------------------------------------

struct JumpTypeTable {
    Jump::Type type;
    const char* text;
    const char* jumpTo;
    const char* playUntil;
    const char* continueAt;
    QString userText;
};

extern const JumpTypeTable jumpTypeTable[];
int jumpTypeTableSize();
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Jump::Type);

#endif
