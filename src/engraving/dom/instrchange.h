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

#ifndef MU_ENGRAVING_INSTRCHANGE_H
#define MU_ENGRAVING_INSTRCHANGE_H

#include "textbase.h"

#include "instrument.h"

namespace mu::engraving {
class Clef;

//---------------------------------------------------------
//   @@ InstrumentChange
//---------------------------------------------------------

class InstrumentChange final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, InstrumentChange)
    DECLARE_CLASSOF(ElementType::INSTRUMENT_CHANGE)

public:
    InstrumentChange(EngravingItem* parent);
    InstrumentChange(const Instrument&, EngravingItem* parent);
    InstrumentChange(const InstrumentChange&);
    ~InstrumentChange();

    InstrumentChange* clone() const override { return new InstrumentChange(*this); }

    Instrument* instrument() const { return m_instrument; }
    void setInstrument(Instrument* i) { m_instrument = i; }
    void setInstrument(Instrument&& i) { *m_instrument = i; }
    void setInstrument(const Instrument& i);
    void setupInstrument(const Instrument* instrument);

    std::vector<KeySig*> keySigs(bool all=false) const;
    std::vector<Clef*> clefs() const;

    bool init() const { return m_init; }
    void setInit(bool init) { m_init = init; }

    Segment* segment() const { return toSegment(explicitParent()); }

    PropertyValue propertyDefault(Pid) const override;

    bool placeMultiple() const override { return false; }

    bool setProperty(Pid id, const PropertyValue& v) override;

    void setCustomText(const String& s) {
        _customText = s;
        _useCustomText = true;
        m_manualText = true;
    }

    String customText() const { return _customText; }

    bool hasCustomText() const { return _useCustomText; }

private:

    Instrument* m_instrument = nullptr;     // Staff holds ownership if part of score
    bool m_init = false;                    // Set if the instrument has been set by the user, as there is no other way to tell.
    String _customText;                     // Custom text entered by user for instrument name
    bool _useCustomText = false;            // True if using stored custom text instead of default
    bool m_manualText = false;              // Set when user edits text directly in UI
};
} // namespace mu::engraving
#endif
