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
#ifndef MU_ENGRAVING_TREAD_H
#define MU_ENGRAVING_TREAD_H

namespace mu::engraving {
class XmlReader;
class ReadContext;
class EngravingItem;
class TextBase;
class TempoText;
class StaffText;
class StaffTextBase;
class Dynamic;
class Expression;
class Harmony;
class FretDiagram;
class TremoloBar;
class Sticking;
class SystemText;
class PlayTechAnnotation;
class RehearsalMark;
class InstrumentChange;
class StaffState;
class FiguredBass;
class Fermata;
class Image;
class Tuplet;
class Beam;
class Ambitus;
class Accidental;
class Marker;
class Jump;
class MeasureNumber;
class MeasureNumberBase;
class MMRestRange;
class SystemDivider;
}

namespace mu::engraving::rw400 {
class TRead
{
public:
    TRead() = default;

    static void read(EngravingItem* el, XmlReader& xml, ReadContext& ctx); // factory

    static void read(TextBase* t, XmlReader& xml, ReadContext& ctx);
    static void read(TempoText* t, XmlReader& xml, ReadContext& ctx);
    static void read(StaffText* t, XmlReader& xml, ReadContext& ctx);
    static void read(StaffTextBase* t, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(StaffTextBase* t, XmlReader& xml, ReadContext& ctx);
    static void read(Dynamic* d, XmlReader& xml, ReadContext& ctx);
    static void read(Expression* expr, XmlReader& xml, ReadContext& ctx);
    static void read(Harmony* h, XmlReader& xml, ReadContext& ctx);
    static void read(FretDiagram* d, XmlReader& xml, ReadContext& ctx);
    static void read(TremoloBar* b, XmlReader& xml, ReadContext& ctx);
    static void read(Sticking* s, XmlReader& xml, ReadContext& ctx);
    static void read(SystemText* t, XmlReader& xml, ReadContext& ctx);
    static void read(PlayTechAnnotation* a, XmlReader& xml, ReadContext& ctx);
    static void read(RehearsalMark* m, XmlReader& xml, ReadContext& ctx);
    static void read(InstrumentChange* c, XmlReader& xml, ReadContext& ctx);
    static void read(StaffState* s, XmlReader& xml, ReadContext& ctx);
    static void read(FiguredBass* b, XmlReader& xml, ReadContext& ctx);
    static void read(Fermata* f, XmlReader& xml, ReadContext& ctx);
    static void read(Image* i, XmlReader& xml, ReadContext& ctx);
    static void read(Tuplet* t, XmlReader& xml, ReadContext& ctx);
    static void read(Beam* b, XmlReader& xml, ReadContext& ctx);
    static void read(Ambitus* a, XmlReader& xml, ReadContext& ctx);
    static void read(Accidental* a, XmlReader& xml, ReadContext& ctx);
    static void read(Marker* m, XmlReader& xml, ReadContext& ctx);
    static void read(Jump* j, XmlReader& xml, ReadContext& ctx);
    static void read(MeasureNumber* n, XmlReader& xml, ReadContext& ctx);
    static void read(MeasureNumberBase* b, XmlReader& xml, ReadContext& ctx);
    static void read(MMRestRange* r, XmlReader& xml, ReadContext& ctx);
    static void read(SystemDivider* d, XmlReader& xml, ReadContext& ctx);
};
}

#endif // MU_ENGRAVING_TREAD_H
