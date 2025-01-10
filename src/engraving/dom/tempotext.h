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

#ifndef MU_ENGRAVING_TEMPOTEXT_H
#define MU_ENGRAVING_TEMPOTEXT_H

#include "durationtype.h"
#include "textbase.h"

namespace mu::engraving {
enum class TempoTextType : signed char
{
    NORMAL,
    A_TEMPO,
    TEMPO_PRIMO,
};

//-------------------------------------------------------------------
//   @@ TempoText
///    Tempo marker which determines the midi tempo.
//
//   @P tempo       float     tempo in quarter notes (crochets) per second
//   @P followText  bool      determine tempo from text
//-------------------------------------------------------------------

class TempoText final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, TempoText)
    DECLARE_CLASSOF(ElementType::TEMPO_TEXT)

public:
    TempoText(Segment* parent);

    TempoText* clone() const override { return new TempoText(*this); }

    Segment* segment() const { return toSegment(explicitParent()); }
    Measure* measure() const { return toMeasure(explicitParent()->explicitParent()); }

    TempoTextType tempoTextType() const { return m_tempoTextType; }
    void setTempoTextType(TempoTextType);

    BeatsPerSecond tempo() const { return m_tempo; }
    double tempoBpm() const;
    void setTempo(BeatsPerSecond v);
    bool isRelative() const { return m_isRelative; }
    void setRelative(double v) { m_isRelative = true; m_relative = v; }

    bool isNormal() const { return m_tempoTextType == TempoTextType::NORMAL; }
    void setNormal() { setTempoTextType(TempoTextType::NORMAL); }

    bool isATempo() const { return m_tempoTextType == TempoTextType::A_TEMPO; }
    void setATempo() { setTempoTextType(TempoTextType::A_TEMPO); }

    bool isTempoPrimo() const { return m_tempoTextType == TempoTextType::TEMPO_PRIMO; }
    void setTempoPrimo() { setTempoTextType(TempoTextType::TEMPO_PRIMO); }

    bool playTempoText() const { return m_playTempoText; }
    void setPlayTempoText(bool v) { m_playTempoText = v; }

    bool followText() const { return m_followText; }
    void setFollowText(bool v) { m_followText = v; }

    void updateTempo();
    void updateRelative();

    TDuration duration() const;

    static int findTempoDuration(const String& s, int& len, TDuration& dur);
    static String duration2tempoTextString(const TDuration dur);
    static String duration2userName(const TDuration t);

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;
    String accessibleInfo() const override;

protected:
    void added() override;
    void removed() override;
    void commitText() override;

    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;

    void updateScore();

    TempoTextType m_tempoTextType;
    BeatsPerSecond m_tempo;             // beats per second
    bool m_followText = false;          // parse text to determine tempo
    bool m_playTempoText = true;
    double m_relative = 0.0;
    bool m_isRelative = false;
};
} // namespace mu::engraving
#endif
