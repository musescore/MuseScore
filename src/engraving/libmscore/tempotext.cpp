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

#include <cmath>
#include <utility>

#include "score.h"
#include "tempotext.h"
#include "textedit.h"
#include "tempo.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "xml.h"
#include "undo.h"
#include "musescoreCore.h"

#include "log.h"

#include <QRegularExpression>

using namespace mu;

namespace Ms {
#define MIN_TEMPO 5.0 / 60
#define MAX_TEMPO 999.0 / 60

//TODO: textChanged() needs to be called during/after editing

//---------------------------------------------------------
//   tempoStyle
//---------------------------------------------------------

static const ElementStyle tempoStyle {
    { Sid::tempoSystemFlag,                    Pid::SYSTEM_FLAG },
    { Sid::tempoPlacement,                     Pid::PLACEMENT },
    { Sid::tempoMinDistance,                   Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
    : TextBase(s, Tid::TEMPO, ElementFlags(ElementFlag::SYSTEM))
{
    initElementStyle(&tempoStyle);
    _playbackTempo      = 2.0;        // propertyDefault(P_TEMPO).toDouble();
    _followText = false;
    _relative   = 1.0;
    _isRelative = false;
    _equation   = "q = 120";
    _tempoText = "";
    _isEquationVisible = true;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TempoText::write(XmlWriter& xml) const
{
    xml.stag(this);
    xml.tag("playbackTempo", _playbackTempo);
    xml.tag("notatedTempo", _notatedTempo);
    xml.tag("equation", _equation);
    xml.tag("tempoText", _tempoText);
    if (_followText) {
        xml.tag("followText", _followText);
    }
    TextBase::writeProperties(xml);
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TempoText::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "playbackTempo") {
            setTempo(e.readDouble());
        } else if (tag == "followText") {
            _followText = e.readInt();
        } else if (tag == "equation") {
            _equation = e.readElementText();
        } else if (tag == "notatedTempo") {
            _notatedTempo = e.readInt();
        } else if (tag == "tempoText") {
            _tempoText = e.readElementText();
        } else if (!TextBase::readProperties(e)) {
            e.unknown();
        }
    }
    // check sanity
    if (xmlText().isEmpty()) {
        setXmlText(QString("q = %1").arg(lrint(60 * _playbackTempo)));
        setVisible(false);
    }
}

qreal TempoText::tempoBpm() const
{
    return _notatedTempo;
}

//---------------------------------------------------------
//   TempoPattern
//---------------------------------------------------------

struct TempoPattern {
    const char* pattern;
    qreal f;
    TDuration d;
    TempoPattern(const char* s, qreal v, TDuration::DurationType val, int dots = 0)
        : pattern(s), f(v), d(val)
    {
        d.setDots(dots);
    }
};

// note: findTempoDuration requires the longer patterns to be before the shorter patterns in tp

static const TempoPattern tp[] = {
    TempoPattern("\uECA5\\s*\uECB7\\s*\uECB7", 1.75 / 60.0,  TDuration::DurationType::V_QUARTER, 2), // double dotted 1/4
    TempoPattern("\uECA5\\s*\uECB7",           1.5 / 60.0,   TDuration::DurationType::V_QUARTER, 1), // dotted 1/4
    TempoPattern("\uECA5",                     1.0 / 60.0,   TDuration::DurationType::V_QUARTER),    // 1/4
    TempoPattern("\uECA3\\s*\uECB7\\s*\uECB7", 1.75 / 30.0,  TDuration::DurationType::V_HALF, 2),    // double dotted 1/2
    TempoPattern("\uECA3\\s*\uECB7",           1.5 / 30.0,   TDuration::DurationType::V_HALF, 1),    // dotted 1/2
    TempoPattern("\uECA3",                     1.0 / 30.0,   TDuration::DurationType::V_HALF),       // 1/2
    TempoPattern("\uECA7\\s*\uECB7\\s*\uECB7", 1.75 / 120.0, TDuration::DurationType::V_EIGHTH, 2),  // double dotted 1/8
    TempoPattern("\uECA7\\s*\uECB7",           1.5 / 120.0,  TDuration::DurationType::V_EIGHTH, 1),  // dotted 1/8
    TempoPattern("\uECA7",                     1.0 / 120.0,  TDuration::DurationType::V_EIGHTH),     // 1/8
    TempoPattern("\uECA2\\s*\uECB7",           1.5 / 15.0,   TDuration::DurationType::V_WHOLE, 1),   // dotted whole
    TempoPattern("\uECA2",                     1.0 / 15.0,   TDuration::DurationType::V_WHOLE),      // whole
    TempoPattern("\uECA9\\s*\uECB7",           1.5 / 240.0,  TDuration::DurationType::V_16TH, 1),    // dotted 1/16
    TempoPattern("\uECA9",                     1.0 / 240.0,  TDuration::DurationType::V_16TH),       // 1/16
    TempoPattern("\uECAB\\s*\uECB7",           1.5 / 480.0,  TDuration::DurationType::V_32ND, 1),    // dotted 1/32
    TempoPattern("\uECAB",                     1.0 / 480.0,  TDuration::DurationType::V_32ND),       // 1/32
    TempoPattern("\uECA1",                     1.0 / 7.5,    TDuration::DurationType::V_BREVE),      // longa
    TempoPattern("\uECA0",                     1.0 / 7.5,    TDuration::DurationType::V_BREVE),      // double whole
    TempoPattern("\uECAD",                     1.0 / 960.0,  TDuration::DurationType::V_64TH),       // 1/64
    TempoPattern("\uECAF",                     1.0 / 1920.0, TDuration::DurationType::V_128TH),      // 1/128
    TempoPattern("\uECB1",                     1.0 / 3840.0, TDuration::DurationType::V_256TH),      // 1/256
    TempoPattern("\uECB3",                     1.0 / 7680.0, TDuration::DurationType::V_512TH),      // 1/512
    TempoPattern("\uECB5",                     1.0 / 15360.0, TDuration::DurationType::V_1024TH),     // 1/1024
};

//---------------------------------------------------------
//   findTempoDuration
//    find the duration part (note + dot) of a tempo text in string s
//    return the match position or -1 if not found
//    set len to the match length and dur to the duration value
//---------------------------------------------------------

int TempoText::findTempoDuration(const QString& s, int& len, TDuration& dur)
{
    len = 0;
    dur = TDuration();
    for (const auto& i : tp) {
        QRegExp re(i.pattern);
        int pos = re.indexIn(s);
        if (pos != -1) {
            len = re.matchedLength();
            dur = i.d;
            return pos;
        }
    }
    return -1;
}

TDuration TempoText::duration() const
{
    int dummy = 0;
    TDuration result;

    findTempoDuration(xmlText(), dummy, result);

    return result;
}

static const TempoPattern tpSym[] = {
    TempoPattern("<sym>metNoteQuarterUp</sym>\\s*<sym>metAugmentationDot</sym>\\s*<sym>metAugmentationDot</sym>",
                 1.75 / 60.0, TDuration::DurationType::V_QUARTER, 2),                                                                                                                          // double dotted 1/4
    TempoPattern("<sym>metNoteQuarterUp</sym>\\s*<sym>metAugmentationDot</sym>",          1.5 / 60.0,  TDuration::DurationType::V_QUARTER,
                 1),                                                                                                                           // dotted 1/4
    TempoPattern("<sym>metNoteQuarterUp</sym>",                                           1.0 / 60.0,  TDuration::DurationType::V_QUARTER),  // 1/4
    TempoPattern("<sym>metNoteHalfUp</sym>\\s*<sym>metAugmentationDot</sym>\\s*<sym>metAugmentationDot</sym>",
                 1.75 / 30.0, TDuration::DurationType::V_HALF, 2),                                                                                                                       // double dotted 1/2
    TempoPattern("<sym>metNoteHalfUp</sym>\\s*<sym>metAugmentationDot</sym>",             1.5 / 30.0,  TDuration::DurationType::V_HALF, 1),    // dotted 1/2
    TempoPattern("<sym>metNoteHalfUp</sym>",                                              1.0 / 30.0,  TDuration::DurationType::V_HALF),     // 1/2
    TempoPattern("<sym>metNote8thUp</sym>\\s*<sym>metAugmentationDot</sym>\\s*<sym>metAugmentationDot</sym>",         1.75 / 120.0,
                 TDuration::DurationType::V_EIGHTH, 2),                                                                                                                    // double dotted 1/8
    TempoPattern("<sym>metNote8thUp</sym>\\s*<sym>metAugmentationDot</sym>",              1.5 / 120.0, TDuration::DurationType::V_EIGHTH,
                 1),                                                                                                                           // dotted 1/8
    TempoPattern("<sym>metNote8thUp</sym>",                                               1.0 / 120.0, TDuration::DurationType::V_EIGHTH),   // 1/8
    TempoPattern("<sym>metNoteWhole</sym>\\s*<sym>metAugmentationDot</sym>",              1.5 / 15.0,  TDuration::DurationType::V_WHOLE, 1),    // dotted whole
    TempoPattern("<sym>metNoteWhole</sym>",                                               1.0 / 15.0,  TDuration::DurationType::V_WHOLE),    // whole
    TempoPattern("<sym>metNote16thUp</sym>\\s*<sym>metAugmentationDot</sym>",             1.5 / 240.0, TDuration::DurationType::V_16TH, 1),  // dotted 1/16
    TempoPattern("<sym>metNote16thUp</sym>",                                              1.0 / 240.0, TDuration::DurationType::V_16TH),     // 1/16
    TempoPattern("<sym>metNote32ndUp</sym>\\s*<sym>metAugmentationDot</sym>",             1.5 / 480.0, TDuration::DurationType::V_32ND, 1),  // dotted 1/32
    TempoPattern("<sym>metNote32ndUp</sym>",                                              1.0 / 480.0, TDuration::DurationType::V_32ND),     // 1/32
    TempoPattern("<sym>metNoteDoubleWholeSquare</sym>",                                   1.0 / 7.5,   TDuration::DurationType::V_BREVE),    // longa
    TempoPattern("<sym>metNoteDoubleWhole</sym>",                                         1.0 / 7.5,   TDuration::DurationType::V_BREVE),    // double whole
    TempoPattern("<sym>metNote64thUp</sym>",                                              1.0 / 960.0, TDuration::DurationType::V_64TH),     // 1/64
    TempoPattern("<sym>metNote128thUp</sym>",                                             1.0 / 1920.0, TDuration::DurationType::V_128TH),    // 1/128
    TempoPattern("<sym>metNote256thUp</sym>",                                             1.0 / 3840.0, TDuration::DurationType::V_256TH),    // 1/256
    TempoPattern("<sym>metNote512thUp</sym>",                                             1.0 / 7680.0, TDuration::DurationType::V_512TH),    // 1/512
    TempoPattern("<sym>metNote1024thUp</sym>",                                            1.0 / 15360.0, TDuration::DurationType::V_1024TH),  // 1/1024
};

//---------------------------------------------------------
//   duration2tempoTextString
//    find the tempoText string representation for duration
//---------------------------------------------------------

QString TempoText::duration2tempoTextString(const TDuration dur)
{
    for (const TempoPattern& pa : tpSym) {
        if (pa.d == dur) {
            QString res = pa.pattern;
            res.replace("\\s*", " ");
            return res;
        }
    }
    return "";
}

//---------------------------------------------------------
// updateScore
//---------------------------------------------------------

void TempoText::updateScore()
{
    if (segment()) {
        score()->setTempo(segment(), _playbackTempo);
    }
    score()->fixTicks();
    score()->setPlaylistDirty();
}

//---------------------------------------------------------
// updateRelative
//---------------------------------------------------------

void TempoText::updateRelative()
{
    qreal tempoBefore = score()->tempo(tick() - Fraction::fromTicks(1));
    setTempo(tempoBefore * _relative);
}

void TempoText::startEdit(EditData& ed)
{
    TextBase::startEdit(ed);

    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
    TextCursor* cursor = ted->cursor();

    int cursorIndex = textIndexFromCursor(cursor->row(), cursor->column());
    std::pair<int, int> eqnIndices = equationIndices();

    if (cursorIndex >= eqnIndices.first && cursorIndex <= eqnIndices.second) {
        cursor->movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, cursorIndex - eqnIndices.first);
    }
}

//---------------------------------------------------------
//   endEdit
//    text may have changed
//---------------------------------------------------------

void TempoText::endEdit(EditData& ed)
{
    TextBase::endEdit(ed);
    if (_followText) {
        UndoStack* us = score()->undoStack();
        UndoCommand* ucmd = us->last();
        if (ucmd) {
            us->reopen();
            updateTempo();
            score()->endCmd();
        } else {
            score()->startCmd();
            updateTempo();
            score()->endCmd();
        }
    }
}

int TempoText::textIndexFromCursor(int row, int column) const
{
    int index = 0;
    for (int i = 0; i < row; i++) {
        index = xmlText().indexOf("\n", index);
    }

    index += column;

    return index;
}

std::pair<int, int> TempoText::cursorIndexFromTextIndex(int index) const
{
    int row = 0;
    int lineIndex = xmlText().indexOf("\n");

    while (lineIndex < index && row < xmlText().count("\n")) {
        lineIndex = xmlText().indexOf("\n", lineIndex);
        row++;
    }

    int column = index - lineIndex - 1;

    return std::make_pair(row, column);
}

std::pair<int, int> TempoText::equationIndices() const
{
    static const QRegularExpression equationExpression("(?<equation>[\\[\\(]?[\uECA0-\uECB7]+ *= *[\uECA0-\uECB70-9]+[\\]\\)]?)");
    QRegularExpressionMatch match = equationExpression.match(xmlText());

    if (!match.hasMatch()) {
        return std::make_pair(0, 0);
    }

    return std::make_pair(match.capturedStart("equation"), match.capturedEnd("equation"));
}

bool TempoText::moveCursor(TextCursor* cursor, int key, bool ctrlPressed, QTextCursor::MoveMode moveMode) const
{
    int cursorIndex = textIndexFromCursor(cursor->row(), cursor->column());

    std::pair<int, int> eqnIndices = equationIndices();

    if (eqnIndices.first == 0 && eqnIndices.second == 0) {
        return TextBase::moveCursor(cursor, key, ctrlPressed, moveMode);
    }

    int length = eqnIndices.second - eqnIndices.first;

    if (eqnIndices.first == cursorIndex && key == Qt::Key_Right) {
        return cursor->movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, length);
    } else if (eqnIndices.second == cursorIndex && key == Qt::Key_Left) {
        return cursor->movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, length);
    } else {
        if (key == Qt::Key_Left) {
            return cursor->movePosition(ctrlPressed ? QTextCursor::WordLeft : QTextCursor::Left, moveMode);
        } else if (key == Qt::Key_Right) {
            return cursor->movePosition(ctrlPressed ? QTextCursor::WordRight : QTextCursor::Right, moveMode);
        }
    }

    return false;
}

void TempoText::dragTo(EditData& ed)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
    TextCursor* cursor = ted->cursor();

    cursor->set(ed.pos, QTextCursor::KeepAnchor);

    int cursorEndIndex = textIndexFromCursor(cursor->row(), cursor->column());
    int cursorStartIndex = textIndexFromCursor(cursor->selectLine(), cursor->selectColumn());

    std::pair<int, int> eqnIndices = equationIndices();

    if (!(eqnIndices.first == 0 && eqnIndices.second == 0)) {
        int equationStart = eqnIndices.first;
        int equationEnd = eqnIndices.second;

        if (cursorStartIndex <= equationStart && cursorEndIndex > equationStart) {
            std::pair<int, int> indices = cursorIndexFromTextIndex(equationStart);

            cursor->setRow(indices.first);
            cursor->setColumn(indices.second);
        } else if (cursorStartIndex >= equationEnd && cursorEndIndex < equationEnd) {
            std::pair<int, int> indices = cursorIndexFromTextIndex(equationEnd);

            cursor->setRow(indices.first);
            cursor->setColumn(indices.second);
        }
    }

    score()->setUpdateAll();
    score()->update();
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void TempoText::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
{
    if (id == Pid::TEMPO_FOLLOW_TEXT) {
        ScoreElement::undoChangeProperty(id, v, ps);
        if (_followText) {
            updateTempo();
        }
    } else {
        ScoreElement::undoChangeProperty(id, v, ps);
    }
}

//---------------------------------------------------------
//   updateTempo
//---------------------------------------------------------

void TempoText::updateTempo()
{
    static const float quarterNotePlayback = 1.f / 60.f;

    static const QRegularExpression bpmExpression("^[\\[,\\(]?[ ]*(?<note>[d,w,h,q,e,s,t].*) *= *(?<bpm>\\d+)[\\],\\)]?$");
    static const QRegularExpression relativeExpression(
        "^[\\[,\\(]?[ ]*(?<note1>[d,w,h,q,e,s,t].*) *= *(?<note2>[d,w,h,q,e,s,t].*)[\\],\\)]?$");

    QRegularExpressionMatch bpmMatch = bpmExpression.match(_equation);
    QRegularExpressionMatch relativeMatch = relativeExpression.match(_equation);

    if (bpmMatch.hasMatch()) {
        const float relativeDuration = getRelativeDuration(bpmMatch.captured("note"));
        const float bpm = bpmMatch.captured("bpm").toFloat();

        _notatedTempo = bpm;

        qreal playbackTempo(relativeDuration * bpm * quarterNotePlayback);

        if (playbackTempo != _playbackTempo) {
            //undoChangeProperty(Pid::TEMPO, QVariant(playbackTempo), propertyFlags(Pid::TEMPO));
            _playbackTempo = playbackTempo;

            _relative = 1.0;
            _isRelative = false;

            updateScore();
        }
    } else if (relativeMatch.hasMatch()) {
        _relative = getRelativeDuration(relativeMatch.captured("note2")) / getRelativeDuration(relativeMatch.captured("note1"));
        _isRelative = true;

        updateRelative();
        updateScore();
    }
}

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void TempoText::setTempo(qreal v)
{
    if (v < MIN_TEMPO) {
        v = MIN_TEMPO;
    } else if (v > MAX_TEMPO) {
        v = MAX_TEMPO;
    }
    _playbackTempo = v;
}

//---------------------------------------------------------
//   undoSetTempo
//---------------------------------------------------------

void TempoText::undoSetTempo(qreal v)
{
    undoChangeProperty(Pid::TEMPO, v, propertyFlags(Pid::TEMPO));
}

//---------------------------------------------------------
//   undoSetFollowText
//---------------------------------------------------------

void TempoText::undoSetFollowText(bool v)
{
    undoChangeProperty(Pid::TEMPO_FOLLOW_TEXT, v, propertyFlags(Pid::TEMPO));
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TempoText::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TEMPO:
        return _playbackTempo;
    case Pid::TEMPO_FOLLOW_TEXT:
        return _followText;
    case Pid::TEMPO_EQUATION:
        LOGD() << _equation;
        return _equation;
    case Pid::TEMPO_EQUATION_VISIBLE:
        return _isEquationVisible;
    default:
        return TextBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TempoText::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::TEMPO:
        setTempo(v.toDouble());
        score()->setTempo(segment(), _playbackTempo);
        score()->fixTicks();
        break;
    case Pid::TEMPO_FOLLOW_TEXT:
        _followText = v.toBool();
        break;
    case Pid::TEMPO_EQUATION:
        if (isEquationValid(v.toString())) {
            _equation = v.toString();
            parseEquation();
        }
        break;
    case Pid::TEMPO_EQUATION_VISIBLE:
        _isEquationVisible = v.toBool();
        parseEquation();
        break;
    default:
        if (!TextBase::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TempoText::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SUB_STYLE:
        return int(Tid::TEMPO);
    case Pid::TEMPO:
        return 2.0;
    case Pid::TEMPO_FOLLOW_TEXT:
        return false;
    case Pid::TEMPO_EQUATION:
        return "q = 120";
    case Pid::TEMPO_EQUATION_VISIBLE:
        return true;
    default:
        return TextBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   layout
//    called after Measure->stretchMeasure()
//---------------------------------------------------------

void TempoText::layout()
{
    TextBase::layout();

    Segment* s = segment();
    if (!s) {                       // for use in palette
        return;
    }

    // tempo text on first chordrest of measure should align over time sig if present
    //
    if (autoplace() && s->rtick().isZero()) {
        Segment* p = segment()->prev(SegmentType::TimeSig);
        if (p) {
            rxpos() -= s->x() - p->x();
            Element* e = p->element(staffIdx() * VOICES);
            if (e) {
                rxpos() += e->x();
            }
        }
    }
    autoplaceSegmentElement();
}

//---------------------------------------------------------
//   parseEquation
//---------------------------------------------------------

void TempoText::parseEquation()
{
    static const QRegularExpression equationExpression("[\\[\\(]?[\uECA0-\uECB7]+ *= *[\uECA0-\uECB70-9]+[\\]\\)]?");

    static const std::map<QString, QString> equationMap = {
        { "d", "\uECA0" },
        { "w", "\uECA2" },
        { "h", "\uECA3" },
        { "q", "\uECA5" },
        { "e", "\uECA7" },
        { "s", "\uECA9" },
        { "t", "\uECAB" },
        { ".", "\uECB7" }
    };

    if (_isEquationVisible) {
        QString equation = _equation;

        for (auto replacement : equationMap) {
            equation.replace(replacement.first, replacement.second);
        }

        if (equationExpression.match(xmlText()).hasMatch()) {
            setXmlText(xmlText().replace(equationExpression, equation));
        } else {
            setXmlText(xmlText() + equation);
        }
    } else {
        setXmlText(xmlText().replace(equationExpression, ""));
    }

    if (_followText) {
        updateTempo();
    }
}

//---------------------------------------------------------
//   isEquationValid
//---------------------------------------------------------

bool TempoText::isEquationValid(const QString equation) const
{
    const QRegularExpression equationExpression("^[\\[,\\(]?[ ]*([dwhqest].*) *= *(\\d+|[dwhqest].*)[\\],\\)]?$");
    QRegularExpressionMatch match = equationExpression.match(equation);

    return match.hasMatch();
}

//---------------------------------------------------------
//   getRelativeDuration
//---------------------------------------------------------

float TempoText::getRelativeDuration(const QString marking) const
{
    static const std::map<QString, float> durationMap = {
        { "d", 8.0f },
        { "w", 4.0f },
        { "h", 2.0f },
        { "q", 1.0f },
        { "e", 0.5f },
        { "s", 0.25f },
        { "t", 0.125f },
    };

    float baseDuration = 1.0f;

    for (auto duration : durationMap) {
        if (marking[0] == duration.first) {
            baseDuration = duration.second;
            break;
        }
    }

    const int dots = marking.count(".");

    return baseDuration * std::powf(1.5, dots);
}

//---------------------------------------------------------
//   duration2userName
//---------------------------------------------------------

QString TempoText::duration2userName(const TDuration t)
{
    QString dots;
    switch (t.dots()) {
    case 1: dots = QObject::tr("Dotted %1").arg(t.durationTypeUserName());
        break;
    case 2: dots = QObject::tr("Double dotted %1").arg(t.durationTypeUserName());
        break;
    case 3: dots = QObject::tr("Triple dotted %1").arg(t.durationTypeUserName());
        break;
    case 4: dots = QObject::tr("Quadruple dotted %1").arg(t.durationTypeUserName());
        break;
    default:
        dots = t.durationTypeUserName();
        break;
    }
    return dots;
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString TempoText::accessibleInfo() const
{
    TDuration t1;
    TDuration t2;
    int len1;
    int len2;
    QString text = plainText();
    QString firstPart = text.split(" = ").first();
    QString secondPart = text.split(" = ").back();
    int x1 = findTempoDuration(firstPart, len1, t1);
    int x2 = -1;
    if (_relative) {
        x2 = findTempoDuration(secondPart, len2, t2);
    }

    if (x1 != -1) {
        QString dots1;
        QString dots2;
        dots1 = duration2userName(t1);
        if (x2 != -1) {
            dots2 = duration2userName(t2);
            return QString("%1: %2 %3 = %4 %5").arg(Element::accessibleInfo(), dots1, QObject::tr("note"), dots2, QObject::tr("note"));
        } else {
            return QString("%1: %2 %3 = %4").arg(Element::accessibleInfo(), dots1, QObject::tr("note"), secondPart);
        }
    } else {
        return TextBase::accessibleInfo();
    }
}
}
