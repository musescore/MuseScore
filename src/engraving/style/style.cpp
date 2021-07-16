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

//! NOTE It is necessary that `StyleDef::styleTypes` is initialized before static `MStyle` is initialized
#include "styledef.cpp"

#include "compat/pageformat.h"
#include "defaultstyle.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   valueType
//---------------------------------------------------------

const char* MStyle::valueType(const Sid i)
{
    return StyleDef::styleValues[int(i)].valueType();
}

//---------------------------------------------------------
//   value
//---------------------------------------------------------

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

//---------------------------------------------------------
//   valueName
//---------------------------------------------------------

const char* MStyle::valueName(const Sid i)
{
    if (i == Sid::NOSTYLE) {
        return "no style";
    }
    return StyleDef::styleValues[int(i)].name();
}

//---------------------------------------------------------
//   styleIdx
//---------------------------------------------------------

Sid MStyle::styleIdx(const QString& name)
{
    for (StyleDef::StyleValue st : StyleDef::styleValues) {
        if (st.name() == name) {
            return st.styleIdx();
        }
    }
    return Sid::NOSTYLE;
}

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

MStyle::MStyle()
{
    m_defaultStyleVersion = MSCVERSION;
    m_customChordList = false;
}

//---------------------------------------------------------
//   precomputeValues
//---------------------------------------------------------

void MStyle::precomputeValues()
{
    qreal _spatium = value(Sid::spatium).toDouble();
    for (const StyleDef::StyleValue& t : StyleDef::styleValues) {
        if (!strcmp(t.valueType(), "Ms::Spatium")) {
            m_precomputedValues[t.idx()] = value(t.styleIdx()).value<Spatium>().val() * _spatium;
        }
    }
}

//---------------------------------------------------------
//   isDefault
//    caution: custom types need to register comparison operator
//          to make this work
//---------------------------------------------------------

bool MStyle::isDefault(Sid idx) const
{
    return value(idx) == DefaultStyle::resolveStyleDefaults(m_defaultStyleVersion)->value(idx);
}

void MStyle::setDefaultStyleVersion(const int defaultsVersion)
{
    m_defaultStyleVersion = defaultsVersion;
}

//---------------------------------------------------------
//   chordDescription
//---------------------------------------------------------

const ChordDescription* MStyle::chordDescription(int id) const
{
    if (!m_chordList.contains(id)) {
        return 0;
    }
    return &*m_chordList.find(id);
}

//---------------------------------------------------------
//   checkChordList
//---------------------------------------------------------

void MStyle::checkChordList()
{
    // make sure we have a chordlist
    if (!m_chordList.loaded()) {
        qreal emag = value(Sid::chordExtensionMag).toDouble();
        qreal eadjust = value(Sid::chordExtensionAdjust).toDouble();
        qreal mmag = value(Sid::chordModifierMag).toDouble();
        qreal madjust = value(Sid::chordModifierAdjust).toDouble();
        m_chordList.configureAutoAdjust(emag, eadjust, mmag, madjust);
        if (value(Sid::chordsXmlFile).toBool()) {
            m_chordList.read("chords.xml");
        }
        m_chordList.read(value(Sid::chordDescriptionFile).toString());
    }
}

//---------------------------------------------------------
//   set
//---------------------------------------------------------
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

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

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
            } else if (!strcmp("QColor", type)) {
                QColor c;
                c.setRed(e.intAttribute("r"));
                c.setGreen(e.intAttribute("g"));
                c.setBlue(e.intAttribute("b"));
                c.setAlpha(e.intAttribute("a", 255));
                set(idx, c);
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

//---------------------------------------------------------
//   load
//---------------------------------------------------------

bool MStyle::load(QFile* qf, bool ign)
{
    XmlReader e(qf);
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
                    load(e);
                } else {
                    e.unknown();
                }
            }
        }
    }
    return true;
}

void MStyle::load(XmlReader& e)
{
    TRACEFUNC;
    QString oldChordDescriptionFile = value(Sid::chordDescriptionFile).toString();
    bool chordListTag = false;
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
            m_chordList.unload();
            m_chordList.read(e);
            m_customChordList = true;
            chordListTag = true;
        } else if (tag == "lyricsDashMaxLegth") { // pre-3.6 typo
            set(Sid::lyricsDashMaxLength, e.readDouble());
        } else if (tag == "dontHidStavesInFirstSystm") { // pre-3.6.3/4.0 typo
            set(Sid::dontHideStavesInFirstSystem, e.readBool());
        } else if (!readProperties(e)) {
            e.unknown();
        }
    }

    // if we just specified a new chord description file
    // and didn't encounter a ChordList tag
    // then load the chord description file

    QString newChordDescriptionFile = value(Sid::chordDescriptionFile).toString();
    if (newChordDescriptionFile != oldChordDescriptionFile && !chordListTag) {
        if (!newChordDescriptionFile.startsWith("chords_") && value(Sid::chordStyle).toString() == "std") {
            // should not normally happen,
            // but treat as "old" (114) score just in case
            set(Sid::chordStyle, QVariant(QString("custom")));
            set(Sid::chordsXmlFile, QVariant(true));
            qDebug("StyleData::load: custom chord description file %s with chordStyle == std", qPrintable(newChordDescriptionFile));
        }
        if (value(Sid::chordStyle).toString() == "custom") {
            m_customChordList = true;
        } else {
            m_customChordList = false;
        }
        m_chordList.unload();
    }

    if (!chordListTag) {
        checkChordList();
    }
}

void MStyle::applyNewDefaults(const MStyle& other, const int defaultsVersion)
{
    m_defaultStyleVersion = defaultsVersion;

    for (StyleDef::StyleValue st : StyleDef::styleValues) {
        if (isDefault(st.styleIdx())) {
            st._defaultValue = other.value(st.styleIdx());
            m_values.at(st.idx()) = other.value(st.styleIdx());
        }
    }
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

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
    if (m_customChordList && !m_chordList.empty()) {
        xml.stag("ChordList");
        m_chordList.write(xml);
        xml.etag();
    }
    xml.tag("Spatium", value(Sid::spatium).toDouble() / DPMM);
    xml.etag();
}
}
