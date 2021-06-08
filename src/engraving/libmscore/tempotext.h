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

#ifndef __TEMPOTEXT_H__
#define __TEMPOTEXT_H__

#include "durationtype.h"
#include "text.h"

namespace Ms {
//-------------------------------------------------------------------
//   @@ TempoText
///    Tempo marker which determines the midi tempo.
//
//   @P tempo       float     tempo in quarter notes (crochets) per second
//   @P followText  bool      determine tempo from text
//-------------------------------------------------------------------

class TempoText final : public TextBase
{
    qreal _tempo;             // beats per second
    bool _followText;         // parse text to determine tempo
    qreal _relative;
    bool _isRelative;

    void updateScore();
    void updateTempo();
    void endEdit(EditData&) override;
    void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;

public:
    TempoText(Score*);

    TempoText* clone() const override { return new TempoText(*this); }
    ElementType type() const override { return ElementType::TEMPO_TEXT; }

    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;

    Segment* segment() const { return toSegment(parent()); }
    Measure* measure() const { return toMeasure(parent()->parent()); }

    qreal tempo() const { return _tempo; }
    qreal tempoBpm() const;
    void setTempo(qreal v);
    void undoSetTempo(qreal v);
    bool isRelative() { return _isRelative; }
    void setRelative(qreal v) { _isRelative = true; _relative = v; }

    bool followText() const { return _followText; }
    void setFollowText(bool v) { _followText = v; }
    void undoSetFollowText(bool v);
    void updateRelative();

    void layout() override;

    TDuration duration() const;

    static int findTempoDuration(const QString& s, int& len, TDuration& dur);
    static QString duration2tempoTextString(const TDuration dur);
    static QString duration2userName(const TDuration t);

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid id) const override;
    QString accessibleInfo() const override;
};
}     // namespace Ms
#endif
