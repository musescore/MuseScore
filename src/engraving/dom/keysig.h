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

#ifndef MU_ENGRAVING_KEYSIG_H
#define MU_ENGRAVING_KEYSIG_H

#include "key.h"
#include "engravingitem.h"

namespace mu::engraving {
class Factory;
class Segment;

//---------------------------------------------------------------------------------------
//   @@ KeySig
///    The KeySig class represents a Key Signature on a staff
//
//   @P showCourtesy  bool  show courtesy key signature for this sig if appropriate
//---------------------------------------------------------------------------------------

class KeySig final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, KeySig)
    DECLARE_CLASSOF(ElementType::KEYSIG)

    M_PROPERTY2(bool, isCourtesy, setIsCourtesy, false)

public:

    KeySig* clone() const override { return new KeySig(*this); }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    double mag() const override;

    //@ sets the key of the key signature (concert key and transposing key)
    void setKey(Key cKey, Key tKey);
    void setKey(Key cKey);

    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return explicitParent() ? (Measure*)explicitParent()->explicitParent() : nullptr; }

    //@ returns the key of the actual key signature (from -7 (flats) to +7 (sharps) )
    Key key() const { return m_sig.key(); }
    //@ returns the key of the concert key signature
    Key concertKey() const { return m_sig.concertKey(); }
    const std::vector<CustDef>& customKeyDefs() const { return m_sig.customKeyDefs(); }
    int degInKey(int degree) const { return m_sig.degInKey(degree); }
    SymId symInKey(SymId sym, int degree) const { return m_sig.symInKey(sym, degree); }
    bool isCustom() const { return m_sig.custom(); }
    bool isAtonal() const { return m_sig.isAtonal(); }
    bool isChange() const;
    const KeySigEvent& keySigEvent() const { return m_sig; }
    bool operator==(const KeySig&) const;
    void changeKeySigEvent(const KeySigEvent&);
    void setKeySigEvent(const KeySigEvent& e) { m_sig = e; }

    bool showCourtesy() const { return m_showCourtesy; }
    void setShowCourtesy(bool v) { m_showCourtesy = v; }
    void undoSetShowCourtesy(bool v);

    KeyMode mode() const { return m_sig.mode(); }
    void setMode(KeyMode v) { m_sig.setMode(v); }
    void undoSetMode(KeyMode v);

    PointF staffOffset() const override;

    bool hideNaturals() const { return m_hideNaturals; }
    void setHideNaturals(bool hide) { m_hideNaturals = hide; }

    void setForInstrumentChange(bool forInstrumentChange) { m_sig.setForInstrumentChange(forInstrumentChange); }
    bool forInstrumentChange() const { return m_sig.forInstrumentChange(); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;

    int subtype() const override { return int(key()) + 7; }
    muse::TranslatableString subtypeUserName() const override;

    struct LayoutData : public EngravingItem::LayoutData {
        std::vector<KeySym> keySymbols;
    };
    DECLARE_LAYOUTDATA_METHODS(KeySig)

private:
    friend class Factory;

    KeySig(Segment* = 0);
    KeySig(const KeySig&);

    void addLayout(SymId sym, int line);

    bool m_showCourtesy;
    bool m_hideNaturals;       // used in layout to override score style (needed for the Continuous panel)
    KeySigEvent m_sig;
};
} // namespace mu::engraving
#endif
