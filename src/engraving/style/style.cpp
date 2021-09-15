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
#include "compat/chordlist.h"
#include "io/xml.h"

#include "libmscore/mscore.h"

#include "defaultstyle.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace Ms;

MStyle::MStyle()
{
    m_defaultStyleVersion = MSCVERSION;
}

const QVariant& MStyle::value(Sid idx) const
{
    const QVariant& val = m_values[int(idx)];
    if (val.isValid()) {
        return val;
    }

    return StyleDef::styleValues[int(idx)].defaultValue();
}

qreal MStyle::pvalue(Sid idx) const
{
    return m_precomputedValues[int(idx)];
}

void MStyle::set(Sid idx, const mu::PointF& v)
{
    set(idx, QVariant::fromValue(v));
}

void MStyle::set(const Sid t, const QVariant& val)
{
    const int idx = int(t);
    m_values[idx] = val;
    if (t == Sid::spatium) {
        precomputeValues();
    } else {
        if (!strcmp(StyleDef::styleValues[idx].valueType(), "Ms::Spatium")) {
            qreal _spatium = value(Sid::spatium).toDouble();
            m_precomputedValues[idx] = m_values[idx].value<Spatium>().val() * _spatium;
        }
    }
}

void MStyle::precomputeValues()
{
    qreal _spatium = value(Sid::spatium).toDouble();
    for (const StyleDef::StyleValue& t : StyleDef::styleValues) {
        if (!strcmp(t.valueType(), "Ms::Spatium")) {
            m_precomputedValues[t.idx()] = value(t.styleIdx()).value<Spatium>().val() * _spatium;
        }
    }
}

bool MStyle::isDefault(Sid idx) const
{
    return value(idx) == DefaultStyle::resolveStyleDefaults(m_defaultStyleVersion).value(idx);
}

void MStyle::setDefaultStyleVersion(const int defaultsVersion)
{
    m_defaultStyleVersion = defaultsVersion;
}

int MStyle::defaultStyleVersion() const
{
    return m_defaultStyleVersion;
}

bool MStyle::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());

    for (const StyleDef::StyleValue& t : StyleDef::styleValues) {
        Sid idx = t.styleIdx();
        if (t.name() == tag) {
            const char* type = t.valueType();
            if (!strcmp("Ms::Spatium", type)) {
                set(idx, Spatium(e.readElementText().toDouble()));
            } else if (!strcmp("double", type)) {
                set(idx, QVariant(e.readElementText().toDouble()));
            } else if (!strcmp("bool", type)) {
                set(idx, QVariant(bool(e.readElementText().toInt())));
            } else if (!strcmp("int", type)) {
                set(idx, QVariant(e.readElementText().toInt()));
            } else if (!strcmp("Ms::Direction", type)) {
                set(idx, QVariant::fromValue(Direction(e.readElementText().toInt())));
            } else if (!strcmp("QString", type)) {
                set(idx, QVariant(e.readElementText()));
            } else if (!strcmp("Ms::Align", type)) {
                QStringList sl = e.readElementText().split(',');
                if (sl.size() != 2) {
                    qDebug("bad align text <%s>", qPrintable(e.readElementText()));
                    return true;
                }
                Align align = Align::LEFT;
                if (sl[0] == "center") {
                    align = align | Align::HCENTER;
                } else if (sl[0] == "right") {
                    align = align | Align::RIGHT;
                } else if (sl[0] == "left") {
                } else {
                    qDebug("bad align text <%s>", qPrintable(sl[0]));
                    return true;
                }
                if (sl[1] == "center") {
                    align = align | Align::VCENTER;
                } else if (sl[1] == "bottom") {
                    align = align | Align::BOTTOM;
                } else if (sl[1] == "baseline") {
                    align = align | Align::BASELINE;
                } else if (sl[1] == "top") {
                } else {
                    qDebug("bad align text <%s>", qPrintable(sl[1]));
                    return true;
                }
                set(idx, QVariant::fromValue(align));
            } else if (!strcmp("QPointF", type)) {
                qreal x = e.doubleAttribute("x", 0.0);
                qreal y = e.doubleAttribute("y", 0.0);
                set(idx, PointF(x, y));
                e.readElementText();
            } else if (!strcmp("mu::PointF", type)) {
                qreal x = e.doubleAttribute("x", 0.0);
                qreal y = e.doubleAttribute("y", 0.0);
                set(idx, PointF(x, y));
                e.readElementText();
            } else if (!strcmp("QSizeF", type)) {
                qreal x = e.doubleAttribute("w", 0.0);
                qreal y = e.doubleAttribute("h", 0.0);
                set(idx, SizeF(x, y));
                e.readElementText();
            } else if (!strcmp("mu::SizeF", type)) {
                qreal x = e.doubleAttribute("w", 0.0);
                qreal y = e.doubleAttribute("h", 0.0);
                set(idx, SizeF(x, y));
                e.readElementText();
            } else if (!strcmp("QColor", type) || !strcmp("mu::draw::Color", type)) {
                mu::draw::Color c;
                c.setRed(e.intAttribute("r"));
                c.setGreen(e.intAttribute("g"));
                c.setBlue(e.intAttribute("b"));
                c.setAlpha(e.intAttribute("a", 255));
                set(idx, QVariant::fromValue<mu::draw::Color>(c));
                e.readElementText();
            } else {
                qFatal("unhandled type %s", type);
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
    const QStringRef tag(e.name());
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
//    Handle transition from separate bold, underline and
//    italic style properties to the single *FontStyle
//    property set.
//---------------------------------------------------------

bool MStyle::readTextStyleValCompat(XmlReader& e)
{
    static const std::array<std::pair<const char*, FontStyle>, 3> styleNamesEndings { {
        { "FontBold",      FontStyle::Bold },
        { "FontItalic",    FontStyle::Italic },
        { "FontUnderline", FontStyle::Underline }
    } };

    const QStringRef tag(e.name());
    FontStyle readFontStyle = FontStyle::Normal;
    QStringRef typeName;
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

    const QString newFontStyleName = typeName.toString() + "FontStyle";
    const Sid sid = MStyle::styleIdx(newFontStyleName);
    if (sid == Sid::NOSTYLE) {
        qWarning() << "readFontStyleValCompat: couldn't read text readFontStyle value:" << tag;
        return false;
    }

    const bool readVal = bool(e.readElementText().toInt());
    const QVariant val = value(sid);
    FontStyle newFontStyle = (val == QVariant()) ? FontStyle::Normal : FontStyle(val.toInt());
    if (readVal) {
        newFontStyle = newFontStyle + readFontStyle;
    } else {
        newFontStyle = newFontStyle - readFontStyle;
    }

    set(sid, int(newFontStyle));
    return true;
}

bool MStyle::read(QIODevice* device, bool ign)
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

bool MStyle::isValid(QIODevice* device)
{
    XmlReader e(device);
    while (e.error() == XmlReader::Error::NoError && e.readNextStartElement()) {
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
        const QStringRef& tag(e.name());

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
            set(Sid::concertPitch, QVariant(bool(e.readInt())));
        } else if (tag == "ChordList") {
            if (readChordListHook) {
                readChordListHook->read(e);
            }
        } else if (tag == "lyricsDashMaxLegth") { // pre-3.6 typo
            set(Sid::lyricsDashMaxLength, e.readDouble());
        } else if (tag == "dontHidStavesInFirstSystm") { // pre-3.6.3/4.0 typo
            set(Sid::dontHideStavesInFirstSystem, e.readBool());
        } else if (!readProperties(e)) {
            e.unknown();
        }
    }

    if (readChordListHook) {
        readChordListHook->validate();
    }
}

bool MStyle::write(QIODevice* device)
{
    XmlWriter xml(nullptr, device);
    xml.header();
    xml.stag("museScore version=\"" MSC_VERSION "\"");
    save(xml, false);
    xml.etag();
    return true;
}

void MStyle::save(XmlWriter& xml, bool optimize)
{
    xml.stag("Style");

    for (const StyleDef::StyleValue& st : StyleDef::styleValues) {
        Sid idx = st.styleIdx();
        if (idx == Sid::spatium) {         // special handling for spatium
            continue;
        }
        if (optimize && isDefault(idx)) {
            continue;
        }
        const char* type = st.valueType();
        if (!strcmp("Ms::Spatium", type)) {
            xml.tag(st.name(), value(idx).value<Spatium>().val());
        } else if (!strcmp("Ms::Direction", type)) {
            xml.tag(st.name(), value(idx).toInt());
        } else if (!strcmp("Ms::Align", type)) {
            Align a = Align(value(idx).toInt());
            // Don't write if it's the default value
            if (optimize && a == Align(st.defaultValue().toInt())) {
                continue;
            }
            QString horizontal = "left";
            QString vertical = "top";
            if (a & Align::HCENTER) {
                horizontal = "center";
            } else if (a & Align::RIGHT) {
                horizontal = "right";
            }

            if (a & Align::VCENTER) {
                vertical = "center";
            } else if (a & Align::BOTTOM) {
                vertical = "bottom";
            } else if (a & Align::BASELINE) {
                vertical = "baseline";
            }

            xml.tag(st.name(), horizontal + "," + vertical);
        } else {
            xml.tag(st.name(), value(idx));
        }
    }

    xml.tag("Spatium", value(Sid::spatium).toDouble() / DPMM);
    xml.etag();
}

// ====================================================
// Static
// ====================================================

const char* MStyle::valueType(const Sid i)
{
    return StyleDef::styleValues[int(i)].valueType();
}

const char* MStyle::valueName(const Sid i)
{
    if (i == Sid::NOSTYLE) {
        static const char* no_style = "no style";
        return no_style;
    }
    return StyleDef::styleValues[int(i)].name();
}

Sid MStyle::styleIdx(const QString& name)
{
    for (StyleDef::StyleValue st : StyleDef::styleValues) {
        if (st.name() == name) {
            return st.styleIdx();
        }
    }
    return Sid::NOSTYLE;
}
