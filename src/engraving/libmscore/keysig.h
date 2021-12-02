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

#ifndef __KEYSIG_H__
#define __KEYSIG_H__

#include "key.h"
#include "engravingitem.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
class Segment;

//---------------------------------------------------------------------------------------
//   @@ KeySig
///    The KeySig class represents a Key Signature on a staff
//
//   @P showCourtesy  bool  show courtesy key signature for this sig if appropriate
//---------------------------------------------------------------------------------------

class KeySig final : public EngravingItem
{
    bool _showCourtesy;
    bool _hideNaturals;       // used in layout to override score style (needed for the Continuous panel)
    KeySigEvent _sig;

    friend class mu::engraving::Factory;
    KeySig(Segment* = 0);
    KeySig(const KeySig&);

    void addLayout(SymId sym, int y);

public:

    KeySig* clone() const override { return new KeySig(*this); }
    void draw(mu::draw::Painter*) const override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;
    void layout() override;
    qreal mag() const override;

    //@ sets the key of the key signature
    Q_INVOKABLE void setKey(Key);

    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return explicitParent() ? (Measure*)explicitParent()->explicitParent() : nullptr; }
    void write(XmlWriter&) const override;
    void read(XmlReader&) override;
    //@ returns the key of the key signature (from -7 (flats) to +7 (sharps) )
    Q_INVOKABLE Key key() const { return _sig.key(); }
    bool isCustom() const { return _sig.custom(); }
    bool isAtonal() const { return _sig.isAtonal(); }
    bool isChange() const;
    KeySigEvent keySigEvent() const { return _sig; }
    bool operator==(const KeySig&) const;
    void changeKeySigEvent(const KeySigEvent&);
    void setKeySigEvent(const KeySigEvent& e) { _sig = e; }

    bool showCourtesy() const { return _showCourtesy; }
    void setShowCourtesy(bool v) { _showCourtesy = v; }
    void undoSetShowCourtesy(bool v);

    KeyMode mode() const { return _sig.mode(); }
    void setMode(KeyMode v) { _sig.setMode(v); }
    void undoSetMode(KeyMode v);

    void setHideNaturals(bool hide) { _hideNaturals = hide; }

    void setForInstrumentChange(bool forInstrumentChange) { _sig.setForInstrumentChange(forInstrumentChange); }
    bool forInstrumentChange() const { return _sig.forInstrumentChange(); }

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    QString accessibleInfo() const override;

    SymId convertFromOldId(int val) const;
};

extern const char* keyNames[];
}     // namespace Ms
#endif
