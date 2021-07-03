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
#include <QRegularExpression>

#include "io/xml.h"

#include "measure.h"
#include "musescoreCore.h"
#include "score.h"
#include "staff.h"
#include "system.h"
#include "tempo.h"
#include "tempotext.h"
#include "undo.h"
#include "textedit.h"

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
    _playbackTempo = 2.0;            // propertyDefault(P_TEMPO).toDouble();
    _notatedTempo = 120;
    _followText = true;
    _relative = 1.0;
    _isRelative = false;
    _equation = "q = 120";
    _isEquationVisible = true;
    _lastEquationIndex = -1;
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
    xml.tag("equationVisible", _isEquationVisible);
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
        } else if (tag == "equationVisible") {
            _isEquationVisible = e.readBool();
        } else if (!TextBase::readProperties(e)) {
            e.unknown();
        }
    }
    // check sanity
    if (xmlText().isEmpty()) {
        _equation = QString("q = %1").arg(lrint(60 * _playbackTempo));
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

struct EquationDescrption
{
    QString letter;

    QString unicode;
    QString symbol;

    float relative;
    TDuration::DurationType duration;

    EquationDescrption(QString l, float r, QString u, QString s, TDuration::DurationType val)
        : letter(l), unicode(u), symbol(s), relative(r), duration(val)
    {
    }
};

static const EquationDescrption durationMap[] = {
    { "d",  8.0f,   "\uECA0", "<sym>metNoteDoubleWhole</sym>",  TDuration::DurationType::V_BREVE },
    { "w",  4.0f,   "\uECA2", "<sym>metNoteWhole</sym>",        TDuration::DurationType::V_WHOLE },
    { "h",  2.0f,   "\uECA3", "<sym>metNoteHalfUp</sym>",       TDuration::DurationType::V_HALF },
    { "q",  1.0f,   "\uECA5", "<sym>metNoteQuarterUp</sym>",    TDuration::DurationType::V_QUARTER },
    { "e",  0.5f,   "\uECA7", "<sym>metNote8thUp</sym>",        TDuration::DurationType::V_EIGHTH },
    { "s",  0.25f,  "\uECA9", "<sym>metNote16thUp</sym>",       TDuration::DurationType::V_16TH },
    { "t",  0.125f, "\uECAB", "<sym>metNote32ndUp</sym>",       TDuration::DurationType::V_32ND },
    { ".",  0,      "\uECB7", "<sym>metAugmentationDot</sym>",  TDuration::DurationType::V_32ND },
};

static const QString allowedEquationCodes = "dwhqest";

QString TempoText::regexGroup(bool symbol)
{
    static QString unicodeString;
    static QString symbolString;

    if (symbol && !symbolString.isEmpty()) {
        return symbolString;
    } else if (!symbol && !unicodeString.isEmpty()) {
        return unicodeString;
    }

    for (auto pattern : durationMap) {
        unicodeString += pattern.unicode;
        symbolString += QString(pattern.symbol).replace("/", "\\/") + "|";
    }

    unicodeString = "[" + unicodeString + "]";
    symbolString = "(" + symbolString.left(symbolString.size() - 1) + ")";

    return symbol ? symbolString : unicodeString;
}

//---------------------------------------------------------
//   findTempoDuration
//    find the duration part (note + dot) of a tempo text in string s
//    return the match position or -1 if not found
//    set len to the match length and dur to the duration value
//---------------------------------------------------------

int TempoText::findTempoDuration(const QString& s, int& len, TDuration& dur)
{
    static const QRegularExpression tempoExpression(QString("(?<pattern>%1+)").arg(TempoText::regexGroup(false)));
    QRegularExpressionMatch match = tempoExpression.match(s);
    if (match.hasMatch()) {
        len = match.capturedLength();
        dur = TempoText::findTempoDuration(match.captured("pattern"));
        return match.capturedStart();
    }

    return -1;
}

TDuration TempoText::findTempoDuration(const QString& s)
{
    static const QRegularExpression tempoExpression(QString("(?<equation>%1)(?<dots>\uECB7*)").arg(TempoText::regexGroup(false)));
    QRegularExpressionMatch match = tempoExpression.match(s);

    if (!match.hasMatch()) {
        return TDuration(TDuration::DurationType::V_INVALID);
    }

    for (auto pattern : durationMap) {
        if (pattern.unicode == match.captured("equation")) {
            TDuration duration(pattern.duration);
            duration.setDots(match.captured("dots").size());
            return duration;
        }
    }

    return TDuration(TDuration::DurationType::V_INVALID);
}

TDuration TempoText::duration() const
{
    return findTempoDuration(TempoText::mapEquationToText(_equation, false).split("=")[0]);
}

//---------------------------------------------------------
//   duration2tempoTextString
//    find the tempoText string representation for duration
//---------------------------------------------------------

QString TempoText::duration2tempoTextString(const TDuration dur)
{
    QString tempoString = "q";

    for (auto pattern : durationMap) {
        if (pattern.duration == dur.type()) {
            tempoString = pattern.letter;
            break;
        }
    }

    for (int i = 0; i < dur.dots(); i++) {
        tempoString += ".";
    }

    return tempoString;
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
        cursor->movePosition(TextCursor::MoveOperation::Left, TextCursor::MoveMode::MoveAnchor, cursorIndex - eqnIndices.first);
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
    int lineIndex = plainText().indexOf("\n");

    while (lineIndex < index && row < xmlText().count("\n")) {
        lineIndex = plainText().indexOf("\n", lineIndex);
        row++;
    }

    int column = index - lineIndex - 1;

    return std::make_pair(row, column);
}

std::pair<int, int> TempoText::equationIndices() const
{
    static const QRegularExpression equationExpression(QString("(?<equation>[\\[\\(]?%1+ *= *(%1+|[0-9]+)[\\]\\)]?)").arg(TempoText::
                                                                                                                          regexGroup(false)));
    QRegularExpressionMatch match = equationExpression.match(plainText());

    if (!match.hasMatch()) {
        return std::make_pair(0, 0);
    }

    return std::make_pair(match.capturedStart("equation"), match.capturedEnd("equation"));
}

bool TempoText::moveCursor(TextCursor* cursor, int key, bool ctrlPressed, TextCursor::MoveMode moveMode) const
{
    int cursorIndex = textIndexFromCursor(cursor->row(), cursor->column());

    std::pair<int, int> eqnIndices = equationIndices();

    if (eqnIndices.first == 0 && eqnIndices.second == 0) {
        return TextBase::moveCursor(cursor, key, ctrlPressed, moveMode);
    }

    int length = eqnIndices.second - eqnIndices.first;

    if (eqnIndices.first == cursorIndex && key == Qt::Key_Right) {
        return cursor->movePosition(TextCursor::MoveOperation::Right, TextCursor::MoveMode::MoveAnchor, length);
    } else if (eqnIndices.second == cursorIndex && key == Qt::Key_Left) {
        return cursor->movePosition(TextCursor::MoveOperation::Left, TextCursor::MoveMode::MoveAnchor, length);
    } else {
        if (key == Qt::Key_Left) {
            return cursor->movePosition(ctrlPressed ? TextCursor::MoveOperation::WordLeft : TextCursor::MoveOperation::Left, moveMode);
        } else if (key == Qt::Key_Right) {
            return cursor->movePosition(ctrlPressed ? TextCursor::MoveOperation::NextWord : TextCursor::MoveOperation::Right, moveMode);
        }
    }

    return false;
}

bool TempoText::canDelete(TextCursor* cursor, int key) const
{
    int cursorIndex = textIndexFromCursor(cursor->row(), cursor->column());

    std::pair<int, int> eqnIndices = equationIndices();

    if (eqnIndices.first == 0 && eqnIndices.second == 0) {
        return true;
    }

    if (eqnIndices.first == cursorIndex && key == Qt::Key_Delete) {
        return false;
    } else if (eqnIndices.second == cursorIndex && key == Qt::Key_Backspace) {
        return false;
    }

    return true;
}

void TempoText::dragTo(EditData& ed)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this));
    TextCursor* cursor = ted->cursor();

    cursor->set(ed.pos, TextCursor::MoveMode::KeepAnchor);

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

    static const QRegularExpression bpmExpression(QString("^[\\[,\\(]?[ ]*(?<note>[%1].*) *= *(?<bpm>\\d+)[\\],\\)]?$").arg(
                                                      allowedEquationCodes));
    static const QRegularExpression relativeExpression(QString("^[\\[,\\(]?[ ]*(?<note1>[%1].*) *= *(?<note2>[%1].*)[\\],\\)]?$").arg(
                                                           allowedEquationCodes));

    QRegularExpressionMatch bpmMatch = bpmExpression.match(_equation);
    QRegularExpressionMatch relativeMatch = relativeExpression.match(_equation);

    if (bpmMatch.hasMatch()) {
        const float relativeDuration = TempoText::getRelativeDuration(bpmMatch.captured("note"));
        const float bpm = bpmMatch.captured("bpm").toFloat();

        _notatedTempo = bpm;

        qreal playbackTempo(relativeDuration * bpm * quarterNotePlayback);

        if (playbackTempo != _playbackTempo) {
            if (segment()) {
                undoChangeProperty(Pid::TEMPO, QVariant(playbackTempo), propertyFlags(Pid::TEMPO));
            }

            _playbackTempo = playbackTempo;

            _relative = 1.0;
            _isRelative = false;

            updateScore();
        }
    } else if (relativeMatch.hasMatch()) {
        _relative = TempoText::getRelativeDuration(relativeMatch.captured("note2"))
                    / TempoText::getRelativeDuration(relativeMatch.captured("note1"));
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

void TempoText::setEquationFromTempo(int tempo)
{
    _equation = QString("q = %1").arg(tempo);
    parseEquation();
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

        if (!_isEquationVisible) {
            _lastEquationIndex = equationIndices().first;
        }

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
    static const QString equationBase = "[\\[\\(]?%1+ *= *(%1+|\\d+)[\\]\\)]?";
    static const QRegularExpression equationExpression(equationBase.arg(TempoText::regexGroup()));
    static const QRegularExpression unicodeEquationExpression(equationBase.arg(TempoText::regexGroup(false)));

    if (_isEquationVisible) {
        QString equation = TempoText::mapEquationToText(_equation);

        if (equationExpression.match(xmlText()).hasMatch()) {
            setXmlText(xmlText().replace(equationExpression, equation));
        } else if (unicodeEquationExpression.match(plainText()).hasMatch()) {
            auto split = plainText().split(unicodeEquationExpression);
            setXmlText(split[0] + equation + split[1]);
        } else {
            setXmlText(xmlText().insert(_lastEquationIndex == -1 ? xmlText().size() : _lastEquationIndex, equation));
        }
    } else {
        if (equationExpression.match(xmlText()).hasMatch()) {
            setXmlText(xmlText().replace(equationExpression, ""));
        } else if (unicodeEquationExpression.match(plainText()).hasMatch()) {
            auto split = plainText().split(unicodeEquationExpression);
            setXmlText(split[0] + split[1]);
        }
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
    const QRegularExpression equationExpression(QString("^[\\[,\\(]?[ ]*([%1].*) *= *(\\d+|[%1].*)[\\],\\)]?$").arg(allowedEquationCodes));
    QRegularExpressionMatch match = equationExpression.match(equation);

    return match.hasMatch();
}

//---------------------------------------------------------
//   mapEquationToText
//---------------------------------------------------------
QString TempoText::mapEquationToText(const QString equation, bool symbol)
{
    QString mapped;

    for (auto c : equation) {
        bool added = false;

        for (auto pattern : durationMap) {
            if (pattern.letter == c) {
                mapped += (symbol ? pattern.symbol : pattern.unicode);
                added = true;
                break;
            }
        }

        if (!added) {
            mapped += c;
        }
    }

    return mapped;
}

//---------------------------------------------------------
//   getRelativeDuration
//---------------------------------------------------------

float TempoText::getRelativeDuration(const QString marking)
{
    float baseDuration = 1.0f;

    for (auto duration : durationMap) {
        if (marking[0] == duration.letter) {
            baseDuration = duration.relative;
            break;
        }
    }

    const int dots = marking.count(".");

    return baseDuration * std::pow(3, dots) / std::pow(2, dots);
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
