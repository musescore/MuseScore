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

#include "notationbraille.h"

#include "translation.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tie.h"

#include "braille.h"
#include "braillecode.h"
#include "brailleinput.h"
#include "brailleinputparser.h"
#include "louis.h"

using namespace mu::braille;
using namespace mu::io;
using namespace mu::notation;

namespace mu::engraving {
void NotationBraille::init()
{
    setEnabled(brailleConfiguration()->braillePanelEnabled());
    setCurrentItemPosition(-1, -1);

    setMode(BrailleMode::Navigation);

    path_t tablesdir = tablesDefaultDirPath();
    setTablesDir(tablesdir.toStdString().c_str());
    initTables(tablesdir.toStdString());

    std::string welcome = braille_translate(table_for_literature.c_str(), "Welcome to MuseScore 4!");
    setBrailleInfo(QString(welcome.c_str()));

    brailleConfiguration()->braillePanelEnabledChanged().onNotify(this, [this]() {
        bool enabled = brailleConfiguration()->braillePanelEnabled();
        setEnabled(enabled);
    });

    updateTableForLyricsFromPreferences();
    brailleConfiguration()->brailleTableChanged().onNotify(this, [this]() {
        updateTableForLyricsFromPreferences();
    });

    setIntervalDirection(brailleConfiguration()->intervalDirection());
    brailleConfiguration()->intervalDirectionChanged().onNotify(this, [this]() {
        BrailleIntervalDirection direction = brailleConfiguration()->intervalDirection();
        setIntervalDirection(direction);
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        if (notation()) {
            notation()->interaction()->selectionChanged().onNotify(this, [this]() {
                doBraille();
            });

            notation()->notationChanged().onNotify(this, [this]() {
                setCurrentItemPosition(0, 0);
                doBraille(true);
            });

            notation()->interaction()->noteInput()->stateChanged().onNotify(this, [this]() {
                if (interaction()->noteInput()->isNoteInputMode()) {
                    if (isNavigationMode()) {
                        setMode(BrailleMode::BrailleInput);
                    }
                } else {
                    if (isBrailleInputMode()) {
                        setMode(BrailleMode::Navigation);
                    }
                }
            });

            notation()->interaction()->noteInput()->noteAdded().onNotify(this, [this]() {
                if (currentEngravingItem() && currentEngravingItem()->isNote()) {
                    LOGD() << "Note added: " << currentEngravingItem()->accessibleInfo();
                    Note* note = toNote(currentEngravingItem());
                    brailleInput()->setOctave(note->octave());
                }
            });
        }
    });
}

void NotationBraille::updateTableForLyricsFromPreferences()
{
    QString table = brailleConfiguration()->brailleTable();
    int startPos = table.indexOf('[');
    int endPos = table.indexOf(']');
    if (startPos != -1 && endPos != -1) {
        table = table.mid(startPos + 1, endPos - startPos - 1);
        QString table_full_path = QString::fromStdString(tables_dir) + "/" + table;
        if (check_tables(table_full_path.toStdString().c_str()) == 0) {
            updateTableForLyrics(table.toStdString());
        } else {
            LOGD() << "Table check error!";
        }
    }
}

void NotationBraille::doBraille(bool force)
{
    if (brailleConfiguration()->braillePanelEnabled()) {
        EngravingItem* e = nullptr;
        Measure* m = nullptr;

        if (selection()->isSingle()) {
            e = selection()->element();
            LOGD() << " selected " << e->accessibleInfo();
            m = e->findMeasure();
        } else if (selection()->isRange()) {
            for (auto el: selection()->elements()) {
                if (el->isMeasure()) {
                    m = toMeasure(el);
                    break;
                } else {
                    m = el->findMeasure();
                    if (m) {
                        break;
                    }
                }
            }
            e = m ? m : selection()->elements().front();
        } else if (selection()->isList()) {
            e = selection()->elements().front();
            m = e->findMeasure();
        }
        if (e) {
            setCurrentEngravingItem(e, false);

            if (!m) {
                brailleEngravingItemList()->clear();
                Braille lb(score());
                bool res = lb.convertItem(e, brailleEngravingItemList());
                if (!res) {
                    QString txt = e->accessibleInfo();
                    std::string braille = braille_long_translate(table_for_literature.c_str(), txt.toStdString());
                    brailleEngravingItemList()->setBrailleStr(QString::fromStdString(braille));
                    setBrailleInfo(QString::fromStdString(braille));
                } else {
                    setBrailleInfo(brailleEngravingItemList()->brailleStr());
                }
                current_measure = nullptr;
            } else {
                if (m != current_measure || force) {
                    brailleEngravingItemList()->clear();
                    Braille lb(score());
                    lb.convertMeasure(m, brailleEngravingItemList());
                    setBrailleInfo(brailleEngravingItemList()->brailleStr());
                    current_measure = m;
                }
                current_bei = brailleEngravingItemList()->getItem(e);
                if (current_bei != nullptr) {
                    setCurrentItemPosition(current_bei->start(), current_bei->end());
                }
            }
        }
    }
}

mu::engraving::Score* NotationBraille::score()
{
    return notation()->elements()->msScore()->score();
}

mu::engraving::Selection* NotationBraille::selection()
{
    return &score()->selection();
}

mu::ValCh<std::string> NotationBraille::brailleInfo() const
{
    return m_brailleInfo;
}

mu::ValCh<int> NotationBraille::cursorPosition() const
{
    return m_cursorPosition;
}

mu::ValCh<int> NotationBraille::currentItemPositionStart() const
{
    return m_currentItemPositionStart;
}

mu::ValCh<int> NotationBraille::currentItemPositionEnd() const
{
    return m_currentItemPositionEnd;
}

mu::ValCh<std::string> NotationBraille::keys() const
{
    return m_keys;
}

mu::ValCh<bool> NotationBraille::enabled() const
{
    return m_enabled;
}

mu::ValCh<BrailleIntervalDirection> NotationBraille::intervalDirection() const
{
    return m_intervalDirection;
}

mu::ValCh<int> NotationBraille::mode() const
{
    return m_mode;
}

mu::ValCh<std::string> NotationBraille::cursorColor() const
{
    return m_cursorColor;
}

void NotationBraille::setEnabled(bool enabled)
{
    if (enabled == m_enabled.val) {
        return;
    }
    m_enabled.set(enabled);
}

void NotationBraille::setIntervalDirection(const BrailleIntervalDirection direction)
{
    if (direction == m_intervalDirection.val) {
        return;
    }
    m_intervalDirection.set(direction);
}

BrailleEngravingItemList* NotationBraille::brailleEngravingItemList()
{
    return &m_beil;
}

QString NotationBraille::getBrailleStr()
{
    return m_beil.brailleStr();
}

BrailleInputState* NotationBraille::brailleInput()
{
    return &m_braille_input;
}

void NotationBraille::setBrailleInfo(const QString& info)
{
    std::string infoStd = info.toStdString();

    if (m_brailleInfo.val == infoStd) {
        return;
    }

    m_brailleInfo.set(infoStd);
}

void NotationBraille::setCursorPosition(const int pos)
{
    if (!isNavigationMode()) {
        return;
    }

    if (m_cursorPosition.val == pos || pos == 0) {
        return;
    }

    m_cursorPosition.set(pos);

    current_bei = brailleEngravingItemList()->getItem(pos);
    if (current_bei != NULL && current_bei->el() != NULL
        && (current_bei->type() == BEIType::EngravingItem
            || current_bei->type() == BEIType::LyricItem)) {
        setCurrentEngravingItem(current_bei->el(), true);
    }
}

void NotationBraille::setCurrentItemPosition(const int start, const int end)
{
    if (m_currentItemPositionStart.val == start
        && m_currentItemPositionEnd.val == end) {
        return;
    }

    m_currentItemPositionStart.set(start);
    m_currentItemPositionEnd.set(end);
}

INotationPtr NotationBraille::notation()
{
    return globalContext()->currentNotation();
}

INotationInteractionPtr NotationBraille::interaction()
{
    return notation() ? notation()->interaction() : nullptr;
}

DurationType getDuration(const QString key)
{
    static const DurationType types[] = {
        DurationType::V_64TH, DurationType::V_32ND, DurationType::V_16TH,
        DurationType::V_EIGHTH, DurationType::V_QUARTER, DurationType::V_HALF,
        DurationType::V_WHOLE, DurationType::V_BREVE, DurationType::V_LONG
    };
    bool is_ok = false;
    int val = key.toInt(&is_ok);

    if (!is_ok || val < 1 || val > 9) {
        return DurationType::V_INVALID;
    } else {
        return types[val - 1];
    }
}

void NotationBraille::setInputNoteDuration(Duration d)
{
    InputState& inputState = score()->inputState();
    inputState.setDuration(d);
    score()->setInputState(inputState);
    brailleInput()->setCurrentDuration(d.type());
}

void NotationBraille::setTupletDuration(int tuplet, Duration d)
{
    LOGD() << tuplet << " " << (int)d.type();
    brailleInput()->setCurrentDuration(d.type());

    switch (tuplet) {
    case 2:
        d = d.shiftRetainDots(-1, false);
        d = d.shiftRetainDots(-1, true);
        break;
    case 3:
        d = d.shiftRetainDots(-1, false);
        break;
    case 5: case 6: case 7:
        d = d.shiftRetainDots(-2, false);
        break;
    case 8:
        d = d.shiftRetainDots(-2, false);
        d = d.shiftRetainDots(-1, true);
        break;
    case 9:
        d = d.shiftRetainDots(-3, false);
        break;
    }
    InputState& inputState = score()->inputState();
    inputState.setDuration(d);
    score()->setInputState(inputState);
}

static TupletOptions makeTupletOption(int num)
{
    TupletOptions option;
    switch (num) {
    case 2:
        option.ratio = { 2, 3 };
        break;
    case 3:
        option.ratio = { 3, 2 };
        break;
    case 5:
        option.ratio = { 5, 4 };
        break;
    case 6:
        option.ratio = { 6, 4 };
        break;
    case 7:
        option.ratio = { 7, 4 };
        break;
    case 8:
        option.ratio = { 8, 6 };
        break;
    case 9:
        option.ratio = { 9, 8 };
        break;
    default:
        break;
    }
    return option;
}

bool matchPattern(const std::string sequence, const std::string pattern)
{
    QStringList seqs = QString::fromStdString(sequence).split("+");
    QStringList pats = QString::fromStdString(pattern).split("+");

    if (seqs.length() != pats.length()) {
        return false;
    }
    seqs.sort();
    pats.sort();

    for (int i = 0; i < seqs.length(); i++) {
        if (seqs.at(i) != pats.at(i)) {
            return false;
        }
    }
    return true;
}

void NotationBraille::setKeys(const QString& sequence)
{
    LOGD() << sequence;
    std::string seq = sequence.toStdString();
    m_keys.set(seq);

    if (seq == "Left") {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::Chord);
    } else if (seq == "Right") {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::Chord);
    } else if (matchPattern(seq, "Ctrl+Left")) {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::Measure);
    } else if (matchPattern(seq, "Ctrl+Right")) {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::Measure);
    } else if (matchPattern(seq, "Alt+Left")) {
        interaction()->moveSelection(MoveDirection::Left, MoveSelectionType::EngravingItem);
    } else if (matchPattern(seq, "Alt+Right")) {
        interaction()->moveSelection(MoveDirection::Right, MoveSelectionType::EngravingItem);
    } else if (matchPattern(seq, "Ctrl+End")) {
        interaction()->selectLastElement();
    } else if (matchPattern(seq, "Ctrl+Home")) {
        interaction()->selectFirstElement();
    } else if (seq == "Delete") {
        if (currentEngravingItem()) {
            interaction()->deleteSelection();
            if (!brailleInput()->intervals().empty()) {
                brailleInput()->removeLastInterval();
            }
        }
    } else if (seq == "N") {
        toggleMode();
        brailleInput()->initialize();
    } else if (seq == "Plus") {
        if (isBrailleInputMode()) {
            interaction()->noteInput()->doubleNoteInputDuration();
        }
    } else if (seq == "Minus") {
        if (isBrailleInputMode()) {
            interaction()->noteInput()->halveNoteInputDuration();
        }
    } else if (matchPattern(seq, "Space+F") && isBrailleInputMode()) {
        brailleInput()->setNoteGroup(NoteGroup::Group1);
        brailleInput()->resetBuffer();
    } else if (matchPattern(seq, "Space+D") && isBrailleInputMode()) {
        brailleInput()->setNoteGroup(NoteGroup::Group2);
        brailleInput()->resetBuffer();
    } else if (matchPattern(seq, "Space+S") && isBrailleInputMode()) {
        brailleInput()->setNoteGroup(NoteGroup::Group3);
        brailleInput()->resetBuffer();
    } else if (matchPattern(seq, "Space+S+D+J+K") && isBrailleInputMode()) {
        LOGD() << "tuplet input";
        brailleInput()->setTupletIndicator(true);
        brailleInput()->resetBuffer();
    } else if (seq == "Space") {
        brailleInput()->reset();
    } else if (isBrailleInputMode() && !sequence.isEmpty()) {
        QString pattern = parseBrailleKeyInput(sequence);
        if (!pattern.isEmpty()) {
            brailleInput()->insertToBuffer(pattern);
        } else {
            brailleInput()->insertToBuffer(sequence);
        }
        LOGD() << brailleInput()->buffer();
        std::string braille = translate2Braille(brailleInput()->buffer().toStdString());
        BieRecognize(braille, brailleInput()->tupletIndicator());

        NoteName prevNoteName = NoteName::C;
        int prevNoteOctave = -1; // unknown octave

        if (Segment* seg = interaction()->noteInput()->state().segment) {
            const track_idx_t track = interaction()->noteInput()->state().currentTrack;
            Chord* prevChord = nullptr;

            for (Segment* s = seg->prev1(SegmentType::ChordRest); s; s = s->prev1(SegmentType::ChordRest)) {
                EngravingItem* e = s->element(track);
                if (e && e->isChord()) {
                    prevChord = toChord(e);
                    break;
                }
            }

            if (prevChord) {
                Note* rootNote = (currentIntervalDirection() == IntervalDirection::Up) ? prevChord->downNote() : prevChord->upNote();
                String pitchName;
                String accidental; // needed for tpc2name
                tpc2name(rootNote->tpc(), NoteSpellingType::STANDARD, NoteCaseType::UPPER, pitchName, accidental);
                size_t inote = String("CDEFGAB").indexOf(pitchName.at(0));
                IF_ASSERT_FAILED(inote <= 6) {
                } else {
                    prevNoteName = static_cast<NoteName>(inote);
                    prevNoteOctave = rootNote->octave();
                }
            }
        }

        brailleInput()->setNoteName(prevNoteName, true);
        brailleInput()->setOctave(prevNoteOctave, true);

        BieSequencePatternType type = brailleInput()->parseBraille(currentIntervalDirection());
        switch (type) {
        case BieSequencePatternType::Note: {
            LOGD() << "note";
            if (brailleInput()->accidental() != mu::notation::AccidentalType::NONE) {
                interaction()->noteInput()->setAccidental(brailleInput()->accidental());
            }

            setVoice(brailleInput()->accord());

            if (brailleInput()->tupletNumber() != -1
                && brailleInput()->tupletDuration().type() != DurationType::V_INVALID) {
                Duration duration = brailleInput()->tupletDuration();
                int tuplet = brailleInput()->tupletNumber();
                setTupletDuration(tuplet, duration);
                TupletOptions option = makeTupletOption(brailleInput()->tupletNumber());
                interaction()->noteInput()->addTuplet(option);
                brailleInput()->clearTuplet();
            }

            DurationType d = brailleInput()->getCloseDuration();
            Duration duration = Duration(d);
            setInputNoteDuration(duration);

            interaction()->noteInput()->addNote(brailleInput()->noteName(), NoteAddingMode::NextChord);

            if (brailleInput()->addedOctave() != -1) {
                if (brailleInput()->addedOctave() < brailleInput()->octave()) {
                    for (int i = brailleInput()->addedOctave(); i < brailleInput()->octave(); i++) {
                        interaction()->movePitch(MoveDirection::Down, PitchMode::OCTAVE);
                    }
                } else if (brailleInput()->addedOctave() > brailleInput()->octave()) {
                    for (int i = brailleInput()->octave(); i < brailleInput()->addedOctave(); i++) {
                        interaction()->movePitch(MoveDirection::Up, PitchMode::OCTAVE);
                    }
                }
                brailleInput()->setOctave(brailleInput()->addedOctave(), true);
            }

            if (brailleInput()->longSlurStart()) {
                if (brailleInput()->longSlurStartNote() == NULL) {
                    if (currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
                        Note* note = toNote(currentEngravingItem());
                        brailleInput()->setLongSlurStartNote(note);
                    }
                }
            }

            if (brailleInput()->tieStartNote() != NULL) {
                addTie();
                brailleInput()->clearTie();
            }

            if (brailleInput()->slurStartNote() != NULL) {
                addSlur();
                brailleInput()->clearSlur();
            }

            playbackController()->playElements({ currentEngravingItem() });
            brailleInput()->reset();
            break;
        }
        case BieSequencePatternType::Rest: {
            LOGD() << "rest";
            DurationType d = brailleInput()->getCloseDuration();
            Duration duration = Duration(d);
            if (brailleInput()->dots() > 0) {
                duration.setDots(brailleInput()->dots());
            }
            setInputNoteDuration(Duration(duration));
            interaction()->putRest(duration);
            brailleInput()->reset();
            break;
        }
        case BieSequencePatternType::Interval: {
            LOGD() << "interval";
            if (brailleInput()->accidental() != mu::notation::AccidentalType::NONE) {
                interaction()->noteInput()->setAccidental(brailleInput()->accidental());
            }
            interaction()->noteInput()->addNote(brailleInput()->noteName(), NoteAddingMode::CurrentChord);

            if (brailleInput()->addedOctave() != -1) {
                if (brailleInput()->addedOctave() < brailleInput()->octave()) {
                    for (int i = brailleInput()->addedOctave(); i < brailleInput()->octave(); i++) {
                        interaction()->movePitch(MoveDirection::Down, PitchMode::OCTAVE);
                    }
                } else if (brailleInput()->addedOctave() > brailleInput()->octave()) {
                    for (int i = brailleInput()->octave(); i < brailleInput()->addedOctave(); i++) {
                        interaction()->movePitch(MoveDirection::Up, PitchMode::OCTAVE);
                    }
                }
                brailleInput()->setOctave(brailleInput()->addedOctave());
            }
            playbackController()->playElements({ currentEngravingItem() });
            brailleInput()->reset();
            break;
        }
        case BieSequencePatternType::Tuplet: case BieSequencePatternType::Tuplet3: {
            LOGD() << "tuplet";
            std::string stateTuplet;
            stateTuplet = "Tuplet " + std::to_string(brailleInput()->tupletNumber());
            auto notationAccessibility = notation()->accessibility();
            if (!notationAccessibility) {
                return;
            }
            notationAccessibility->setTriggeredCommand(stateTuplet);
            break;
        }
        case BieSequencePatternType::Tie: {
            LOGD() << "tie";
            if (brailleInput()->tie()) {
                if (currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
                    Note* note = toNote(currentEngravingItem());
                    brailleInput()->setTieStartNote(note);
                }
            }
            brailleInput()->reset();
            break;
        }
        case BieSequencePatternType::NoteSlur: {
            LOGD() << "note slur";
            if (brailleInput()->noteSlur()) {
                if (brailleInput()->slurStartNote() == NULL) {
                    if (currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
                        Note* note = toNote(currentEngravingItem());
                        brailleInput()->setSlurStartNote(note);
                    }
                }
            }
            brailleInput()->reset();
            break;
        }
        case BieSequencePatternType::LongSlurStop: {
            LOGD() << "long slur stop";
            if (brailleInput()->longSlurStop()) {
                if (brailleInput()->longSlurStartNote() != NULL) {
                    addLongSlur();
                    brailleInput()->clearLongSlur();
                }
            }
            brailleInput()->reset();
            break;
        }
        case BieSequencePatternType::Dot: {
            LOGD() << "dot " << brailleInput()->dots();
            if (brailleInput()->dots() > 0) {
                interaction()->increaseDecreaseDuration(-brailleInput()->dots(), true);
            }
            brailleInput()->reset();
            break;
        }
        default: {
            // TODO
        }
        }
    } else if (isBrailleInputMode() && !sequence.isEmpty()) {
        QString pattern = parseBrailleKeyInput(sequence);
        if (!pattern.isEmpty()) {
            brailleInput()->insertToBuffer(pattern);
        } else {
            brailleInput()->insertToBuffer(sequence);
        }
    }
}

bool NotationBraille::addTie()
{
    if (!brailleInput()->tieStartNote()) {
        return false;
    }

    if (!currentEngravingItem() || !currentEngravingItem()->isNote()) {
        return false;
    }

    score()->startCmd();
    Note* note = toNote(currentEngravingItem());

    Tie* tie = Factory::createTie(score()->dummy());
    tie->setStartNote(brailleInput()->tieStartNote());
    tie->setEndNote(note);
    tie->setTrack(brailleInput()->tieStartNote()->track());
    tie->setTick(brailleInput()->tieStartNote()->chord()->segment()->tick());
    tie->setTicks(note->chord()->segment()->tick() - brailleInput()->tieStartNote()->chord()->segment()->tick());
    score()->undoAddElement(tie);
    score()->endCmd();
    return true;
}

bool NotationBraille::addSlur()
{
    if (brailleInput()->slurStartNote() != NULL
        && currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
        Note* note1 = brailleInput()->slurStartNote();
        Note* note2 = toNote(currentEngravingItem());

        if (note1->parent()->isChordRest() && note2->parent()->isChordRest()) {
            LOGD() << "add slur";
            ChordRest* firstChordRest = toChordRest(note1->parent());
            ChordRest* secondChordRest = toChordRest(note2->parent());

            score()->startCmd();

            Slur* slur = Factory::createSlur(firstChordRest->measure()->system());
            slur->setScore(firstChordRest->score());
            slur->setTick(firstChordRest->tick());
            slur->setTick2(secondChordRest->tick());
            slur->setTrack(firstChordRest->track());
            if (secondChordRest->staff()->part() == firstChordRest->staff()->part()
                && !secondChordRest->staff()->isLinked(firstChordRest->staff())) {
                slur->setTrack2(secondChordRest->track());
            } else {
                slur->setTrack2(firstChordRest->track());
            }
            slur->setStartElement(firstChordRest);
            slur->setEndElement(secondChordRest);

            firstChordRest->score()->undoAddElement(slur);
            SlurSegment* ss = new SlurSegment(firstChordRest->score()->dummy()->system());
            ss->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            if (firstChordRest == secondChordRest) {
                ss->setSlurOffset(Grip::END, PointF(3.0 * firstChordRest->spatium(), 0.0));
            }
            slur->add(ss);

            score()->endCmd();
            doBraille(true);
            return true;
        }
        return false;
    } else {
        return false;
    }
}

bool NotationBraille::addLongSlur()
{
    if (brailleInput()->longSlurStartNote() != NULL
        && currentEngravingItem() != NULL && currentEngravingItem()->isNote()) {
        Note* note1 = brailleInput()->longSlurStartNote();
        Note* note2 = toNote(currentEngravingItem());

        if (note1->parent()->isChordRest() && note2->parent()->isChordRest()) {
            ChordRest* firstChordRest = toChordRest(note1->parent());
            ChordRest* secondChordRest = toChordRest(note2->parent());

            score()->startCmd();

            Slur* slur = Factory::createSlur(firstChordRest->measure()->system());
            slur->setScore(firstChordRest->score());
            slur->setTick(firstChordRest->tick());
            slur->setTick2(secondChordRest->tick());
            slur->setTrack(firstChordRest->track());
            if (secondChordRest->staff()->part() == firstChordRest->staff()->part()
                && !secondChordRest->staff()->isLinked(firstChordRest->staff())) {
                slur->setTrack2(secondChordRest->track());
            } else {
                slur->setTrack2(firstChordRest->track());
            }
            slur->setStartElement(firstChordRest);
            slur->setEndElement(secondChordRest);

            firstChordRest->score()->undoAddElement(slur);
            SlurSegment* ss = new SlurSegment(firstChordRest->score()->dummy()->system());
            ss->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            if (firstChordRest == secondChordRest) {
                ss->setSlurOffset(Grip::END, PointF(3.0 * firstChordRest->spatium(), 0.0));
            }
            slur->add(ss);

            score()->endCmd();
            doBraille(true);
            return true;
        }
        return false;
    } else {
        return false;
    }
}

bool NotationBraille::setVoice(bool new_voice)
{
    if (currentEngravingItem() == NULL || currentMeasure() == NULL) {
        return false;
    }

    staff_idx_t staff = currentEngravingItem()->staffIdx();

    if (new_voice) {
        int voices = 0;
        for (size_t i = 1; i < VOICES; ++i) {
            if (current_measure->hasVoice(staff * VOICES + i)) {
                voices++;
            }
        }
        if (voices >= 3) {
            return false;
        }

        score()->inputState().moveInputPos(currentMeasure()->segments().firstCRSegment());
        interaction()->noteInput()->setCurrentVoice(voices + 1);
        return true;
    } else {
        Segment* segment = score()->inputState().segment();
        Measure* measure = segment->measure();
        if (!measure->hasVoice(staff + 1) && segment->tick() == measure->tick()) {
            interaction()->noteInput()->setCurrentVoice(0);
        }
        return false;
    }
}

void NotationBraille::setMode(const BrailleMode mode)
{
    switch (mode) {
    case BrailleMode::Undefined:
    case BrailleMode::Navigation:
        setCursorColor("black");
        break;
    case BrailleMode::BrailleInput:
        setCursorColor("green");
        break;
    }

    m_mode.set((int)mode);
}

void NotationBraille::toggleMode()
{
    std::string stateTitle;

    switch ((BrailleMode)mode().val) {
    case BrailleMode::Undefined:
    case BrailleMode::Navigation:
        setMode(BrailleMode::BrailleInput);
        interaction()->noteInput()->startNoteInput();
        stateTitle = trc("notation", "Note input mode");
        break;
    case BrailleMode::BrailleInput:
        setMode(BrailleMode::Navigation);
        interaction()->noteInput()->endNoteInput();
        stateTitle = trc("notation", "Normal mode");
        break;
    }

    auto notationAccessibility = notation()->accessibility();
    if (!notationAccessibility) {
        return;
    }
    notationAccessibility->setTriggeredCommand(stateTitle);
}

bool NotationBraille::isNavigationMode()
{
    return mode().val == (int)BrailleMode::Navigation;
}

bool NotationBraille::isBrailleInputMode()
{
    return mode().val == (int)BrailleMode::BrailleInput;
}

void NotationBraille::setCursorColor(const QString color)
{
    m_cursorColor.set(color.toStdString());
}

path_t NotationBraille::tablesDefaultDirPath() const
{
    return globalConfiguration()->appDataPath() + "tables";
}

void NotationBraille::setCurrentEngravingItem(EngravingItem* e, bool select)
{
    if (!e) {
        return;
    }

    bool play = current_engraving_item != e;
    current_engraving_item = e;
    if (isNavigationMode()) {
        if (select) {
            if (play) {
                playbackController()->playElements({ e });
            }
            interaction()->select({ e });
        }
    }
}

IntervalDirection NotationBraille::currentIntervalDirection()
{
    switch (m_intervalDirection.val) {
    case BrailleIntervalDirection::Up:
        return IntervalDirection::Up;
    case BrailleIntervalDirection::Down:
        return IntervalDirection::Down;
    case BrailleIntervalDirection::Auto:
        break;
    }
    if (EngravingItem* element = currentEngravingItem()) {
        if (Staff* staff = element->staff()) {
            ClefType clef = staff->clef(element->tick());
            if (clef >= ClefType::G && clef <= ClefType::C3) {
                return IntervalDirection::Down;
            }
            return IntervalDirection::Up;
        }
    }
    return IntervalDirection::Down;
}

EngravingItem* NotationBraille::currentEngravingItem()
{
    return current_engraving_item;
}

Measure* NotationBraille::currentMeasure()
{
    return current_measure;
}
}
