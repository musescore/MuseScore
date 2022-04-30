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

#include "stafftype.h"

#include <QFile>
#include <QFileInfo>

#include "draw/fontmetrics.h"
#include "draw/pen.h"
#include "rw/xml.h"
#include "types/typesconv.h"

#include "chord.h"
#include "measure.h"
#include "mscore.h"
#include "navigate.h"
#include "staff.h"
#include "score.h"

using namespace mu;
using namespace mu::engraving;

#define TAB_DEFAULT_LINE_SP   (1.5)
#define TAB_RESTSYMBDISPL     2.0

namespace Ms {
//---------------------------------------------------------
//   StaffTypeTablature
//---------------------------------------------------------

#define TAB_DEFAULT_DUR_YOFFS (-1.0)

std::vector<TablatureFretFont> StaffType::_fretFonts = {};
std::vector<TablatureDurationFont> StaffType::_durationFonts = {};

const char StaffType::groupNames[STAFF_GROUP_MAX][STAFF_GROUP_NAME_MAX_LENGTH] = {
    QT_TRANSLATE_NOOP("Staff type group name", "Standard"),
    QT_TRANSLATE_NOOP("Staff type group name", "Percussion"),
    QT_TRANSLATE_NOOP("Staff type group name", "Tablature")
};

const QString StaffType::fileGroupNames[STAFF_GROUP_MAX] = { "pitched", "percussion", "tablature" };

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

StaffType::StaffType()
{
    // set reasonable defaults for type-specific members */
    _symRepeat = TablatureSymbolRepeat::NEVER;
    setDurationFontName(_durationFonts[0].displayName);
    setFretFontName(_fretFonts[0].displayName);
}

StaffType::StaffType(StaffGroup sg, const QString& xml, const QString& name, int lines, int stpOff, qreal lineDist,
                     bool genClef, bool showBarLines, bool stemless, bool genTimeSig, bool genKeySig, bool showLedgerLines, bool invisible,
                     const mu::draw::Color& color)
    : _group(sg), _xmlName(xml), _name(name),
    _invisible(invisible),
    _color(color),
    _lines(lines),
    _stepOffset(stpOff),
    _lineDistance(Spatium(lineDist)),
    _showBarlines(showBarLines),
    _showLedgerLines(showLedgerLines),
    _stemless(stemless),
    _genClef(genClef),
    _genTimesig(genTimeSig),
    _genKeysig(genKeySig)
{
}

StaffType::StaffType(StaffGroup sg, const QString& xml, const QString& name, int lines, int stpOff, qreal lineDist,
                     bool genClef,
                     bool showBarLines, bool stemless, bool genTimesig, bool invisible, const mu::draw::Color& color,
                     const QString& durFontName, qreal durFontSize, qreal durFontUserY, qreal genDur,
                     const QString& fretFontName, qreal fretFontSize, qreal fretFontUserY,
                     TablatureSymbolRepeat symRepeat, bool linesThrough, TablatureMinimStyle minimStyle, bool onLines,
                     bool showRests, bool stemsDown, bool stemThrough, bool upsideDown, bool showTabFingering, bool useNumbers,
                     bool showBackTied)
{
    Q_UNUSED(invisible);
    Q_UNUSED(color);
    _group   = sg;
    _xmlName = xml;
    _name    = name;
    setLines(lines);
    setStepOffset(stpOff);
    setLineDistance(Spatium(lineDist));
    setGenClef(genClef);
    setShowBarlines(showBarLines);
    setStemless(stemless);
    setGenTimesig(genTimesig);
    setGenKeysig(sg != StaffGroup::TAB);
    setDurationFontName(durFontName);
    setDurationFontSize(durFontSize);
    setDurationFontUserY(durFontUserY);
    setGenDurations(genDur);
    setFretFontName(fretFontName);
    setFretFontSize(fretFontSize);
    setFretFontUserY(fretFontUserY);
    setSymbolRepeat(symRepeat);
    setLinesThrough(linesThrough);
    setMinimStyle(minimStyle);
    setOnLines(onLines);
    setShowRests(showRests);
    setStemsDown(stemsDown);
    setStemsThrough(stemThrough);
    setUpsideDown(upsideDown);
    setShowTabFingering(showTabFingering);
    setUseNumbers(useNumbers);
    setShowBackTied(showBackTied);
}

//---------------------------------------------------------
//   groupName
//---------------------------------------------------------

const char* StaffType::groupName() const
{
    return groupName(_group);
}

const char* StaffType::groupName(StaffGroup r)
{
    if (r < StaffGroup::STANDARD || (int)r >= STAFF_GROUP_MAX) {
        r = StaffGroup::STANDARD;
    }
    return groupNames[(int)r];
}

int StaffType::middleLine() const
{
    return _lines - 1 - _stepOffset;
}

int StaffType::bottomLine() const
{
    return (_lines - 1) * 2;
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool StaffType::operator==(const StaffType& st) const
{
    bool equal = true;

    equal &= (_group == st._group);
    equal &= (_xmlName == st._xmlName);
    equal &= (_name == st._name);
    equal &= (_userMag == st._userMag);
    equal &= (_yoffset == st._yoffset);
    equal &= (_small == st._small);
    equal &= (_invisible == st._invisible);
    equal &= (_color == st._color);
    equal &= (_lines == st._lines);
    equal &= (_stepOffset == st._stepOffset);
    equal &= (_lineDistance == st._lineDistance);
    equal &= (_showBarlines == st._showBarlines);
    equal &= (_showLedgerLines == st._showLedgerLines);
    equal &= (_stemless == st._stemless);
    equal &= (_genClef == st._genClef);
    equal &= (_genTimesig == st._genTimesig);
    equal &= (_genKeysig == st._genKeysig);
    equal &= (_noteHeadScheme == st._noteHeadScheme);
    equal &= (_durationFontSize == st._durationFontSize);
    equal &= (_durationFontUserY == st._durationFontUserY);
    equal &= (_fretFontSize == st._fretFontSize);
    equal &= (_fretFontUserY == st._fretFontUserY);
    equal &= (_genDurations == st._genDurations);
    equal &= (_linesThrough == st._linesThrough);
    equal &= (_minimStyle == st._minimStyle);
    equal &= (_symRepeat == st._symRepeat);
    equal &= (_onLines == st._onLines);
    equal &= (_showRests == st._showRests);
    equal &= (_stemsDown == st._stemsDown);
    equal &= (_stemsThrough == st._stemsThrough);
    equal &= (_upsideDown == st._upsideDown);
    equal &= (_showTabFingering == st._showTabFingering);
    equal &= (_useNumbers == st._useNumbers);
    equal &= (_showBackTied == st._showBackTied);
    equal &= (_durationBoxH == st._durationBoxH);
    equal &= (_durationBoxY == st._durationBoxY);
    equal &= (_durationFont == st._durationFont);
    equal &= (_durationFontIdx == st._durationFontIdx);
    equal &= (_durationYOffset == st._durationYOffset);
    equal &= (_durationGridYOffset == st._durationGridYOffset);
    equal &= (_durationMetricsValid == st._durationMetricsValid);
    equal &= (_fretBoxH == st._fretBoxH);
    equal &= (_fretBoxY == st._fretBoxY);
    equal &= (_fretFont == st._fretFont);
    equal &= (_fretFontIdx == st._fretFontIdx);
    equal &= (_fretYOffset == st._fretYOffset);
    equal &= (_fretMetricsValid == st._fretMetricsValid);
    equal &= (_refDPI == st._refDPI);

    return equal;
}

StaffTypes StaffType::type() const
{
    static const std::map<QString, StaffTypes> xmlNameToType {
        { "stdNormal", StaffTypes::STANDARD },

        { "perc1Line", StaffTypes::PERC_1LINE },
        { "perc3Line", StaffTypes::PERC_3LINE },
        { "perc5Line", StaffTypes::PERC_5LINE },

        { "tab4StrSimple", StaffTypes::TAB_4SIMPLE },
        { "tab4StrCommon", StaffTypes::TAB_4COMMON },
        { "tab4StrFull", StaffTypes::TAB_4FULL },

        { "tab5StrSimple", StaffTypes::TAB_5SIMPLE },
        { "tab5StrCommon", StaffTypes::TAB_5COMMON },
        { "tab5StrFull", StaffTypes::TAB_5FULL },

        { "tab6StrSimple", StaffTypes::TAB_6SIMPLE },
        { "tab6StrCommon", StaffTypes::TAB_6COMMON },
        { "tab6StrFull", StaffTypes::TAB_6FULL },

        { "tabUkulele", StaffTypes::TAB_UKULELE },
        { "tabBalajka", StaffTypes::TAB_BALALAJKA },
        { "tabDulcimer", StaffTypes::TAB_DULCIMER },

        { "tab6StrItalian", StaffTypes::TAB_ITALIAN },
        { "tab6StrFrench", StaffTypes::TAB_FRENCH },

        { "tab7StrCommon", StaffTypes::TAB_7COMMON },
        { "tab8StrCommon", StaffTypes::TAB_8COMMON },
    };

    return mu::value(xmlNameToType, _xmlName, StaffTypes::STANDARD);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffType::write(XmlWriter& xml) const
{
    xml.startObject(QString("StaffType group=\"%1\"").arg(fileGroupNames[(int)_group]));
    if (!_xmlName.isEmpty()) {
        xml.tag("name", _xmlName);
    }
    if (_lines != 5) {
        xml.tag("lines", _lines);
    }
    if (_lineDistance.val() != 1.0) {
        xml.tag("lineDistance", _lineDistance.val());
    }
    if (_yoffset.val() != 0.0) {
        xml.tag("yoffset", _yoffset.val());
    }
    if (_userMag != 1.0) {
        xml.tag("mag", _userMag);
    }
    if (_small) {
        xml.tag("small", _small);
    }
    if (_stepOffset) {
        xml.tag("stepOffset", _stepOffset);
    }
    if (!_genClef) {
        xml.tag("clef", _genClef);
    }
    if (_stemless) {
        xml.tag("slashStyle", _stemless);     // for backwards compatibility
        xml.tag("stemless", _stemless);
    }
    if (!_showBarlines) {
        xml.tag("barlines", _showBarlines);
    }
    if (!_genTimesig) {
        xml.tag("timesig", _genTimesig);
    }
    if (_invisible) {
        xml.tag("invisible", _invisible);
    }
    if (_color != engravingConfiguration()->defaultColor()) {
        xml.tag("color", _color.toString().c_str());
    }
    if (_group == StaffGroup::STANDARD) {
        xml.tag("noteheadScheme", TConv::toXml(_noteHeadScheme), TConv::toXml(NoteHeadScheme::HEAD_NORMAL));
    }
    if (_group == StaffGroup::STANDARD || _group == StaffGroup::PERCUSSION) {
        if (!_genKeysig) {
            xml.tag("keysig", _genKeysig);
        }
        if (!_showLedgerLines) {
            xml.tag("ledgerlines", _showLedgerLines);
        }
    } else {
        xml.tag("durations",        _genDurations);
        xml.tag("durationFontName", _durationFonts[_durationFontIdx].displayName);     // write font names anyway for backward compatibility
        xml.tag("durationFontSize", _durationFontSize);
        xml.tag("durationFontY",    _durationFontUserY);
        xml.tag("fretFontName",     _fretFonts[_fretFontIdx].displayName);
        xml.tag("fretFontSize",     _fretFontSize);
        xml.tag("fretFontY",        _fretFontUserY);
        if (_symRepeat != TablatureSymbolRepeat::NEVER) {
            xml.tag("symbolRepeat", int(_symRepeat));
        }
        xml.tag("linesThrough",     _linesThrough);
        xml.tag("minimStyle",       int(_minimStyle));
        xml.tag("onLines",          _onLines);
        xml.tag("showRests",        _showRests);
        xml.tag("stemsDown",        _stemsDown);
        xml.tag("stemsThrough",     _stemsThrough);
        xml.tag("upsideDown",       _upsideDown);
        xml.tag("showTabFingering", _showTabFingering, false);
        xml.tag("useNumbers",       _useNumbers);
        // only output "showBackTied" if different from !"stemless"
        // to match the behaviour in 2.0.2 scores (or older)
        if (_showBackTied != !_stemless) {
            xml.tag("showBackTied",  _showBackTied);
        }
    }
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffType::read(XmlReader& e)
{
    QString group = e.attribute("group", fileGroupNames[(int)StaffGroup::STANDARD]);
    if (group == fileGroupNames[(int)StaffGroup::TAB]) {
        _group = StaffGroup::TAB;
    } else if (group == fileGroupNames[(int)StaffGroup::PERCUSSION]) {
        _group = StaffGroup::PERCUSSION;
    } else if (group == fileGroupNames[(int)StaffGroup::STANDARD]) {
        _group = StaffGroup::STANDARD;
    } else {
        qDebug("StaffType::read: unknown group: %s", qPrintable(group));
        _group = StaffGroup::STANDARD;
    }

    if (_group == StaffGroup::TAB) {
        setGenKeysig(false);
    }

    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "name") {
            setXmlName(e.readElementText());
        } else if (tag == "lines") {
            setLines(e.readInt());
        } else if (tag == "lineDistance") {
            setLineDistance(Spatium(e.readDouble()));
        } else if (tag == "yoffset") {
            _yoffset = Spatium(e.readDouble());
        } else if (tag == "mag") {
            _userMag = e.readDouble();
        } else if (tag == "small") {
            _small = e.readBool();
        } else if (tag == "stepOffset") {
            _stepOffset = e.readInt();
        } else if (tag == "clef") {
            setGenClef(e.readInt());
        } else if ((tag == "slashStyle") || (tag == "stemless")) {
            bool val = e.readInt() != 0;
            setStemless(val);
            setShowBackTied(!val);        // for compatibility with 2.0.2 scores where this prop
        }                                 // was lacking and controlled by "slashStyle" instead
        else if (tag == "barlines") {
            setShowBarlines(e.readInt());
        } else if (tag == "timesig") {
            setGenTimesig(e.readInt());
        } else if (tag == "noteheadScheme") {
            setNoteHeadScheme(TConv::fromXml(e.readElementText(), NoteHeadScheme::HEAD_NORMAL));
        } else if (tag == "keysig") {
            _genKeysig = e.readInt();
        } else if (tag == "ledgerlines") {
            _showLedgerLines = e.readInt();
        } else if (tag == "invisible") {
            _invisible = e.readInt();
        } else if (tag == "color") {
            _color = e.readColor();
        } else if (tag == "durations") {
            setGenDurations(e.readBool());
        } else if (tag == "durationFontName") {
            setDurationFontName(e.readElementText());
        } else if (tag == "durationFontSize") {
            setDurationFontSize(e.readDouble());
        } else if (tag == "durationFontY") {
            setDurationFontUserY(e.readDouble());
        } else if (tag == "fretFontName") {
            setFretFontName(e.readElementText());
        } else if (tag == "fretFontSize") {
            setFretFontSize(e.readDouble());
        } else if (tag == "fretFontY") {
            setFretFontUserY(e.readDouble());
        } else if (tag == "symbolRepeat") {
            setSymbolRepeat((TablatureSymbolRepeat)e.readInt());
        } else if (tag == "linesThrough") {
            setLinesThrough(e.readBool());
        } else if (tag == "minimStyle") {
            setMinimStyle((TablatureMinimStyle)e.readInt());
        } else if (tag == "onLines") {
            setOnLines(e.readBool());
        } else if (tag == "showRests") {
            setShowRests(e.readBool());
        } else if (tag == "stemsDown") {
            setStemsDown(e.readBool());
        } else if (tag == "stemsThrough") {
            setStemsThrough(e.readBool());
        } else if (tag == "upsideDown") {
            setUpsideDown(e.readBool());
        } else if (tag == "showTabFingering") {
            setShowTabFingering(e.readBool());
        } else if (tag == "useNumbers") {
            setUseNumbers(e.readBool());
        } else if (tag == "showBackTied") {           // must be after reading "slashStyle"/"stemless" prop, as in older
            setShowBackTied(e.readBool());            // scores, this prop was lacking and controlled by "slashStyle"
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   doty1
//    get y dot position of first repeat barline dot
//---------------------------------------------------------

qreal StaffType::doty1() const
{
    return _lineDistance.val() * (static_cast<qreal>((_lines - 1) / 2) - 0.5);
}

//---------------------------------------------------------
//   doty2
//    get y dot position of second repeat barline dot
//---------------------------------------------------------

qreal StaffType::doty2() const
{
    return _lineDistance.val() * (static_cast<qreal>(_lines / 2) + 0.5);
}

//---------------------------------------------------------
//   setOnLines
//---------------------------------------------------------

void StaffType::setOnLines(bool val)
{
    _onLines = val;
    _durationMetricsValid = _fretMetricsValid = false;
}

//---------------------------------------------------------
//   setDurationMetrics
//    checks whether the internally computed metrics are is still valid and re-computes them, if not
//---------------------------------------------------------

void StaffType::setDurationMetrics() const
{
    if (_durationMetricsValid && _refDPI == DPI) {           // metrics are still valid
        return;
    }

// FontMetrics returns results unreliably rounded to integral pixels;
// use a scaled up font and then scale computed values down
    mu::draw::Font font(durationFont());
    font.setPointSizeF(_durationFontSize);
    mu::draw::FontMetrics fm(font);
    QString txt(_durationFonts[_durationFontIdx].displayValue, int(TabVal::NUM_OF));
    RectF bb(fm.tightBoundingRect(txt));
    // raise symbols by a default margin and, if marks are above lines, by half the line distance
    // (converted from spatium units to raster units)
    _durationGridYOffset = (TAB_DEFAULT_DUR_YOFFS - (_onLines ? 0.0 : lineDistance().val() * 0.5)) * SPATIUM20;
    // this is the bottomest point of any duration sign
    _durationYOffset        = _durationGridYOffset;
    // move symbols so that the lowest margin 'sits' on the base line:
    // move down by the whole part above (negative) the base line
    // ( -bb.y() ) then up by the whole height ( -bb.height() )
    _durationYOffset        -= (bb.height() + bb.y()) / 100.0;
    _durationBoxH           = bb.height() / 100.0;
    _durationBoxY           = _durationGridYOffset - bb.height() / 100.0;
    // keep track of the conditions under which metrics have been computed
    _refDPI = DPI;
    _durationMetricsValid = true;
}

void StaffType::setFretMetrics() const
{
    if (_fretMetricsValid && _refDPI == DPI) {
        return;
    }

    mu::draw::FontMetrics fm(fretFont());
    RectF bb;
    // compute vertical displacement
    if (_useNumbers) {
        // compute total height of used characters
        QString txt = QString();
        for (int idx = 0; idx < 10; idx++) {    // use only first 10 digits
            txt.append(_fretFonts[_fretFontIdx].displayDigit[idx]);
        }
        bb = fm.tightBoundingRect(txt);
        // for numbers: centre on '0': move down by the whole part above (negative)
        // the base line ( -bb.y() ) then up by half the whole height ( -bb.height()/2 )
        RectF bx(fm.tightBoundingRect(_fretFonts[_fretFontIdx].displayDigit[0]));
        _fretYOffset = -(bx.y() + bx.height() / 2.0);
        // _fretYOffset = -(bb.y() + bb.height()/2.0);  // <- using bbox of all chars
    } else {
        // compute total height of used characters
        QString txt(_fretFonts[_fretFontIdx].displayLetter, NUM_OF_LETTERFRETS);
        bb = fm.tightBoundingRect(txt);
        // for letters: centre on the 'a' ascender, by moving down half of the part above the base line in bx
        RectF bx(fm.tightBoundingRect(_fretFonts[_fretFontIdx].displayLetter[0]));
        _fretYOffset = -bx.y() / 2.0;
    }
    // if on string, we are done; if between strings, raise by half line distance
    if (!_onLines) {
        _fretYOffset -= lineDistance().val() * SPATIUM20 * 0.5;
    }

    // from _fretYOffset, compute _fretBoxH and _fretBoxY
    _fretBoxH = bb.height();
    _fretBoxY = bb.y() + _fretYOffset;

    // keep track of the conditions under which metrics have been computed
    _refDPI = DPI;
    _fretMetricsValid = true;
}

//---------------------------------------------------------
//   setDurationFontName / setFretFontName
//---------------------------------------------------------

void StaffType::setDurationFontName(const QString& name)
{
    size_t idx;
    for (idx = 0; idx < _durationFonts.size(); idx++) {
        if (_durationFonts[idx].displayName == name) {
            break;
        }
    }
    if (idx >= _durationFonts.size()) {
        idx = 0;              // if name not found, use first font
    }
    _durationFont.setFamily(_durationFonts[idx].family);
    _durationFontIdx = idx;
    _durationMetricsValid = false;
}

void StaffType::setFretFontName(const QString& name)
{
    size_t idx;
    QString locName = name;
    // convert old names for two built-in fonts which have changed of name
    if (name == "MuseScore Tab Late Renaiss") {
        locName = "MuseScore Phalèse";
    }
    for (idx = 0; idx < _fretFonts.size(); idx++) {
        if (_fretFonts[idx].displayName == locName) {
            break;
        }
    }
    if (idx >= _fretFonts.size()) {
        idx = 0;              // if name not found, use first font
    }
    _fretFont.setFamily(_fretFonts[idx].family);
    _fretFontIdx = idx;
    _fretMetricsValid = false;
}

//---------------------------------------------------------
//   durationBoxH / durationBoxY
//---------------------------------------------------------

qreal StaffType::durationBoxH() const
{
    if (!_genDurations && !_stemless) {
        return 0.0;
    }
    setDurationMetrics();
    return _durationBoxH;
}

qreal StaffType::durationBoxY() const
{
    if (!_genDurations && !_stemless) {
        return 0.0;
    }
    setDurationMetrics();
    return _durationBoxY + _durationFontUserY * SPATIUM20;
}

//---------------------------------------------------------
//   setDurationFontSize / setFretFontSize
//---------------------------------------------------------

void StaffType::setDurationFontSize(qreal val)
{
    _durationFontSize = val;
    _durationFont.setPointSizeF(val);
    _durationMetricsValid = false;
}

void StaffType::setFretFontSize(qreal val)
{
    _fretFontSize = val;
    _fretFont.setPointSizeF(val);
    _fretMetricsValid = false;
}

//---------------------------------------------------------
//   chordRestStemPosY / chordStemPos / chordStemPosBeam / chordStemLength
//
//    computes the stem data for the given chord, according to TAB settings
//    NOTE: unit: spatium, position: relative to chord (DIFFERENT from Chord own functions)
//
//   chordRestStemPosY
//          returns the vertical position of stem start point
//---------------------------------------------------------

qreal StaffType::chordRestStemPosY(const ChordRest* chordRest) const
{
    if (stemThrough()) {            // does not make sense for "stems through staves" setting; just return top line vert. position
        return 0.0;
    }

    // if stems beside staff, position are fixed, but take into account delta for half notes
    qreal delta                                 // displacement for half note stems (if used)
        =   // if half notes have not a short stem OR not a half note => 0
          (minimStyle() != TablatureMinimStyle::SHORTER || chordRest->durationType().type() != DurationType::V_HALF)
          ? 0.0
          :       // if stem is up, displace of half stem length down (positive)
                  // if stem is down, displace of half stem length up (negative)
          (chordRest->up()
           ? -STAFFTYPE_TAB_DEFAULTSTEMLEN_UP : STAFFTYPE_TAB_DEFAULTSTEMLEN_DN) * 0.5;
    // if fret marks above lines and chordRest is up, move half a line distance up
    if (!onLines() && chordRest->up()) {
        delta -= _lineDistance.val() * 0.5;
    }
    qreal y = (chordRest->up() ? STAFFTYPE_TAB_DEFAULTSTEMPOSY_UP : (_lines - 1) * _lineDistance.val() + STAFFTYPE_TAB_DEFAULTSTEMPOSY_DN)
              + delta;
    return y;
}

//---------------------------------------------------------
//   chordStemPos
//    return position of note at other side of beam
//---------------------------------------------------------

PointF StaffType::chordStemPos(const Chord* chord) const
{
    qreal y;
    if (stemThrough()) {
        // if stems are through staff, stem goes from fartest note string
        y = (chord->up() ? chord->downString() : chord->upString()) * _lineDistance.val();
    } else {
        // if stems are beside staff, stem start point has a fixed vertical position,
        // according to TAB parameters and stem up/down
        y = chordRestStemPosY(chord);
    }
    return PointF(chordStemPosX(chord), y);
}

//---------------------------------------------------------
//   chordStemPosBeam
//          return position of note at beam side of stem
//---------------------------------------------------------

PointF StaffType::chordStemPosBeam(const Chord* chord) const
{
    qreal y = (stemsDown() ? chord->downString() : chord->upString()) * _lineDistance.val();

    return PointF(chordStemPosX(chord), y);
}

//---------------------------------------------------------
//   chordStemLength
//          return length of stem
//---------------------------------------------------------

qreal StaffType::chordStemLength(const Chord* chord) const
{
    qreal stemLen;
    // if stems are through staff, length should be computed by relevant chord algorithm;
    // here, just return default length (= 1 'octave' = 3.5 line spaces)
    if (stemThrough()) {
        return STAFFTYPE_TAB_DEFAULTSTEMLEN_THRU * _lineDistance.val();
    }
    // if stems beside staff, length is fixed, but take into account shorter half note stems
    else {
        bool shrt = (minimStyle() == TablatureMinimStyle::SHORTER) && (chord->durationType().type() == DurationType::V_HALF);
        stemLen = (stemsDown() ? STAFFTYPE_TAB_DEFAULTSTEMLEN_DN : STAFFTYPE_TAB_DEFAULTSTEMLEN_UP)
                  * (shrt ? STAFFTYPE_TAB_SHORTSTEMRATIO : 1.0) * (chord->score()->styleB(Sid::useWideBeams) ? 1.25 : 1.0);
    }
    // scale length by scale of parent chord, but relative to scale of context staff
    return stemLen * chord->mag() / chord->staff()->staffMag(chord->tick());
}

//---------------------------------------------------------
//   fretString / durationString
//
//    construct the text string for a given fret / duration
//---------------------------------------------------------

static const QString unknownFret = QString("?");

QString StaffType::fretString(int fret, int string, bool deadNote) const
{
    if (fret == INVALID_FRET_INDEX) {
        return unknownFret;
    }
    if (deadNote) {
        return _fretFonts[_fretFontIdx].deadNoteChar;
    } else {
        bool hasFret;
        QString text  = tabBassStringPrefix(string, &hasFret);
        if (!hasFret) {             // if the notation does not allow to fret this string,
            return text;            // return the prefix only
        }
        // otherwise, add to prefix the relevant digit/letter string
        return text
               + (_useNumbers
                  ? (fret >= NUM_OF_DIGITFRETS ? unknownFret : _fretFonts[_fretFontIdx].displayDigit[fret])
                  : (fret >= NUM_OF_LETTERFRETS ? unknownFret : _fretFonts[_fretFontIdx].displayLetter[fret]));
    }
}

QString StaffType::durationString(DurationType type, int dots) const
{
    QString s = _durationFonts[_durationFontIdx].displayValue[int(type)];
    for (int count=0; count < dots; count++) {
        s.append(_durationFonts[_durationFontIdx].displayDot);
    }
    return s;
}

//---------------------------------------------------------
//    tabBassStringPrefix
//
//    returns a QString (possibly empty) with the prefix identifying a bass string in TAB's;
//    can deal with non-bass strings (i.e. regular TAB lines).
//
//    Implements the specifics of historic notations for bass lines (i.e. strings outside
//    the lines of the tab), both Italian and French.
//
//    strg   the instrument physical string ordinal (0 = topmost string, may exceed the number
//                of lines actually present in the TAB to reference a bass string)
//    bool   pntr to a bool receiving the info if notation allows to express a fret number or not
//                (this is potentially different from the fact that the instrument string itself can be fretted or not)
//---------------------------------------------------------

QString StaffType::tabBassStringPrefix(int strg, bool* hasFret) const
{
    *hasFret    = true;             // assume notation allows to fret this string
    int bassStrgIdx  = (strg >= _lines ? strg - _lines + 1 : 0);
    if (_useNumbers) {
        // if above the max bass string which can be fretted with number notation
        // return a number with the string index
        if (bassStrgIdx > NUM_OF_BASSSTRINGS_WITH_NUMBER) {
            *hasFret    = false;
            return _fretFonts[_fretFontIdx].displayDigit[strg + 1];
        }
        // if a frettable bass string, return an empty string
        return QString();
    } else {
        // bass string notation
        // if above the max bass string which can be fretted with letter notation
        // return a number with the bass string index itself
        if (bassStrgIdx > NUM_OF_BASSSTRINGS_WITH_LETTER) {
            *hasFret    = false;
            return _fretFonts[_fretFontIdx].displayDigit[bassStrgIdx - 1];
        }
        // if a frettable bass string, return a character with the relevant num. of slashes;
        // note that the number of slashes is bassStrgIdx-1 (1st bass has no slash)
        // and slashChar[] is 0-based (slashChar[0] => 1 slash, ...), whence the -2
        QString prefix    = bassStrgIdx > 1
                            ? QString(_fretFonts[_fretFontIdx].slashChar[bassStrgIdx - 2]) : QString();
        return prefix;
    }
}

//---------------------------------------------------------
//   numOfLedgerLines
//
//    in TAB's, returns the number of ledgerlines needed by bass lines in some TAB styles.
//
//    Returns 0 if staff is not a TAB, if a TAB but style does not use ledger lines
//    or ledger lines do not apply to the given string.
//---------------------------------------------------------

int StaffType::numOfTabLedgerLines(int string) const
{
    if (_group != StaffGroup::TAB || !_useNumbers) {
        return 0;
    }

    int numOfLedgers= string < 0 ? -string : string - _lines + 1;
    return numOfLedgers >= 1 && numOfLedgers <= NUM_OF_BASSSTRINGS_WITH_NUMBER ? numOfLedgers : 0;
}

//---------------------------------------------------------
//   physStringToVisual / visualStringToPhys
//
//    returns the string ordinal in visual order (top to down) from a string ordinal in physical order
//    or viceversa: manages upsideDown
//---------------------------------------------------------

int StaffType::physStringToVisual(int strg) const
{
    if (strg < 0) {                       // if above top string, return top string
        strg = 0;
    }
//      // NO: bass strings may exist, which are in addition to tab string lines
//      if (strg >= _lines)                 // if physical string has no visual representation,
//            strg = _lines - 1;            // reduce to nearest visual line
    // if TAB upside down, flip around top line
    return _upsideDown ? _lines - 1 - strg : strg;
}

int StaffType::visualStringToPhys(int line) const
{
    // if TAB upside down, reverse string number
    line = (_upsideDown ? _lines - 1 - line : line);

    if (line < 0) {           // if above top string, reduce to top string
        line = 0;
    }
// NO: bass strings may exist, which are in addition to tab string lines
//      if (line >= _lines)
//            line = _lines - 1;
    return line;
}

//---------------------------------------------------------
//   physStringToYOffset
//
//    returns the string Y offset from a string ordinal in physical order:
//    manages upsideDown and extra bass strings.
//
//    The returned values is in sp. and is relative to the staff top line.
//
//    Note: the difference with physStringToVisual() is that this function takes into account
//          peculiarities of bass string notations.
//---------------------------------------------------------

qreal StaffType::physStringToYOffset(int strg) const
{
    qreal yOffset = strg;                       // the y offset of the visual string, as a multiple of line distance
    if (yOffset < 0) {                          // if above top physical string, limit to top string
        yOffset = 0;
    }
    if (yOffset >= _lines) {                    // if physical string 'below' tab lines,
        yOffset = _lines;                       // reduce to first string 'below' tab body
        if (!_useNumbers) {                     // with letters, add some space for the slashes ascender
            yOffset = _onLines ? _lines : _lines + STAFFTYPE_TAB_BASSSLASH_YOFFSET;
        }
    }
    // if TAB upside down, flip around top line
    yOffset = _upsideDown ? (qreal)(_lines - 1) - yOffset : yOffset;
    return yOffset * _lineDistance.val();
}

//---------------------------------------------------------
//   TabDurationSymbol
//---------------------------------------------------------

TabDurationSymbol::TabDurationSymbol(ChordRest* parent)
    : EngravingItem(ElementType::TAB_DURATION_SYMBOL, parent, ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
    _beamGrid   = TabBeamGrid::NONE;
    _beamLength = 0.0;
    _tab        = 0;
    _text       = QString();
}

TabDurationSymbol::TabDurationSymbol(ChordRest* parent, const StaffType* tab, DurationType type, int dots)
    : EngravingItem(ElementType::TAB_DURATION_SYMBOL, parent, ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
    _beamGrid   = TabBeamGrid::NONE;
    _beamLength = 0.0;
    setDuration(type, dots, tab);
}

TabDurationSymbol::TabDurationSymbol(const TabDurationSymbol& e)
    : EngravingItem(e)
{
    _tab = e._tab;
    _text = e._text;
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TabDurationSymbol::layout()
{
    if (!_tab) {
        setbbox(RectF());
        return;
    }
    qreal _spatium    = spatium();
    qreal hbb, wbb, xbb, ybb;       // bbox sizes
    qreal xpos, ypos;               // position coords

    _beamGrid = TabBeamGrid::NONE;
    Chord* chord = explicitParent() && explicitParent()->isChord() ? toChord(explicitParent()) : nullptr;
    // if no chord (shouldn't happens...) or not a special beam mode, layout regular symbol
    if (!chord || !chord->isChord()
        || (chord->beamMode() != BeamMode::BEGIN && chord->beamMode() != BeamMode::MID
            && chord->beamMode() != BeamMode::END)) {
        mu::draw::FontMetrics fm(_tab->durationFont());
        hbb   = _tab->durationBoxH();
        wbb   = fm.width(_text);
        xbb   = 0.0;
        xpos  = 0.0;
        ypos  = _tab->durationFontYOffset();
        ybb   = _tab->durationBoxY() - ypos;
        // with rests, move symbol down by half its displacement from staff
        if (explicitParent() && explicitParent()->isRest()) {
            ybb  += TAB_RESTSYMBDISPL * _spatium;
            ypos += TAB_RESTSYMBDISPL * _spatium;
        }
    }
    // if on a chord with special beam mode, layout an 'English'-style duration grid
    else {
        TablatureDurationFont font = _tab->_durationFonts[_tab->_durationFontIdx];
        hbb   = font.gridStemHeight * _spatium;             // bbox height is stem height
        wbb   = font.gridStemWidth * _spatium;              // bbox width is stem width
        xbb   = -wbb * 0.5;                                 // bbox is half at left and half at right of stem centre
        ybb   = -hbb;                                       // bbox top is at top of stem height
        xpos  = 0.75 * _spatium;                            // conventional centring of stem on fret marks
        ypos  = _tab->durationGridYOffset();                // stem start is at bottom
        if (chord->beamMode() == BeamMode::BEGIN) {
            _beamGrid   = TabBeamGrid::INITIAL;
            _beamLength = 0.0;
        } else if (chord->beamMode() == BeamMode::MID || chord->beamMode() == BeamMode::END) {
            _beamLevel  = static_cast<int>(chord->durationType().type()) - static_cast<int>(font.zeroBeamLevel);
            _beamGrid   = (_beamLevel < 1 ? TabBeamGrid::INITIAL : TabBeamGrid::MEDIALFINAL);
            // _beamLength and bbox x and width will be set in layout2(),
            // once horiz. positions of chords are known
        }
    }
    // set this' mag from parent chord mag (include staff mag)
    qreal mag = chord != nullptr ? chord->mag() : 1.0;
    setMag(mag);
    mag = magS();             // local mag * score mag
    // set magnified bbox and position
    bbox().setRect(xbb * mag, ybb * mag, wbb * mag, hbb * mag);
    setPos(xpos * mag, ypos * mag);
}

//---------------------------------------------------------
//   layout2
//
//    Second step: after horizontal positions of elements involved are defined,
//    compute width of 'grid beams'
//---------------------------------------------------------

void TabDurationSymbol::layout2()
{
    // if not within a TAB or not a MEDIALFINAL grid element, do nothing
    if (!_tab || _beamGrid != TabBeamGrid::MEDIALFINAL) {
        return;
    }

    // get 'grid' beam length from page positions of this' chord and previous chord
    Chord* chord       = toChord(explicitParent());
    ChordRest* prevChord   = prevChordRest(chord, true);
    if (chord == nullptr || prevChord == nullptr) {
        return;
    }
    qreal mags        = magS();
    qreal beamLen     = prevChord->pagePos().x() - chord->pagePos().x();            // negative
    // page pos. difference already includes any magnification in effect:
    // scale it down, as it will be magnified again during drawing
    _beamLength = beamLen / mags;
    // update bbox x and w, but keep current y and h
    bbox().setLeft(beamLen);
    // set bbox width to half a stem width (magnified) plus beam length (already magnified)
    bbox().setWidth(_tab->_durationFonts[_tab->_durationFontIdx].gridStemWidth * spatium() * 0.5 * mags - beamLen);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TabDurationSymbol::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    if (!_tab) {
        return;
    }

    if (_repeat && (_tab->symRepeat() == TablatureSymbolRepeat::SYSTEM)) {
        Chord* chord = toChord(explicitParent());
        ChordRest* prevCR = prevChordRest(chord);
        if (prevCR && (chord->measure()->system() == prevCR->measure()->system())) {
            return;
        }
    }

    qreal mag = magS();
    qreal imag = 1.0 / mag;

    Pen pen(curColor());
    painter->setPen(pen);
    painter->scale(mag, mag);
    if (_beamGrid == TabBeamGrid::NONE) {
        // if no beam grid, draw symbol
        mu::draw::Font f(_tab->durationFont());
        f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
        painter->setFont(f);
        painter->drawText(PointF(0.0, 0.0), _text);
    } else {
        // if beam grid, draw stem line
        TablatureDurationFont& font = _tab->_durationFonts[_tab->_durationFontIdx];
        qreal _spatium = spatium();
        pen.setCapStyle(PenCapStyle::FlatCap);
        pen.setWidthF(font.gridStemWidth * _spatium);
        painter->setPen(pen);
        // take stem height from bbox, but de-magnify it, as drawing is already magnified
        qreal h     = bbox().y() / mag;
        painter->drawLine(PointF(0.0, h), PointF(0.0, 0.0));
        // if beam grid is medial/final, draw beam lines too: lines go from mid of
        // previous stem (delta x stored in _beamLength) to mid of this' stem (0.0)
        if (_beamGrid == TabBeamGrid::MEDIALFINAL) {
            pen.setWidthF(font.gridBeamWidth * _spatium);
            painter->setPen(pen);
            // lower height available to beams by half a beam width,
            // so that top beam upper border aligns with stem top
            h += (font.gridBeamWidth * _spatium) * 0.5;
            // draw beams equally spaced within the stem height (this is
            // different from modern engraving, but common in historic prints)
            qreal step  = -h / _beamLevel;
            qreal y     = h;
            for (int i = 0; i < _beamLevel; i++, y += step) {
                painter->drawLine(PointF(_beamLength, y), PointF(0.0, y));
            }
        }
    }
    painter->scale(imag, imag);
}

//---------------------------------------------------------
//   STATIC FUNCTIONS FOR FONT CONFIGURATION MANAGEMENT
//---------------------------------------------------------

bool TablatureFretFont::read(XmlReader& e)
{
    defPitch    = 9.0;
    defYOffset  = 0.0;
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());

        int val = e.intAttribute("value");

        if (tag == "family") {
            family = e.readElementText();
        } else if (tag == "displayName") {
            displayName = e.readElementText();
        } else if (tag == "defaultPitch") {
            defPitch = e.readDouble();
        } else if (tag == "defaultYOffset") {
            defYOffset = e.readDouble();
        } else if (tag == "mark") {
            QString sval = e.attribute("value");
            int num  = e.intAttribute("number", 1);
            QString txt(e.readElementText());
            if (sval.size() < 1) {
                return false;
            }
            if (sval == "x") {
                xChar = txt[0];
            } else if (sval == "ghost") {
                deadNoteChar = txt[0];
            } else if (sval == "slash") {
                // limit within legal range
                if (num < 1) {
                    num = 1;
                }
                if (num > NUM_OF_BASSSTRING_SLASHES) {
                    num = NUM_OF_BASSSTRING_SLASHES;
                }
                slashChar[num - 1] = txt;
            }
        } else if (tag == "fret") {
            bool bLetter = e.intAttribute("letter");
            QString txt(e.readElementText());
            if (bLetter) {
                if (val >= 0 && val < NUM_OF_LETTERFRETS) {
                    displayLetter[val] = txt[0];
                }
            } else {
                if (val >= 0 && val < NUM_OF_DIGITFRETS) {
                    displayDigit[val] = txt;
                }
            }
        } else {
            e.unknown();
            return false;
        }
    }
    return true;
}

bool TablatureDurationFont::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());

        if (tag == "family") {
            family = e.readElementText();
        } else if (tag == "displayName") {
            displayName = e.readElementText();
        } else if (tag == "defaultPitch") {
            defPitch = e.readDouble();
        } else if (tag == "defaultYOffset") {
            defYOffset = e.readDouble();
        } else if (tag == "beamWidth") {
            gridBeamWidth = e.readDouble();
        } else if (tag == "stemHeight") {
            gridStemHeight = e.readDouble();
        } else if (tag == "stemWidth") {
            gridStemWidth = e.readDouble();
        } else if (tag == "zeroBeamValue") {
            QString val(e.readElementText());
            if (val == "longa") {
                zeroBeamLevel = DurationType::V_LONG;
            } else if (val == "brevis") {
                zeroBeamLevel = DurationType::V_BREVE;
            } else if (val == "semibrevis") {
                zeroBeamLevel = DurationType::V_WHOLE;
            } else if (val == "minima") {
                zeroBeamLevel = DurationType::V_HALF;
            } else if (val == "semiminima") {
                zeroBeamLevel = DurationType::V_QUARTER;
            } else if (val == "fusa") {
                zeroBeamLevel = DurationType::V_EIGHTH;
            } else if (val == "semifusa") {
                zeroBeamLevel = DurationType::V_16TH;
            } else if (val == "32") {
                zeroBeamLevel = DurationType::V_32ND;
            } else if (val == "64") {
                zeroBeamLevel = DurationType::V_64TH;
            } else if (val == "128") {
                zeroBeamLevel = DurationType::V_128TH;
            } else if (val == "256") {
                zeroBeamLevel = DurationType::V_256TH;
            } else if (val == "512") {
                zeroBeamLevel = DurationType::V_512TH;
            } else if (val == "1024") {
                zeroBeamLevel = DurationType::V_1024TH;
            } else {
                e.unknown();
            }
        } else if (tag == "duration") {
            QString val = e.attribute("value");
            QString txt(e.readElementText());
            QChar chr = txt[0];
            if (val == "longa") {
                displayValue[int(TabVal::VAL_LONGA)] = chr;
            } else if (val == "brevis") {
                displayValue[int(TabVal::VAL_BREVIS)] = chr;
            } else if (val == "semibrevis") {
                displayValue[int(TabVal::VAL_SEMIBREVIS)] = chr;
            } else if (val == "minima") {
                displayValue[int(TabVal::VAL_MINIMA)] = chr;
            } else if (val == "semiminima") {
                displayValue[int(TabVal::VAL_SEMIMINIMA)] = chr;
            } else if (val == "fusa") {
                displayValue[int(TabVal::VAL_FUSA)] = chr;
            } else if (val == "semifusa") {
                displayValue[int(TabVal::VAL_SEMIFUSA)] = chr;
            } else if (val == "32") {
                displayValue[int(TabVal::VAL_32)] = chr;
            } else if (val == "64") {
                displayValue[int(TabVal::VAL_64)] = chr;
            } else if (val == "128") {
                displayValue[int(TabVal::VAL_128)] = chr;
            } else if (val == "256") {
                displayValue[int(TabVal::VAL_256)] = chr;
            } else if (val == "512") {
                displayValue[int(TabVal::VAL_512)] = chr;
            } else if (val == "1024") {
                displayValue[int(TabVal::VAL_1024)] = chr;
            } else if (val == "dot") {
                displayDot = chr;
            } else {
                e.unknown();
            }
        } else {
            e.unknown();
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   Read Configuration File
//
//    reads a configuration and appends read data to g_TABFonts
//    resets everything and reads the built-in config file if fileName is null or empty
//---------------------------------------------------------

bool StaffType::readConfigFile(const QString& fileName)
{
    QString path;

    if (fileName == 0 || fileName.isEmpty()) {         // defaults to built-in xml
#ifdef Q_OS_IOS
        {
            extern QString resourcePath();
            QString rpath = resourcePath();
            path = rpath + QString("/fonts_tablature.xml");
        }
#else
        path = ":/fonts/fonts_tablature.xml";
#endif
        _durationFonts.clear();
        _fretFonts.clear();
    } else {
        path = fileName;
    }

    QFileInfo fi(path);
    QFile f(path);

    if (!fi.exists() || !f.open(QIODevice::ReadOnly)) {
        MScore::lastError = QObject::tr("Cannot open tablature font description:\n%1\n%2").arg(f.fileName(), f.errorString());
        qDebug("StaffTypeTablature::readConfigFile failed: <%s>", qPrintable(path));
        return false;
    }

    XmlReader e(&f);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            while (e.readNextStartElement()) {
                const QStringRef& tag(e.name());
                if (tag == "fretFont") {
                    TablatureFretFont ff;
                    if (ff.read(e)) {
                        _fretFonts.push_back(ff);
                    } else {
                        continue;
                    }
                } else if (tag == "durationFont") {
                    TablatureDurationFont df;
                    if (df.read(e)) {
                        _durationFonts.push_back(df);
                    } else {
                        continue;
                    }
                } else {
                    e.unknown();
                }
            }
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   fontNames
//
//    returns a list of display names for the fonts  configured to work with Tablatures;
//    the index of a name in the list can be used to retrieve the font data with fontData()
//---------------------------------------------------------

std::vector<QString> StaffType::fontNames(bool bDuration)
{
    std::vector<QString> names;
    if (bDuration) {
        for (const TablatureDurationFont& f : _durationFonts) {
            names.push_back(f.displayName);
        }
    } else {
        for (const TablatureFretFont& f : _fretFonts) {
            names.push_back(f.displayName);
        }
    }
    return names;
}

//---------------------------------------------------------
//   fontData
//
//    retrieves data about a Tablature font.
//    returns: true if idx is valid | false if it is not
// any of the pointer parameter can be null, if that datum is not needed
//---------------------------------------------------------

bool StaffType::fontData(bool bDuration, size_t nIdx, QString* pFamily, QString* pDisplayName,
                         qreal* pSize, qreal* pYOff)
{
    if (bDuration) {
        if (nIdx < _durationFonts.size()) {
            TablatureDurationFont f = _durationFonts.at(nIdx);
            if (pFamily) {
                *pFamily          = f.family;
            }
            if (pDisplayName) {
                *pDisplayName     = f.displayName;
            }
            if (pSize) {
                *pSize            = f.defPitch;
            }
            if (pYOff) {
                *pYOff            = f.defYOffset;
            }
            return true;
        }
    } else {
        if (nIdx < _fretFonts.size()) {
            TablatureFretFont f = _fretFonts.at(nIdx);
            if (pFamily) {
                *pFamily          = f.family;
            }
            if (pDisplayName) {
                *pDisplayName     = f.displayName;
            }
            if (pSize) {
                *pSize            = f.defPitch;
            }
            if (pYOff) {
                *pYOff            = f.defYOffset;
            }
            return true;
        }
    }
    return false;
}

//=========================================================
//
//   BUILT-IN STAFF TYPES and STAFF TYPE PRESETS
//
//=========================================================

static const int _defaultPreset[STAFF_GROUP_MAX] =
{ 0,                    // default pitched preset is "stdNormal"
  3,                    // default percussion preset is "perc5lines"
  5                     // default tab preset is "tab6StrCommon"
};

static const QString _emptyString = QString();

//---------------------------------------------------------
//   Static functions for StaffType presets
//---------------------------------------------------------

const StaffType* StaffType::preset(StaffTypes idx)
{
    if (int(idx) < 0 || int(idx) >= int(_presets.size())) {
        return &_presets[0];
    }

    return &_presets[int(idx)];
}

const StaffType* StaffType::presetFromXmlName(const QString& xmlName)
{
    for (size_t i = 0; i < _presets.size(); ++i) {
        if (_presets[i].xmlName() == xmlName) {
            return &_presets[i];
        }
    }

    return nullptr;
}

const StaffType* StaffType::getDefaultPreset(StaffGroup grp)
{
    int _idx = _defaultPreset[int(grp)];
    return &_presets[_idx];
}

//---------------------------------------------------------
//   initStaffTypes
//---------------------------------------------------------

std::vector<StaffType> StaffType::_presets;
/* *INDENT-OFF* */
void StaffType::initStaffTypes()
{
    readConfigFile(0);            // get TAB font config, before initStaffTypes()

    // keep in sync with enum class StaffTypes
    _presets = {
//                       group,              xml-name,  human-readable-name,          lin stpOff  dist clef   bars stmless time  key    ledger invis     color
        StaffType(StaffGroup::STANDARD,   "stdNormal", QObject::tr("Standard"),        5, 0,     1,   true,  true, false, true, true, true, false,  engravingConfiguration()->defaultColor()),
//       StaffType(StaffGroup::PERCUSSION, "perc1Line", QObject::tr("Perc. 1 line"),    1, -4,    1,   true,  true, false, true, false, true, false,   engravingConfiguration()->defaultColor()),
        StaffType(StaffGroup::PERCUSSION, "perc1Line", QObject::tr("Perc. 1 line"),    1, 0,     1,   true,  true, false, true, false, true, false,  engravingConfiguration()->defaultColor()),
        StaffType(StaffGroup::PERCUSSION, "perc3Line", QObject::tr("Perc. 3 lines"),   3, 0,     2,   true,  true, false, true, false, true, false,  engravingConfiguration()->defaultColor()),
        StaffType(StaffGroup::PERCUSSION, "perc5Line", QObject::tr("Perc. 5 lines"),   5, 0,     1,   true,  true, false, true, false, true, false,  engravingConfiguration()->defaultColor()),

//                 group            xml-name,     human-readable-name                  lin stpOff dist clef   bars stemless time   invis     color   duration font     size off genDur     fret font          size off  duration symbol repeat      thru       minim style              onLin  rests  stmDn  stmThr upsDn  sTFing nums  bkTied
//        StaffType(StaffGroup::TAB, "tab6StrSimple", QObject::tr("Tab. 6-str. simple"), 6, 2,     1.5, true,  true, true,  false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Sans",    9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,   true,  false, true,  false, false, false, true, false),
//        StaffType(StaffGroup::TAB, "tab6StrCommon", QObject::tr("Tab. 6-str. common"), 6, 2,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  false, true,  false, false, false, true, true),
//        StaffType(StaffGroup::TAB, "tab6StrFull",   QObject::tr("Tab. 6-str. full"),   6, 2,     1.5, true,  true, false, true, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED,true,  true,  true,  true,  false, false, true, true),
        StaffType(StaffGroup::TAB, "tab6StrSimple", QObject::tr("Tab. 6-str. simple"), 6, 0,     1.5, true,  true, true,  false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Sans",    9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,   true,  false, true,  false, false, false, true, false),
        StaffType(StaffGroup::TAB, "tab6StrCommon", QObject::tr("Tab. 6-str. common"), 6, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  false, true,  false, false, false, true, true),
        StaffType(StaffGroup::TAB, "tab6StrFull",   QObject::tr("Tab. 6-str. full"),   6, 0,     1.5, true,  true, false, true, false,  engravingConfiguration()->defaultColor(),  "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED,  true,  true,  true,  true,  false, false, true, true),
        StaffType(StaffGroup::TAB, "tab4StrSimple", QObject::tr("Tab. 4-str. simple"), 4, 0,     1.5, true,  true, true,  false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Sans",    9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,   true,  false, true,  false, false, false, true, false),
        StaffType(StaffGroup::TAB, "tab4StrCommon", QObject::tr("Tab. 4-str. common"), 4, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  false, true,  false, false, false, true, true),
        StaffType(StaffGroup::TAB, "tab4StrFull",   QObject::tr("Tab. 4-str. full"),   4, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED,  true,  true,  true,  true,  false, false, true, true),
        StaffType(StaffGroup::TAB, "tab5StrSimple", QObject::tr("Tab. 5-str. simple"), 5, 0,     1.5, true,  true, true,  false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Sans",    9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::NONE,   true,  false, true,  false, false, false, true, false),
        StaffType(StaffGroup::TAB, "tab5StrCommon", QObject::tr("Tab. 5-str. common"), 5, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  false, true,  false, false, false, true, true),
        StaffType(StaffGroup::TAB, "tab5StrFull",   QObject::tr("Tab. 5-str. full"),   5, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SLASHED,  true,  true,  true,  true,  false, false, true, true),
        StaffType(StaffGroup::TAB, "tabUkulele",    QObject::tr("Tab. ukulele"),       4, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true,  true,  false, false, false, true, true),
        StaffType(StaffGroup::TAB, "tabBalajka",    QObject::tr("Tab. balalaika"),     3, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true,  true,  false, false, false, true, true),
        StaffType(StaffGroup::TAB, "tabDulcimer",   QObject::tr("Tab. dulcimer"),      3, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  true,  true,  false, true,  false, true, true),
//       StaffType(StaffGroup::TAB, "tab6StrItalian",QObject::tr("Tab. 6-str. Italian"),6, 2,     1.5, false, true, true,  true,  "MuseScore Tab Italian",15, 0, true,  "MuseScore Tab Renaiss",10, 0, TablatureSymbolRepeat::NEVER, true,  TablatureMinimStyle::NONE,   true,  true,  false, false, true,  false, true, false),
//       StaffType(StaffGroup::TAB, "tab6StrFrench", QObject::tr("Tab. 6-str. French"), 6, 2,     1.5, false, true, true,  true,  "MuseScore Tab French", 15, 0, true,  "MuseScore Tab Renaiss",10, 0, TablatureSymbolRepeat::NEVER, true,  TablatureMinimStyle::NONE,   false, false, false, false, false, false, false,false)
        StaffType(StaffGroup::TAB, "tab6StrItalian",QObject::tr("Tab. 6-str. Italian"),6, 0,     1.5, false, true, true,  true, false,  engravingConfiguration()->defaultColor(),  "MuseScore Tab Italian",15, 0, true,  "MuseScore Tab Renaiss",10, 0, TablatureSymbolRepeat::NEVER, true,  TablatureMinimStyle::NONE,   true,  true,  false, false, true,  false, true, false),
        StaffType(StaffGroup::TAB, "tab6StrFrench", QObject::tr("Tab. 6-str. French"), 6, 0,     1.5, false, true, true,  true, false,  engravingConfiguration()->defaultColor(),  "MuseScore Tab French", 15, 0, true,  "MuseScore Tab Renaiss",10, 0, TablatureSymbolRepeat::NEVER, true,  TablatureMinimStyle::NONE,   false, false, false, false, false, false, false, false),
        StaffType(StaffGroup::TAB, "tab7StrCommon", QObject::tr("Tab. 7-str. common"), 7, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  false, true,  false, false, false, true, true),
        StaffType(StaffGroup::TAB, "tab8StrCommon", QObject::tr("Tab. 8-str. common"), 8, 0,     1.5, true,  true, false, false, false,  engravingConfiguration()->defaultColor(), "MuseScore Tab Modern", 15, 0, false, "MuseScore Tab Serif",   9, 0, TablatureSymbolRepeat::NEVER, false, TablatureMinimStyle::SHORTER,true,  false, true,  false, false, false, true, true),
    };
}
/* *INDENT-ON* */
//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal StaffType::spatium(Score* score) const
{
    return score->spatium() * (isSmall() ? score->styleD(Sid::smallStaffMag) : 1.0) * userMag();
}
} // namespace Ms
