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

#include "style.h"

#include "compat/pageformat.h"
#include "rw/compat/readchordlisthook.h"
#include "rw/xml.h"
#include "types/typesconv.h"

#include "libmscore/mscore.h"

#include "defaultstyle.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

const PropertyValue& MStyle::value(Sid idx) const
{
    if (idx == Sid::NOSTYLE) {
        static PropertyValue dummy;
        return dummy;
    }

    const PropertyValue& val = m_values[size_t(idx)];
    if (val.isValid()) {
        return val;
    }

    return StyleDef::styleValues[size_t(idx)].defaultValue();
}

Millimetre MStyle::valueMM(Sid idx) const
{
    if (idx == Sid::NOSTYLE) {
        return Millimetre();
    }

    return m_precomputedValues[size_t(idx)];
}

void MStyle::set(const Sid t, const PropertyValue& val)
{
    if (t == Sid::NOSTYLE) {
        return;
    }

    const size_t idx = size_t(t);
    m_values[idx] = val;
    if (t == Sid::spatium) {
        precomputeValues();
    } else {
        if (StyleDef::styleValues[idx].valueType() == P_TYPE::SPATIUM) {
            qreal _spatium = value(Sid::spatium).toReal();
            m_precomputedValues[idx] = m_values[idx].value<Spatium>().val() * _spatium;
        }
    }
}

void MStyle::precomputeValues()
{
    qreal _spatium = value(Sid::spatium).toReal();
    for (const StyleDef::StyleValue& t : StyleDef::styleValues) {
        if (t.valueType() == P_TYPE::SPATIUM) {
            m_precomputedValues[t.idx()] = value(t.styleIdx()).value<Spatium>().val() * _spatium;
        }
    }
}

bool MStyle::isDefault(Sid idx) const
{
    return value(idx) == DefaultStyle::resolveStyleDefaults(defaultStyleVersion()).value(idx);
}

void MStyle::setDefaultStyleVersion(const int defaultsVersion)
{
    set(Sid::defaultsVersion, defaultsVersion);
}

int MStyle::defaultStyleVersion() const
{
    return styleI(Sid::defaultsVersion);
}

bool MStyle::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());

    for (const StyleDef::StyleValue& t : StyleDef::styleValues) {
        Sid idx = t.styleIdx();
        if (t.name() == tag) {
            P_TYPE type = t.valueType();
            if (P_TYPE::SPATIUM == type) {
                set(idx, Spatium(e.readElementText().toDouble()));
            } else if (P_TYPE::REAL == type) {
                set(idx, e.readElementText().toDouble());
            } else if (P_TYPE::BOOL == type) {
                set(idx, bool(e.readElementText().toInt()));
            } else if (P_TYPE::INT == type) {
                set(idx, e.readElementText().toInt());
            } else if (P_TYPE::DIRECTION_V == type) {
                set(idx, DirectionV(e.readElementText().toInt()));
            } else if (P_TYPE::STRING == type) {
                set(idx, e.readElementText());
            } else if (P_TYPE::ALIGN == type) {
                Align align = TConv::fromXml(e.readElementText(), Align());
                set(idx, align);
            } else if (P_TYPE::POINT == type) {
                qreal x = e.doubleAttribute("x", 0.0);
                qreal y = e.doubleAttribute("y", 0.0);
                set(idx, PointF(x, y));
                e.readElementText();
            } else if (P_TYPE::SIZE == type) {
                qreal x = e.doubleAttribute("w", 0.0);
                qreal y = e.doubleAttribute("h", 0.0);
                set(idx, SizeF(x, y));
                e.readElementText();
            } else if (P_TYPE::SCALE == type) {
                qreal sx = e.doubleAttribute("w", 0.0);
                qreal sy = e.doubleAttribute("h", 0.0);
                set(idx, ScaleF(sx, sy));
                e.readElementText();
            } else if (P_TYPE::COLOR == type) {
                mu::draw::Color c;
                c.setRed(e.intAttribute("r"));
                c.setGreen(e.intAttribute("g"));
                c.setBlue(e.intAttribute("b"));
                c.setAlpha(e.intAttribute("a", 255));
                set(idx, c);
                e.readElementText();
            } else if (P_TYPE::PLACEMENT_V == type) {
                set(idx, PlacementV(e.readElementText().toInt()));
            } else if (P_TYPE::PLACEMENT_H == type) {
                set(idx, PlacementH(e.readElementText().toInt()));
            } else if (P_TYPE::HOOK_TYPE == type) {
                set(idx, HookType(e.readElementText().toInt()));
            } else {
                ASSERT_X("unhandled type " + QString::number(int(type)));
            }
            return true;
        }
    }
    if (readStyleValCompat(e)) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   readStyleValCompat
//    Read obsolete style values which may appear in files
//    produced by older versions of MuseScore.
//---------------------------------------------------------

bool MStyle::readStyleValCompat(XmlReader& e)
{
    const AsciiStringView tag(e.name());
    if (tag == "tempoOffset") {   // pre-3.0-beta
        const qreal x = e.doubleAttribute("x", 0.0);
        const qreal y = e.doubleAttribute("y", 0.0);
        const PointF val(x, y);
        set(Sid::tempoPosAbove, val);
        set(Sid::tempoPosBelow, val);
        e.readElementText();
        return true;
    }
    if (readTextStyleValCompat(e)) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   readTextStyleValCompat
//    Handle transition from separate bold, underline, strike
//    and italic style properties to the single *FontStyle
//    property set.
//---------------------------------------------------------

bool MStyle::readTextStyleValCompat(XmlReader& e)
{
    static const std::array<std::pair<const char*, FontStyle>, 4> styleNamesEndings { {
        { "FontBold",      FontStyle::Bold },
        { "FontItalic",    FontStyle::Italic },
        { "FontUnderline", FontStyle::Underline },
        { "FontStrike",    FontStyle::Strike }
    } };

    const QString tag(e.name().toQLatin1String());
    FontStyle readFontStyle = FontStyle::Normal;
    QString typeName;
    for (auto& fontStyle : styleNamesEndings) {
        if (tag.endsWith(fontStyle.first)) {
            readFontStyle = fontStyle.second;
            typeName = tag.mid(0, tag.length() - int(strlen(fontStyle.first)));
            break;
        }
    }
    if (readFontStyle == FontStyle::Normal) {
        return false;
    }

    const QString newFontStyleName = typeName + "FontStyle";
    const Sid sid = MStyle::styleIdx(newFontStyleName);
    if (sid == Sid::NOSTYLE) {
        LOGW() << "readFontStyleValCompat: couldn't read text readFontStyle value:" << tag;
        return false;
    }

    const bool readVal = bool(e.readElementText().toInt());
    const PropertyValue& val = value(sid);
    FontStyle newFontStyle = val.isValid() ? FontStyle(val.toInt()) : FontStyle::Normal;
    if (readVal) {
        newFontStyle = newFontStyle + readFontStyle;
    } else {
        newFontStyle = newFontStyle - readFontStyle;
    }

    set(sid, int(newFontStyle));
    return true;
}

bool MStyle::read(IODevice* device, bool ign)
{
    XmlReader e(device);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            QString version = e.attribute("version");
            QStringList sl  = version.split('.');
            int mscVersion  = sl[0].toInt() * 100 + sl[1].toInt();
            if (mscVersion != MSCVERSION && !ign) {
                return false;
            }
            while (e.readNextStartElement()) {
                if (e.name() == "Style") {
                    read(e, nullptr);
                } else {
                    e.unknown();
                }
            }
        }
    }
    return true;
}

bool MStyle::isValid(IODevice* device)
{
    XmlReader e(device);
    while (!e.isError() && e.readNextStartElement()) {
        if (e.name() == "museScore") {
            while (e.readNextStartElement()) {
                if (e.name() == "Style") {
                    return true;
                }
            }
        }
    }
    return false;
}

void MStyle::read(XmlReader& e, compat::ReadChordListHook* readChordListHook)
{
    TRACEFUNC;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "TextStyle") {
            //readTextStyle206(this, e);        // obsolete
            e.readElementText();
        } else if (tag == "ottavaHook") {             // obsolete, for 3.0dev bw. compatibility, should be removed in final release
            qreal y = qAbs(e.readDouble());
            set(Sid::ottavaHookAbove, y);
            set(Sid::ottavaHookBelow, -y);
        } else if (tag == "Spatium") {
            set(Sid::spatium, e.readDouble() * DPMM);
        } else if (tag == "page-layout") {      // obsolete
            compat::readPageFormat206(this, e);
        } else if (tag == "displayInConcertPitch") {
            set(Sid::concertPitch, bool(e.readInt()));
        } else if (tag == "ChordList") {
            if (readChordListHook) {
                readChordListHook->read(e);
            }
        } else if (tag == "lyricsDashMaxLegth") { // pre-3.6 typo
            set(Sid::lyricsDashMaxLength, Spatium(e.readDouble()));
        } else if (tag == "dontHidStavesInFirstSystm") { // pre-3.6.3/4.0 typo
            set(Sid::dontHideStavesInFirstSystem, e.readBool());
        } else if (tag == "beamDistance") { // beamDistance maps to useWideBeams in 4.0
            set(Sid::useWideBeams, e.readDouble() > 0.75);
        } else if (!readProperties(e)) {
            e.unknown();
        }
    }

    if (readChordListHook) {
        readChordListHook->validate();
    }
}

bool MStyle::write(IODevice* device)
{
    XmlWriter xml(device);
    xml.startDocument();
    xml.startElement("museScore", { { "version", MSC_VERSION } });
    save(xml, false);
    xml.endElement();
    return true;
}

void MStyle::save(XmlWriter& xml, bool optimize)
{
    xml.startElement("Style");

    for (const StyleDef::StyleValue& st : StyleDef::styleValues) {
        Sid idx = st.styleIdx();
        if (idx == Sid::spatium) {         // special handling for spatium
            continue;
        }
        if (optimize && isDefault(idx)) {
            continue;
        }
        P_TYPE type = st.valueType();
        if (P_TYPE::SPATIUM == type) {
            xml.tag(st.name(), value(idx).value<Spatium>().val());
        } else if (P_TYPE::DIRECTION_V == type) {
            xml.tag(st.name(), int(value(idx).value<DirectionV>()));
        } else if (P_TYPE::ALIGN == type) {
            Align a = value(idx).value<Align>();
            // Don't write if it's the default value
            if (optimize && a == st.defaultValue().value<Align>()) {
                continue;
            }
            xml.tag(st.name(), TConv::toXml(a));
        } else {
            PropertyValue val = value(idx);
            //! NOTE for compatibility
            if (val.isEnum()) {
                val = val.value<int>();
            }
            xml.tagProperty(st.name(), val);
        }
    }

    xml.tag("Spatium", value(Sid::spatium).toReal() / DPMM);
    xml.endElement();
}

// ====================================================
// Static
// ====================================================

P_TYPE MStyle::valueType(const Sid i)
{
    return StyleDef::styleValues[size_t(i)].valueType();
}

const char* MStyle::valueName(const Sid i)
{
    if (i == Sid::NOSTYLE) {
        static const char* no_style = "no style";
        return no_style;
    }
    return StyleDef::styleValues[size_t(i)].name().ascii();
}

Sid MStyle::styleIdx(const QString& name)
{
    QByteArray ba = name.toLatin1();
    for (const StyleDef::StyleValue& st : StyleDef::styleValues) {
        if (st.name() == ba.constData()) {
            return st.styleIdx();
        }
    }
    return Sid::NOSTYLE;
}
