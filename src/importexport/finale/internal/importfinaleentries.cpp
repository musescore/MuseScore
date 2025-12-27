/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "internal/importfinaleparser.h"
#include "internal/importfinalelogger.h"
#include "internal/finaletypesconv.h"
#include "internal/text/finaletextconv.h"

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "types/string.h"

#include "engraving/dom/accidental.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/beambase.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hook.h"
#include "engraving/dom/laissezvib.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/navigate.h"
#include "engraving/dom/note.h"
#include "engraving/dom/parenthesis.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/symbol.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/utils.h"

// #include "engraving/rendering/score/restlayout.h"
#include "engraving/rendering/score/beamtremololayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {
void FinaleParser::mapLayers()
{
    // This function maps layers to voices based on the layer stem directions and the hardwired MuseScore directions.
    // MuseScore hardwire voices 0,2 up and voices 1,3 down.

    m_layer2Voice.clear();
    std::unordered_map<voice_idx_t, LayerIndex> reverseMap;

    auto layerAttrs = m_doc->getOthers()->getArray<others::LayerAttributes>(m_currentMusxPartId);

    const auto mapLayer = [&](const MusxInstance<others::LayerAttributes>& layerAttr) {
        std::array<voice_idx_t, 4> tryOrder;
        if (!layerAttr->freezeLayer) {
            tryOrder = { 0, 1, 2, 3 }; // default
        } else if (layerAttr->freezeStemsUp) {
            tryOrder = { 0, 2, 1, 3 }; // prefer upstem voices
        } else {
            tryOrder = { 1, 3, 0, 2 }; // prefer downstem voices
        }
        const LayerIndex layerIndex = layerAttr->getCmper();
        for (voice_idx_t idx : tryOrder) {
            auto [it, emplaced] = reverseMap.emplace(idx, layerIndex);
            if (emplaced) {
                m_layer2Voice.emplace(layerIndex, idx);
                return;
            }
        }
        logger()->logWarning(String(u"Unable to map Finale layer %1 to a MuseScore voice due to incompatible layer attributes").arg(
                                 int(layerIndex) + 1));
    };
    for (const auto& layerAttr : layerAttrs) {
        if (layerAttr->freezeLayer) {
            mapLayer(layerAttr);
        }
    }
    for (const auto& layerAttr : layerAttrs) {
        if (!layerAttr->freezeLayer) {
            mapLayer(layerAttr);
        }
    }
}

std::unordered_map<int, voice_idx_t> FinaleParser::mapFinaleVoices(const std::map<LayerIndex, int>& finaleVoiceMap,
                                                                   StaffCmper curStaff, MeasCmper curMeas) const
{
    using FinaleVoiceID = int;
    std::unordered_map<FinaleVoiceID, voice_idx_t> result;
    std::unordered_map<voice_idx_t, FinaleVoiceID> reverseMap;
    for (const auto& [layerIndex, voice2Count] : finaleVoiceMap) {
        const auto& it = m_layer2Voice.find(layerIndex);
        if (it != m_layer2Voice.end()) {
            auto [revIt, emplaced] = reverseMap.emplace(it->second, createFinaleVoiceId(layerIndex, false));
            if (emplaced) {
                result.emplace(revIt->second, revIt->first);
                continue;
            }
        }
        logger()->logWarning(String(u"Layer %1 was not mapped to a voice").arg(int(layerIndex) + 1), m_doc, curStaff, curMeas);
    }
    for (const auto& [layerIndex, voice2Count] : finaleVoiceMap) {
        if (voice2Count != 0) {
            bool foundVoice = false;
            for (voice_idx_t v : { 0, 1, 2, 3 }) {
                auto [revIt, emplaced] = reverseMap.emplace(v, createFinaleVoiceId(layerIndex, true));
                if (emplaced) {
                    result.emplace(revIt->second, revIt->first);
                    foundVoice = true;
                    break;
                }
            }
            if (!foundVoice) {
                logger()->logWarning(String(u"Voice 2 exceeded available MuseScore voices for layer %1.").arg(
                                         int(layerIndex) + 1), m_doc, curStaff, curMeas);
                break;
            }
        }
    }
    return result;
}

MusxInstance<others::LayerAttributes> FinaleParser::layerAttributes(const Fraction& tick, track_idx_t track)
{
    if (m_track2Layer.empty() || m_track2Layer.at(track).empty()) {
        return nullptr;
    }
    const auto it = muse::findLessOrEqual(m_track2Layer.at(track), tick.ticks());
    if (it != m_track2Layer.at(track).cend()) {
        return m_doc->getOthers()->get<others::LayerAttributes>(m_currentMusxPartId, it->second);
    }
    return nullptr;
}

DirectionV FinaleParser::getDirectionVForLayer(const ChordRest* cr)
{
    const auto& layerInfo = layerAttributes(cr->segment()->tick(), cr->track());
    if (!layerInfo || !layerInfo->freezeLayer) {
        return DirectionV::AUTO;
    }

    DirectionV d = layerInfo->freezeStemsUp ? DirectionV::UP : DirectionV::DOWN;
    if (!layerInfo->onlyIfOtherLayersHaveNotes) {
        return d;
    }

    track_idx_t strack = trackZeroVoice(cr->track());
    track_idx_t etrack = strack + VOICES;
    for (Segment* s = cr->measure()->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (track_idx_t track = strack; track < etrack; ++track) {
            if (track == cr->track()) {
                continue;
            }
            if (ChordRest* chordRest = toChordRest(s->element(track))) {
                if (chordRest->isFullMeasureRest() && !chordRest->visible()) {
                    continue;
                }
                if (!layerInfo->ignoreHiddenNotesOnly) {
                    return d;
                }
                if (chordRest->isChord()) {
                    Chord* c = toChord(chordRest);
                    for (engraving::Note* n : c->notes()) {
                        if (n->visible()) {
                            return d;
                        }
                    }
                } else if (chordRest->isRest() && chordRest->visible() && !toRest(chordRest)->isGap()) {
                    return d;
                }
            }
        }
    }
    return DirectionV::AUTO;
}

engraving::Note* FinaleParser::noteFromEntryInfoAndNumber(const EntryInfoPtr& entryInfoPtr, NoteNumber nn)
{
    if (!entryInfoPtr) {
        return nullptr;
    }
    return muse::value(m_entryNoteNumber2Note, std::make_pair(entryInfoPtr->getEntry()->getEntryNumber(), nn), nullptr);
}

engraving::Note* FinaleParser::noteFromNoteInfoPtr(const NoteInfoPtr& noteInfoPtr)
{
    if (!noteInfoPtr) {
        return nullptr;
    }
    return noteFromEntryInfoAndNumber(noteInfoPtr.getEntryInfo(), noteInfoPtr->getNoteId());
}

ChordRest* FinaleParser::chordRestFromEntryInfoPtr(const EntryInfoPtr& entryInfoPtr)
{
    return muse::value(m_entryNumber2CR, entryInfoPtr->getEntry()->getEntryNumber());
}

static size_t indexOfParentTuplet(std::vector<ReadableTuplet> tupletMap, size_t index)
{
    for (size_t i = index; i > 0; --i) {
        if (tupletMap[i].layer + 1 == tupletMap[index].layer) {
            if (tupletMap[i].startTick <= tupletMap[index].startTick && tupletMap[i].endTick >= tupletMap[index].endTick) {
                return i;
            }
        }
    }
    return 0;
}

static Tuplet* bottomTupletFromTick(std::vector<ReadableTuplet> tupletMap, Fraction pos)
{
    // return first tuplet the pos is contained within,
    // starting from the end (bottom layers) and working our way up
    for (size_t i = tupletMap.size() - 1; i > 0; --i) {
        if (pos >= tupletMap[i].startTick && pos < tupletMap[i].endTick) {
            return tupletMap[i].scoreTuplet;
        }
    }
    return nullptr;
}

static Fraction findParentTickForGraceNote(EntryInfoPtr entryInfo, bool& insertAfter, FinaleLoggerPtr& logger)
{
    if (const EntryInfoPtr mainNote = entryInfo.findMainEntryForGraceNote(/*ignoreRests*/ true)) {
        insertAfter = false;
        return musxFractionToFraction(mainNote.calcGlobalElapsedDuration()).reduced();
    }
    if (const EntryInfoPtr prevNonGrace = entryInfo.getPreviousSameVNoGrace()) {
        if (!prevNonGrace.calcDisplaysAsRest()) {
            insertAfter = true;
            return musxFractionToFraction(prevNonGrace.calcGlobalElapsedDuration()).reduced();
        }
    }
    // MuseScore requires grace notes be attached to a chord. The above code
    // tries to find a chord adjacent to the grace notes, but will not yield
    // a result in certain edge cases. One day MuseScore will allow the same
    // flexibility for grace notes as Finale, but until then we must do this
    logger->logWarning(String(u"Failed to attach grace notes to a chord."));
    return Fraction(-1, 1);
}

static void setNoteHeadSymbol(engraving::Note* note, SymId noteHeadSym)
{
    // MuseScore doesn't allow custom symbols directly, so instead we try to find
    // head group and head type settings that will produce the desired symbol on layout.
    if (noteHeadSym == SymId::space || noteHeadSym == SymId::noSym) {
        // Don't hide head for noSym, as that is the fallback value
        if (noteHeadSym == SymId::space) {
            note->setVisible(false);
        }
        return;
    }

    for (int direction = 1; direction >= 0; --direction) {
        for (NoteHeadGroup group = NoteHeadGroup::HEAD_NORMAL; group < NoteHeadGroup::HEAD_GROUPS; group = NoteHeadGroup(int(group) + 1)) {
            for (NoteHeadType type = NoteHeadType::HEAD_AUTO; type < NoteHeadType::HEAD_TYPES; type = NoteHeadType(int(type) + 1)) {
                if (engraving::Note::noteHead(direction, group, type) == noteHeadSym) {
                    note->setHeadGroup(group);
                    note->setHeadType(type);
                    return;
                }
            }
        }
    }
}

static std::pair<bool, bool> getAccidentalProperties(std::string symbolName, SymId actualSym)
{
    bool hasParentheses = false;
    bool isSmall = false;
    switch (actualSym) {
    case SymId::accidentalFlat:
        hasParentheses = symbolName == "accidentalFlatParens" || symbolName == "accidentalFlatParenthesesSmall";
        isSmall = symbolName == "accidentalFlatSmall" || symbolName == "accidentalFlatParenthesesSmall";
        break;
    case SymId::accidentalDoubleFlat:
        hasParentheses = symbolName == "accidentalDoubleFlatParens" || symbolName == "accidentalDoubleFlatParenthesesSmall";
        isSmall = symbolName == "accidentalDoubleFlatSmall" || symbolName == "accidentalDoubleFlatParenthesesSmall";
        break;
    case SymId::accidentalNatural:
        hasParentheses = symbolName == "accidentalNaturalParens" || symbolName == "accidentalNaturalParenthesesSmall";
        isSmall = symbolName == "accidentalNaturalSmall" || symbolName == "accidentalNaturalParenthesesSmall";
        break;
    case SymId::accidentalSharp:
        hasParentheses = symbolName == "accidentalSharpParens" || symbolName == "accidentalSharpParenthesesSmall";
        isSmall = symbolName == "accidentalSharpSmall" || symbolName == "accidentalSharpParenthesesSmall";
        break;
    case SymId::accidentalDoubleSharp:
        hasParentheses = symbolName == "accidentalDoubleSharpParens" || symbolName == "accidentalDoubleSharpParenthesesSmall";
        isSmall = symbolName == "accidentalDoubleSharpSmall" || symbolName == "accidentalDoubleSharpParenthesesSmall";
        break;
    default:
        break;
    }
    return std::make_pair(hasParentheses, isSmall);
}

bool FinaleParser::processEntryInfo(EntryInfoPtr::InterpretedIterator result, track_idx_t curTrackIdx, Measure* measure, bool graceNotes,
                                    std::vector<engraving::Note*>& notesWithUnmanagedTies,
                                    std::vector<ReadableTuplet>& tupletMap, bool hasVoice1Voice2)
{
    // Retrieve fields from WorkaroundAwareResult
    EntryInfoPtr entryInfo = result.getEntryInfo();
    bool effectiveHidden = result.getEffectiveHidden();

    // Retrieve entry from entryInfo
    MusxInstance<Entry> currentEntry = entryInfo->getEntry();
    IF_ASSERT_FAILED(currentEntry) {
        logger()->logWarning(String(u"Failed to get entry"));
        return false;
    }
    EntryNumber currentEntryNumber = currentEntry->getEntryNumber();

    const bool isGrace = currentEntry->graceNote;
    if (isGrace != graceNotes) {
        return true;
    }
    bool graceAfterType = false;

    Fraction entryStartTick = Fraction(-1, 1);
    // todo: save the fraction to avoid calling this function for every grace note
    // And the grace note code is sparse in safety checks by comparison with the rest of the code.
    if (isGrace) {
        if (entryInfo.calcDisplaysAsRest()) {
            logger()->logWarning(String(u"Grace rests are not supported"));
            return false;
        }
        entryStartTick = findParentTickForGraceNote(result.getEntryInfo(), graceAfterType, logger()); // use iterated entry to get start tick for source entries.
    } else {
        entryStartTick = musxFractionToFraction(result.getEffectiveElapsedDuration(/*global*/ true));
    }
    if (entryStartTick.negative()) {
        // Return true for non-anchorable grace notes, else false
        return isGrace;
    }
    Measure* originalMeasure = measure;
    Fraction originalTick = entryStartTick;
    while (entryStartTick >= measure->ticks()) {
        // If entries spill past the end of the measure, put them in the next measure.
        // A common situation for this is beams over barlines created by the Beam Over Barline plugin.
        // There are other situations (tuplets over barlines come to mind) where users have made adhoc
        // use of extra entries in a measure. Since MuseScore hates these, we put them in the next measure,
        // then possibly overwrite them when we import the entries for the next measure.
        /// @todo We may need to adjust `curTrackIdx` to match the layers/voices in the next measure, but let's
        /// hope that is such a rare edge case that we don't.
        entryStartTick -= measure->ticks();
        measure = measure->nextMeasure();
        if (!measure) {
            logger()->logWarning(String(u"Encountered entry number %1 beyond the end of the document.").arg(currentEntry->getEntryNumber()));
            measure = originalMeasure;
            entryStartTick = originalTick;
            break;
        }
    }
    if (Segment* existingSeg = measure->findSegmentR(SegmentType::ChordRest, entryStartTick)) {
        if (toChordRest(existingSeg->element(curTrackIdx))) {
            if (entryInfo.calcCanBeBeamed() && currentEntry->isHidden) {
                // This entry is probably a placeholder for a beam over barline that was created
                // in the pass for the previous measure, so skip it.
                return true;
            }
        }
    }
    Segment* segment = measure->getSegmentR(SegmentType::ChordRest, entryStartTick);

    // durationType
    TDuration d = musxDurationInfoToDuration(currentEntry->calcDurationInfo());
    if (!d.isValid()) {
        logger()->logWarning(String(u"Given ChordRest duration not supported in MuseScore"));
        return false;
    }

    ChordRest* cr = nullptr;
    int crossStaffMove = 0;
    staff_idx_t staffIdx = track2staff(curTrackIdx);
    const bool unbeamed = entryInfo.calcUnbeamed();

    // because we need the real staff to calculate when to show accidentals,
    // we have to calculate cross-staffing before pitches
    /// @todo what if cross-staff chord from other staves interfere with this one
    for (size_t i = 0; i < currentEntry->notes.size(); ++i) {
        NoteInfoPtr noteInfoPtr = NoteInfoPtr(entryInfo, i);
        if (noteInfoPtr->crossStaff) {
            StaffCmper nextMusxStaff = noteInfoPtr.calcStaff();
            staff_idx_t crossStaffIdx = muse::value(m_inst2Staff, nextMusxStaff, muse::nidx);
            IF_ASSERT_FAILED(crossStaffIdx != muse::nidx) {
                logger()->logWarning(String(u"Collect cross staffing: Musx inst value not found for staff cmper %1"), m_doc, nextMusxStaff);
                continue;
            }
            int newStaffMove = int(crossStaffIdx) - int(staffIdx);
            // MuseScore doesn't support individual cross-staff notes, either move the whole chord or nothing.
            // When moving, prioritise outermost staves
            if (std::abs(newStaffMove) > std::abs(crossStaffMove)) {
                crossStaffMove = newStaffMove;
            }
        }
    }
    // check if the calculate cross-staff position is valid
    staff_idx_t idx = static_cast<staff_idx_t>(int(staffIdx) + crossStaffMove);
    Staff* baseStaff = m_score->staff(staffIdx);
    Staff* targetStaff = m_score->staff(idx);
    if (!(targetStaff && targetStaff->visible() && targetStaff->isLinked() == baseStaff->isLinked()
          && staff2track(idx) >= baseStaff->part()->startTrack() && staff2track(idx) < baseStaff->part()->endTrack()
          && targetStaff->staffType(segment->tick())->group() == baseStaff->staffType(segment->tick())->group())) {
        crossStaffMove = 0;
        targetStaff = baseStaff;
        idx = staffIdx;
    }

    const auto& layerInfo = m_doc->getOthers()->get<others::LayerAttributes>(m_currentMusxPartId, entryInfo.getLayerIndex());
    const bool neverPlayback = layerInfo && !layerInfo->playback;

    if (!entryInfo.calcDisplaysAsRest()) {
        engraving::Chord* chord = Factory::createChord(segment);

        for (size_t i = 0; i < currentEntry->notes.size(); ++i) {
            NoteInfoPtr noteInfoPtr = NoteInfoPtr(entryInfo, i);

            engraving::Note* note = Factory::createNote(chord);
            note->setParent(chord);
            note->setTrack(curTrackIdx);
            note->setVisible(!effectiveHidden);
            note->setPlay(!currentEntry->noPlayback && !neverPlayback); /// @todo account for spanners
            note->setAutoplace(!noteInfoPtr->noSpacing);

            if (targetStaff->isDrumStaff(segment->tick())) {
                NoteVal nval;
                const Drumset* ds = targetStaff->part()->instrument()->drumset();
                MusxInstance<others::PercussionNoteInfo> percNoteInfo = noteInfoPtr.calcPercussionNoteInfo();
                if (!percNoteInfo) {
                    delete note;
                    continue;
                }
                nval.pitch = midiNoteFromPercussionNoteType(targetStaff->part()->instrument()->id(), percNoteInfo->getBaseNoteTypeId());
                // Only add notes we find in the drumset, but don't create empty grace chords (grace rests aren't possible)
                if (!ds->isValid(nval.pitch) && (!isGrace || i > 0)) {
                    delete note;
                    continue;
                }
                nval.headGroup = ds->noteHead(nval.pitch);
                note->setNval(nval);
            } else {
                // calculate pitch & accidentals
                NoteVal nval = notePropertiesToNoteVal(noteInfoPtr.calcNotePropertiesConcert(), baseStaff->concertKey(segment->tick()));
                NoteVal nvalTransposed = notePropertiesToNoteVal(noteInfoPtr.calcNoteProperties(), baseStaff->key(segment->tick()));
                nval.tpc2 = nvalTransposed.tpc2;
                note->setNval(nval);

                if (targetStaff->isPitchedStaff(segment->tick())) {
                    // Add accidental if needed
                    /// @todo Do we really need to explicitly add the accidental object if it's not frozen?
                    /// RGP: if it has been manually moved, it looks like it.Otherwise perhaps not.
                    AccidentalVal accVal = tpc2alter(nval.tpc1);
                    bool hasAccidental = noteInfoPtr->freezeAcci && noteInfoPtr->showAcci;
                    if (!hasAccidental) {
                        if (Segment* startSegment = note->firstTiedNote()->chord()->segment()) {
                            int line = noteValToLine(nval, targetStaff, segment->tick());
                            bool error = false;
                            AccidentalVal defaultAccVal = startSegment->measure()->findAccidental(startSegment, idx, line, error);
                            hasAccidental = error || (defaultAccVal != accVal);
                        }
                    }

                    if (hasAccidental) {
                        AccidentalType at = Accidental::value2subtype(accVal);
                        Accidental* a = Factory::createAccidental(note);
                        a->setAccidentalType(at);
                        a->setRole(noteInfoPtr->freezeAcci ? AccidentalRole::USER : AccidentalRole::AUTO);
                        a->setVisible(note->visible() && noteInfoPtr->showAcci);
                        a->setBracket(noteInfoPtr->parenAcci ? AccidentalBracket::PARENTHESIS : AccidentalBracket::NONE);
                        a->setParent(note);
                        if (currentEntry->noteDetail) {
                            // Accidental size and offset
                            /// @todo Finale doesn't offset notes for ledger lines, MuseScore offsets
                            /// rightmost accidentals matching the type of an accidental on a note with ledger lines.
                            /// @todo decide when to disable autoplace
                            if (const MusxInstance<details::AccidentalAlterations>& accidentalInfo
                                    = m_doc->getDetails()->getForNote<details::AccidentalAlterations>(noteInfoPtr)) {
                                if (muse::RealIsEqualOrLess(doubleFromPercent(accidentalInfo->percent),
                                                            m_score->style().styleD(Sid::smallNoteMag))) {
                                    a->setSmall(true);
                                }
                                /// @todo this calculation needs to take into account the default accidental separation amounts in accidentalOptions. The options
                                /// should allow us to calculate the default position of the accidental relative to the note. (But it may not be easy.)
                                if (importCustomPositions()) {
                                    Evpu accVert = accidentalInfo->allowVertPos ? -accidentalInfo->vOffset : 0;
                                    a->setOffset(evpuToPointF(accidentalInfo->hOffset, accVert) * a->defaultSpatium());
                                }

                                if (accidentalInfo->altChar) {
                                    /// @todo verify if we can always use custom font (like for articulations) or not
                                    auto [canParenthesise, isSmall]
                                        = getAccidentalProperties(FinaleTextConv::charNameFinale(accidentalInfo->altChar,
                                                                                                 accidentalInfo->customFont), a->symId());
                                    if (!canParenthesise && !isSmall) {
                                        SymId customSym = FinaleTextConv::symIdFromFinaleChar(accidentalInfo->altChar,
                                                                                              accidentalInfo->customFont);
                                        if (customSym != SymId::noSym) {
                                            Symbol* sym = new Symbol(note);
                                            sym->setTrack(curTrackIdx);
                                            if (fontIsEngravingFont(accidentalInfo->customFont)) {
                                                sym->setSym(customSym,
                                                            note->score()->engravingFonts()->fontByName(
                                                                accidentalInfo->customFont->getName()));
                                            } else {
                                                sym->setSym(customSym);
                                            }
                                            sym->setOffset(a->offset()); /// @todo exact positioning
                                            a->setVisible(false);
                                            note->add(sym);
                                        }
                                    } else {
                                        if (canParenthesise) {
                                            setAndStyleProperty(a, Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::PARENTHESIS));
                                        }
                                        if (isSmall) {
                                            a->setSmall(true);
                                        }
                                    }
                                }
                            }
                        }
                        note->add(a);
                    }
                } else if (targetStaff->isTabStaff(segment->tick())) {
                    if (const MusxInstance<details::TablatureNoteMods> tabInfo
                            = m_doc->getDetails()->getForNote<details::TablatureNoteMods>(noteInfoPtr)) {
                        note->setString(tabInfo->stringNumber - 1);
                        const StringData* stringData = targetStaff->part()->stringData(segment->tick(), idx);
                        note->setFret(stringData->fret(note->pitch(), note->string(), targetStaff)); // we may not need to set this
                    }
                }
            }
            if (currentEntry->noteDetail) {
                if (const MusxInstance<details::NoteAlterations> noteInfo
                        = m_doc->getDetails()->getForNote<details::NoteAlterations>(noteInfoPtr)) {
                    if (noteInfo->percent
                        && muse::RealIsEqualOrLess(doubleFromPercent(noteInfo->percent), m_score->style().styleD(Sid::smallNoteMag))) {
                        note->setSmall(true);
                    }
                    if (importCustomPositions()) {
                        note->setOffset(evpuToPointF(noteInfo->nxdisp,
                                                     noteInfo->allowVertPos ? -noteInfo->nydisp : 0) * note->defaultSpatium());
                    }
                    if (targetStaff->isTabStaff(segment->tick())
                        && (noteInfo->altNhead == U'X' || noteInfo->altNhead == U'x')) {
                        // Shortcut for dead notes
                        note->setProperty(Pid::HEAD_GROUP, NoteHeadGroup::HEAD_CROSS);
                    } else {
                        SymId customSym = FinaleTextConv::symIdFromFinaleChar(noteInfo->altNhead, noteInfo->customFont);
                        if (customSym == SymId::noSym) {
                            customSym = unparenthesisedNoteHead(FinaleTextConv::charNameFinale(noteInfo->altNhead, noteInfo->customFont));
                            if (customSym != SymId::noSym) {
                                for (int j = 0; j < 2; ++j) {
                                    Parenthesis* p = Factory::createParenthesis(note);
                                    p->setParent(note);
                                    p->setTrack(curTrackIdx);
                                    p->setVisible(note->visible());
                                    p->setDirection(j == 0 ? DirectionH::LEFT : DirectionH::RIGHT);
                                    note->add(p);
                                }
                            }
                        }
                        setNoteHeadSymbol(note, customSym);
                    }
                }
            }

            chord->add(note);

            // set up ties
            if (noteInfoPtr->tieStart) {
                // We can't tell for sure at this point whether a tie will have an end note,
                // so we decide between real tie and l.v. tie later on.
                notesWithUnmanagedTies.emplace_back(note);
            }
            /// @todo This code won't work if the start note is in a currently unmapped voice. But because of
            /// the fact we explicitly create accidentals, we need the ties to be correct during processEntryInfo. (Do we?)
            // Don't use noteInfoPtr->tieEnd as it's unreliable
            engraving::Note* prevTied = noteFromNoteInfoPtr(noteInfoPtr.calcTieFrom());
            if (prevTied) {
                Tie* tie = Factory::createTie(m_score->dummy());
                tie->setStartNote(prevTied);
                tie->setTick(prevTied->tick());
                tie->setTrack(prevTied->track());
                // Finale offers independent visibility controls for tie segments, MuseScore currently does not.
                // We set the visibility based on
                tie->setVisible(prevTied->visible());
                tie->setParent(prevTied);
                prevTied->setTieFor(tie);
                tie->setEndNote(note);
                tie->setTick2(note->tick());
                tie->setTrack2(note->track());
                note->setTieBack(tie);
                muse::remove(notesWithUnmanagedTies, prevTied);
            } else {
                logger()->logInfo(String(u"Tie does not have starting note. Possibly a partial tie, currently unsupported."));
            }
            m_entryNoteNumber2Note.emplace(std::make_pair(currentEntryNumber, noteInfoPtr->getNoteId()), note);
        }

        if (chord->notes().empty()) {
            delete chord;
            cr = toChordRest(Factory::createRest(segment, d));
            toRest(cr)->setVisible(false);
        } else {
            // Stem and stem direction
            if (importCustomPositions()) {
                const auto [freezeStem, upStem] = entryInfo.calcEntryStemSettings();
                // LayerAttributes are read later on, once all voices have been added to the score.
                // Additionally, beams have their own vertical direction, which is set in processBeams.
                if (freezeStem) {
                    chord->setStemDirection(upStem ? DirectionV::UP : DirectionV::DOWN);
                    m_fixedChords.insert(chord);
                } else if (hasVoice1Voice2) {
                    // Freeze all stems in a v1v2 context, because otherwise MuseScore treats it
                    // like layers, flipping all stems in track 0 up, track 1 down, etc.
                    chord->setStemDirection(entryInfo.calcUpStem() ? DirectionV::UP : DirectionV::DOWN);
                }
            }
            if (chord->shouldHaveStem() || d.hasStem()) {
                Stem* stem = Factory::createStem(chord);
                stem->setVisible(!effectiveHidden);
                chord->add(stem);
            }
            if (unbeamed && d.hooks() > 0) {
                chord->setBeamMode(BeamMode::NONE);
                Hook* hook = new Hook(chord);
                hook->setVisible(!effectiveHidden);
                chord->setHook(hook);
                chord->add(hook);
            }
            cr = toChordRest(chord);
        }
    } else {
        const MusxInstance<others::Staff> musxStaff = entryInfo.createCurrentStaff();
        if (entryInfo.calcIsFullMeasureRest()) {
            d = TDuration(DurationType::V_MEASURE);
        }
        Rest* rest = Factory::createRest(segment, d);
        // Fixed-positioning for rests is calculated in a 2nd pass after all voices in all layers have been created.
        // This allows MuseScore code to calculate correctly the voice offset for the rest.
        if (importCustomPositions() && !currentEntry->floatRest && !currentEntry->notes.empty()) {
            NoteInfoPtr noteInfoPtr = NoteInfoPtr(entryInfo, 0);
            StaffCmper targetMusxStaffId = muse::value(m_staff2Inst, idx, 0);
            IF_ASSERT_FAILED(targetMusxStaffId) {
                logger()->logWarning(String(u"Entry %1 (a rest) was not mapped to a known musx staff.").arg(
                                         currentEntry->getEntryNumber()), m_doc, entryInfo.getStaff(), entryInfo.getMeasure());
                return false;
            }
            if (noteInfoPtr->getNoteId() == musx::dom::Note::RESTID) {
                /// @todo correctly calculate default rest position in multi-voice situation. rest->setAutoplace is problematic because it also affects spacing
                /// (and does not cover all vertical placement situations either).
                MusxInstance<others::StaffComposite> currMusxStaff = noteInfoPtr.getEntryInfo().createCurrentStaff(targetMusxStaffId);
                IF_ASSERT_FAILED(currMusxStaff) {
                    logger()->logWarning(String(u"Target staff %1 not found.").arg(targetMusxStaffId), m_doc,
                                         entryInfo.getStaff(), entryInfo.getMeasure());
                }
                auto [pitchClass, octave, alteration, staffPosition] = noteInfoPtr.calcNotePropertiesInView();
                //const int defaultLine = (baseStaff->lines(entryStartTick) + 1) / 2; // Spatiums relative to top staff
                const double lineSpacing = baseStaff->lineDistance(entryStartTick);
                // const int defaultMusxLine = 2 * defaultLine - currMusxStaff->calcTopLinePosition();
                staffPosition += currMusxStaff->calcTopLinePosition();

                // convert defaultLine to staff position offset for Finale rest. This value is measured in 0.5sp steps.
                if (rest->isWholeRest()) {
                    staffPosition += 2; // account for a whole rest's staff line discrepancy between Finale and MuseScore
                }
                rest->setAlignWithOtherRests(false); // override as much automatic positioning as possible
                rest->setMinDistance(Spatium(-999.0));
                rest->ryoffset() = double(-staffPosition /*- defaultMusxLine*/) * baseStaff->spatium(entryStartTick) * lineSpacing / 2.0;
                /// @todo Account for additional default positioning around collision avoidance (when the rest is on the "wrong" side for the voice.)
                /// Unfortunately, we can't set `autoplace` to false because that also suppresses horizontal spacing.
                /// The test file `beamsAndRest.musx` includes an example of this issue in the first 32nd rest in the top staff.
                /// It should be at the same vertical position as the second 32nd rest in the same staff.
            } else {
                logger()->logWarning(String(u"Rest found with unexpected note ID %1").arg(
                                         noteInfoPtr->getNoteId()), m_doc, entryInfo.getStaff(), entryInfo.getMeasure());
                return false;
            }
        }
        cr = toChordRest(rest);
        cr->setVisible(!musxStaff->hideRests && !effectiveHidden);
    }

    int entrySize = entryInfo.calcEntrySize();
    if (entrySize <= MAX_CUE_PERCENTAGE) {
        double crMag = doubleFromPercent(entrySize);
        if (muse::RealIsEqualOrLess(crMag, m_score->style().styleD(Sid::smallNoteMag))) { // is just less enough here?
            if (m_smallNoteMagFound) {
                logger()->logWarning(String(u"Inconsistent cue note sizes found. Using the smallest encountered."),
                                     m_doc, entryInfo.getStaff(), entryInfo.getMeasure());
            }
            collectGlobalProperty(Sid::smallNoteMag, crMag);
        }
        m_smallNoteMagFound = true;
        cr->setSmall(true);
    }

    cr->setDurationType(d);
    cr->setStaffMove(crossStaffMove);
    cr->setTrack(curTrackIdx);
    if (cr->durationType().isMeasure()) {
        cr->setTicks(measure->stretchedLen(baseStaff)); // baseStaff because that's the staff the cr 'belongs to'
    } else {
        cr->setTicks(cr->actualDurationType().fraction());
    }
    if (isGrace) {
        engraving::Chord* gc = toChord(cr);
        /// @todo Account for stem slash plugin instead of just document options
        gc->setNoteType((!graceAfterType /* && gc->beams() > 0 */ && unbeamed
                         && (currentEntry->slashGrace || musxOptions().graceOptions->slashFlaggedGraceNotes))
                        ? engraving::NoteType::ACCIACCATURA : durationTypeToNoteType(d.type(), graceAfterType));
        engraving::Chord* graceParentChord = toChord(segment->element(curTrackIdx));
        gc->setGraceIndex(static_cast<int>(graceAfterType ? 0 : graceParentChord->graceNotesBefore().size()));
        graceParentChord->add(gc);
    } else {
        segment->add(cr);
        if (Tuplet* parentTuplet = bottomTupletFromTick(tupletMap, entryStartTick)) {
            parentTuplet->add(cr);
        }
        logger()->logInfo(String(u"Adding entry of duration %2 at tick %1").arg(entryStartTick.toString(),
                                                                                cr->durationTypeTicks().toString()));
    }

    // Dot offset
    /// Only generate dots if they have modified properties, otherwise created automatically on layout
    if (importCustomPositions() && currentEntry->dotTieAlt) {
        MusxInstanceList<details::DotAlterations> dotAlterations = m_doc->getDetails()->getArray<details::DotAlterations>(
            m_currentMusxPartId, currentEntryNumber);
        for (const MusxInstance<details::DotAlterations>& da : dotAlterations) {
            engraving::Note* n = cr->isChord() ? noteFromEntryInfoAndNumber(entryInfo, da->getNoteId()) : nullptr;
            Rest* r = cr->isRest() ? toRest(cr) : nullptr;
            EvpuFloat museInterdot = EvpuFloat(da->interdotSpacing) - evpuAugmentationDotWidth(); /// @todo not sure about this, but it gets the closest result to Finale in my testing
            if (n) {
                for (int i = 0; i < cr->dots(); ++i) {
                    NoteDot* dot = Factory::createNoteDot(n);
                    dot->setParent(n);
                    dot->setVisible(n->visible());
                    dot->setTrack(cr->track());
                    dot->setOffset(evpuToPointF(da->hOffset + i * museInterdot, -da->vOffset) * dot->defaultSpatium()); // correctly scaled?
                    n->add(dot);
                }
            } else if (r) {
                for (int i = 0; i < cr->dots(); ++i) {
                    NoteDot* dot = Factory::createNoteDot(r);
                    dot->setParent(r);
                    dot->setVisible(r->visible());
                    dot->setTrack(cr->track());
                    dot->setOffset(evpuToPointF(da->hOffset + i * museInterdot, -da->vOffset) * dot->defaultSpatium()); // correctly scaled?
                    r->add(dot);
                }
                break;
            }
        }
    }
    m_entryNumber2CR.emplace(currentEntryNumber, cr);
    return true;
}

bool FinaleParser::processBeams(EntryInfoPtr entryInfoPtr, track_idx_t curTrackIdx)
{
    /// @todo get beaming rules from time signature and only override properties when necessary
    if (!entryInfoPtr.calcIsBeamStart(EntryInfoPtr::BeamIterationMode::Interpreted)) {
        // This check is necessary because we process beams one measure at a time.
        const bool isBeamContinuation = !entryInfoPtr.getPreviousSameV() && entryInfoPtr.getPreviousInBeamGroupAcrossBars();
        if (!isBeamContinuation) {
            return true;
        }
    }
    const MusxInstance<Entry>& firstEntry = entryInfoPtr->getEntry();
    ChordRest* firstCr = chordRestFromEntryInfoPtr(entryInfoPtr);
    IF_ASSERT_FAILED(firstCr || entryInfoPtr.calcCreatesSingletonBeamLeft()) {
        logger()->logWarning(String(u"Entry %1 was not mapped").arg(firstEntry->getEntryNumber()), m_doc,
                             entryInfoPtr.getStaff(), entryInfoPtr.getMeasure());
        return false;
    }

    auto calcBeamMode = [](unsigned count) -> BeamMode {
        if (count <= 1) {
            return BeamMode::MID;
        } else if (count == 2) {
            return BeamMode::BEGIN16;
        } else {
            return BeamMode::BEGIN32;
        }
    };

    Beam* beam = Factory::createBeam(m_score->dummy()->system());
    beam->setTrack(curTrackIdx);
    if (firstCr) {
        beam->add(firstCr);
        if (!entryInfoPtr.calcBeamContinuesLeftOverBarline()) {
            firstCr->setBeamMode(BeamMode::BEGIN);
        } else {
            const unsigned beamBreaks = entryInfoPtr.calcLowestBeamStart(/*considerBeamOverBarlines*/ true);
            firstCr->setBeamMode(calcBeamMode(beamBreaks));
        }
        if (firstEntry->isNote && firstCr->isChord()) {
            DirectionV stemDir = toChord(firstCr)->stemDirection();
            if (stemDir != DirectionV::AUTO) {
                beam->doSetDirection(stemDir);
            }
        }
    }

    const MeasCmper startMeasureId = entryInfoPtr.getMeasure();

    for (EntryInfoPtr nextInBeam = entryInfoPtr.getNextInBeamGroupAcrossBars(EntryInfoPtr::BeamIterationMode::Interpreted);
         nextInBeam;
         nextInBeam = nextInBeam.getNextInBeamGroupAcrossBars(EntryInfoPtr::BeamIterationMode::Interpreted)) {
        if (nextInBeam.getMeasure() != startMeasureId) {
            break;
        }
        const MusxInstance<Entry>& currentEntry = nextInBeam->getEntry();
        EntryNumber currentEntryNumber = currentEntry->getEntryNumber();
        if (entryInfoPtr->getEntry()->graceNote && !currentEntry->isNote) {
            // Grace rests are unmapped and not supported
            continue;
        }
        ChordRest* currentCr = chordRestFromEntryInfoPtr(nextInBeam);
        if (!currentCr) {
            // this can happen if the entry was the first (invisible) entry of a singleton beam left. Such entries should be skipped.
            logger()->logWarning(String(u"Entry %1 was not mapped").arg(currentEntryNumber), m_doc,
                                 nextInBeam.getStaff(), nextInBeam.getMeasure());
            continue;
        }
        beam->add(currentCr);

        // Secondary beam breaks
        const unsigned secBeamStart = nextInBeam.calcLowestBeamStart(/*considerBeamOverBarlines*/ true);
        currentCr->setBeamMode(calcBeamMode(secBeamStart));

        // Stem direction
        if (currentEntry->isNote && currentCr->isChord()) {
            DirectionV stemDir = toChord(currentCr)->stemDirection();
            if (stemDir != DirectionV::AUTO) {
                beam->doSetDirection(stemDir);
            }
        }
    }
    beam->resetExplicitParent();
    return true;
}

static void processTremolos(const std::vector<ReadableTuplet>& tremoloMap, track_idx_t curTrackIdx, Measure* measure)
{
    /// @todo account for invalid durations
    Fraction timeStretch = measure->score()->staff(track2staff(curTrackIdx))->timeStretch(measure->tick());
    for (const ReadableTuplet& tuplet : tremoloMap) {
        engraving::Chord* c1 = measure->findChord(measure->tick() + tuplet.startTick, curTrackIdx); // timestretch?
        engraving::Chord* c2 = measure->findChord(measure->tick() + ((tuplet.startTick + tuplet.endTick) / 2), curTrackIdx);
        IF_ASSERT_FAILED(c1 && c2 && c1->ticks() == c2->ticks()) {
            continue;
        }

        // the current (too short) duration indicates the
        // tremolo type, we calculate it from number of beams
        int tremoloBeamsNum = int(TremoloType::C8) - 1 + c1->durationType().hooks();
        // rather than not import the tremolo, force it to be a valid type
        tremoloBeamsNum = std::clamp(tremoloBeamsNum, int(TremoloType::C8), int(TremoloType::C64));

        // now we have to set the correct duration for the chords to,
        // fill the space (and account for any tuplets they may be in)
        Fraction d = (c2->tick() - c1->tick()) * timeStretch;
        for (Tuplet* t = c1->tuplet(); t; t = t->tuplet()) {
            d *= t->ratio();
        }
        c1->setDurationType(d.reduced());
        c1->setTicks(c1->actualDurationType().fraction());
        // since durationType and ticks aren't global but supposed to
        // not account  for tuplets, c2 can match c1 even if it's in another tuplet
        c2->setDurationType(c1->durationType());
        c2->setTicks(c1->ticks());

        TremoloTwoChord* tremolo = Factory::createTremoloTwoChord(c1);
        tremolo->setTremoloType(TremoloType(tremoloBeamsNum));
        tremolo->setTrack(curTrackIdx);
        tremolo->setVisible(c1->notes().front()->visible());
        tremolo->setParent(c1);
        tremolo->setChords(c1, c2);
        c1->setTremoloTwoChord(tremolo);
    }
}

static void createTupletMap(const std::vector<EntryFrame::TupletInfo>& tupletInfo,
                            std::vector<ReadableTuplet>& tupletMap, std::vector<ReadableTuplet>& tremoloMap, int voice)
{
    const bool forVoice2 = bool(voice);
    for (const auto& tuplet : tupletInfo) {
        if (forVoice2 != tuplet.voice2 || tuplet.tuplet->calcRatio() == 0) {
            continue;
        }
        ReadableTuplet rTuplet;
        rTuplet.startTick  = musxFractionToFraction(tuplet.startDura).reduced();
        rTuplet.endTick    = musxFractionToFraction(tuplet.endDura).reduced();
        rTuplet.musxTuplet = tuplet.tuplet;
        if (tuplet.calcIsTremolo() && tuplet.numEntries() == 2) {
            tremoloMap.emplace_back(rTuplet);
        } else {
            tupletMap.emplace_back(rTuplet);
        }
    }

    for (size_t i = 0; i < tupletMap.size(); ++i) {
        for (size_t j = 0; j < tupletMap.size(); ++j) {
            if (i == j) {
                continue;
            }

            if (tupletMap[i].startTick >= tupletMap[j].startTick && tupletMap[i].endTick <= tupletMap[j].endTick
                && (tupletMap[i].startTick > tupletMap[j].startTick || tupletMap[i].endTick < tupletMap[j].endTick
                    || tupletMap[i].layer == tupletMap[j].layer)) {
                tupletMap[i].layer = std::max(tupletMap[i].layer, tupletMap[j].layer + 1);
            }
        }
    }

    std::sort(tupletMap.begin(), tupletMap.end(), [](const ReadableTuplet& a, const ReadableTuplet& b) {
        return (a.layer == b.layer) ? (a.startTick < b.startTick) : (a.layer < b.layer);
    });
}

void FinaleParser::createTupletsFromMap(Measure* measure, track_idx_t curTrackIdx, std::vector<ReadableTuplet>& tupletMap)
{
    auto transferTupletProperties = [this](const MusxInstance<details::TupletDef>& musxTuplet, Tuplet* scoreTuplet) {
        scoreTuplet->setNumberType(toMuseScoreTupletNumberType(musxTuplet->numStyle));
        // actual number object is generated on score layout

        scoreTuplet->setVisible(!musxTuplet->hidden);
        if (musxTuplet->autoBracketStyle != options::TupletOptions::AutoBracketStyle::Always) {
            // Can't be determined until we write all the notes/beams
            /// @todo write this setting on a second pass, along with musxTuplet->posStyle
            logger()->logWarning(String(u"Unsupported"));
        }

        // Following options are supported as styles but not as individual properties:
        collectGlobalProperty(Sid::tupletOutOfStaff, musxTuplet->avoidStaff);
        collectGlobalProperty(Sid::tupletNumberRythmicCenter, musxTuplet->metricCenter);
        collectGlobalProperty(Sid::tupletExtendToEndOfDuration, musxTuplet->fullDura);
        collectGlobalProperty(Sid::tupletBracketHookHeight,
                              Spatium(doubleFromEvpu(-(std::max)(musxTuplet->leftHookLen, musxTuplet->rightHookLen)))); /// or use average

        if (importAllPositions()) {
            scoreTuplet->setAutoplace(musxTuplet->smartTuplet);
            // separate bracket/number offset not supported, just add it to the whole tuplet for now
            /// @todo needs to be negated?
            scoreTuplet->setOffset(evpuToPointF(musxTuplet->tupOffX + musxTuplet->brackOffX,
                                                musxTuplet->tupOffY + musxTuplet->brackOffY) * scoreTuplet->spatium());
            // bracket extensions
            /// @todo account for the fact that Finale uses 'main note' for total bracket width, an option not in MuseScore. See #16973
            scoreTuplet->setUserPoint1(evpuToPointF(-musxTuplet->leftHookExt, 0) * scoreTuplet->spatium());
            scoreTuplet->setUserPoint2(evpuToPointF(musxTuplet->rightHookExt, -musxTuplet->manualSlopeAdj) * scoreTuplet->spatium());
            /// @todo fix position calculations and adjust post layout
            if (musxTuplet->alwaysFlat) {
                scoreTuplet->setUserPoint2(PointF(scoreTuplet->userP2().x(), scoreTuplet->userP1().y()));
            }
        }

        // unsupported: breakBracket, ignoreHorzNumOffset, allowHorz, useBottomNote
    };

    // create Tuplets as needed, starting with the outermost
    for (size_t i = 1; i < tupletMap.size(); ++i) {
        TDuration baseLen = musxDurationInfoToDuration(calcDurationInfoFromEdu(tupletMap[i].musxTuplet->referenceDuration));
        Fraction tupletRatio = Fraction(tupletMap[i].musxTuplet->displayNumber,
                                        tupletMap[i].musxTuplet->referenceNumber * tupletMap[i].musxTuplet->referenceDuration
                                        / tupletMap[i].musxTuplet->displayDuration);
        if (!baseLen.isValid() || tupletRatio <= Fraction(0, 1)) {
            logger()->logWarning(String(u"Given Tuplet duration not supported in MuseScore"));
            continue;
        }
        tupletMap[i].scoreTuplet = Factory::createTuplet(measure);
        tupletMap[i].scoreTuplet->setTrack(curTrackIdx);
        tupletMap[i].scoreTuplet->setTick(measure->tick() + tupletMap[i].startTick);
        tupletMap[i].scoreTuplet->setParent(measure);
        tupletMap[i].scoreTuplet->setRatio(tupletRatio);
        tupletMap[i].scoreTuplet->setBaseLen(baseLen);
        Fraction f = baseLen.fraction() * tupletRatio.denominator();
        tupletMap[i].scoreTuplet->setTicks(f.reduced());
        logger()->logInfo(String(u"Detected Tuplet: Starting at %1, duration: %2, ratio: %3, base duration: %4").arg(
                              tupletMap[i].startTick.toString(), f.reduced().toString(),
                              tupletRatio.toString(), baseLen.fraction().toString()));
        transferTupletProperties(tupletMap[i].musxTuplet, tupletMap[i].scoreTuplet);
        collectElementStyle(tupletMap[i].scoreTuplet);
        // reparent tuplet if needed
        size_t parentIndex = indexOfParentTuplet(tupletMap, i);
        if (tupletMap[parentIndex].layer >= 0) {
            tupletMap[parentIndex].scoreTuplet->add(tupletMap[i].scoreTuplet);
        }
    }
}

void FinaleParser::importEntries()
{
    // Add entries (notes, rests, tuplets)
    if (m_score->measures()->empty()) {
        logger()->logWarning(String(u"Add entries: No measures in score"));
        return;
    }
    MusxInstanceList<others::Measure> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    MusxInstanceList<others::StaffUsed> musxScrollView = m_doc->getScrollViewStaves(m_currentMusxPartId);
    std::vector<engraving::Note*> notesWithUnmanagedTies;
    m_track2Layer.assign(m_score->ntracks(), std::map<int, LayerIndex> {});
    for (const MusxInstance<others::StaffUsed>& musxScrollViewItem : musxScrollView) {
        StaffCmper musxStaffId = musxScrollViewItem->staffId;
        staff_idx_t curStaffIdx = muse::value(m_inst2Staff, musxStaffId, muse::nidx);
        IF_ASSERT_FAILED(curStaffIdx != muse::nidx) {
            logger()->logWarning(String(u"Add entries: Musx inst value not found."), m_doc, musxStaffId, 1);
            continue;
        }
        track_idx_t staffTrackIdx = staff2track(curStaffIdx);

        Staff* curStaff = m_score->staff(curStaffIdx);

        for (const MusxInstance<others::Measure>& musxMeasure : musxMeasures) {
            MeasCmper measureId = musxMeasure->getCmper();
            musx::util::Fraction legacyPickupSpacer = musxMeasure->calcMinLegacyPickupSpacer(musxStaffId);
            Fraction currTick = muse::value(m_meas2Tick, measureId, Fraction(-1, 1));
            Measure* measure = !currTick.negative() ? m_score->tick2measure(currTick) : nullptr;
            bool measureHasVoices = false;
            if (!measure) {
                logger()->logWarning(String(u"Unable to retrieve measure by tick"), m_doc, musxStaffId, measureId);
                break;
            }
            details::GFrameHoldContext gfHold(musxMeasure->getDocument(), m_currentMusxPartId, musxStaffId, measureId, legacyPickupSpacer);
            bool processContext = bool(gfHold);
            if (processContext && gfHold.calcIsCuesOnly()) {
                logger()->logWarning(String(u"Cue notes not yet supported"), m_doc, musxStaffId, measureId);
                processContext = false;
            }
            // Note from RGP: You cannot short-circuit out of this code with a `if (!processContext) continue` statement.
            // The code after the if statement must still be executed.
            if (processContext) {
                // gfHold.calcVoices() guarantees that every layer/voice returned contains entries
                std::map<LayerIndex, int> finaleLayers = gfHold.calcVoices();
                std::unordered_map<int, track_idx_t> finaleVoiceMap = mapFinaleVoices(finaleLayers, musxStaffId, measureId);
                for (const auto& finaleLayer : finaleLayers) {
                    const LayerIndex layer = finaleLayer.first;
                    MusxInstance<EntryFrame> entryFrame = gfHold.createEntryFrame(layer);
                    if (!entryFrame) {
                        logger()->logWarning(String(u"Layer %1 not found.").arg(int(layer)), m_doc, musxStaffId, measureId);
                        continue;
                    }
                    const int maxV1V2 = finaleLayer.second ? 1 : 0;
                    for (int voice = 0; voice <= maxV1V2; voice++) {
                        // calculate current track
                        voice_idx_t voiceOff = muse::value(finaleVoiceMap, createFinaleVoiceId(layer, bool(voice)), muse::nidx);
                        IF_ASSERT_FAILED(voiceOff != muse::nidx && voiceOff < VOICES) {
                            logger()->logWarning(String(u"Encountered incorrectly mapped voice ID for layer %1").arg(
                                                     int(layer) + 1), m_doc, musxStaffId, measureId);
                            continue;
                        }

                        track_idx_t curTrackIdx = staffTrackIdx + voiceOff;
                        m_track2Layer.at(curTrackIdx).emplace(currTick.ticks(), layer);
                        if (curTrackIdx != staffTrackIdx) {
                            measureHasVoices = true;
                        }

                        // generate tuplet map, tremolo map and create tuplets
                        // trick: insert invalid 'tuplet' spanning the whole measure. useful for fallback
                        ReadableTuplet rTuplet;
                        rTuplet.startTick = Fraction(0, 1);
                        rTuplet.endTick = measure->stretchedLen(curStaff).reduced(); // accounts for local timesigs (needed?)
                        rTuplet.layer = -1;
                        std::vector<ReadableTuplet> tupletMap = { rTuplet };
                        std::vector<ReadableTuplet> tremoloMap;
                        createTupletMap(entryFrame->tupletInfo, tupletMap, tremoloMap, voice);
                        createTupletsFromMap(measure, curTrackIdx, tupletMap);

                        // MuseScore cannot use the default mode of EntryInfoPtr::InterpretedIterator
                        // for two reasons:
                        //
                        // 1. Beam processing: MuseScore must create all CRs (ChordRests) involved in a
                        //    beam *before* running processBeams. The default mode remaps continuation
                        //    notes into later measures, preventing us from constructing all CRs in
                        //    the beam prior to beam processing. (This could eventually be
                        //    addressed by restructuring processBeams to run in a separate pass.)
                        //
                        // 2. Grace notes: MuseScore’s current grace-note model requires access to the
                        //    hidden source entries in their raw measure positions so grace notes can be
                        //    attached to the correct CR. The default mode substitutes those hidden
                        //    entries with their displayed counterparts, obscuring the true attachment
                        //    point. This limitation may disappear if MuseScore ever supports grace notes
                        //    as independent notes rather than CR-attached items.
                        //
                        constexpr static bool remapBeamovers = false;
                        // add chords and rests
                        for (EntryInfoPtr::InterpretedIterator result = entryFrame->getFirstInterpretedIterator(voice + 1, remapBeamovers);
                             result; result = result.getNext()) {
                            processEntryInfo(result, curTrackIdx, measure, /*graceNotes*/ false, notesWithUnmanagedTies, tupletMap,
                                             bool(maxV1V2));
                        }
                        for (EntryInfoPtr::InterpretedIterator result = entryFrame->getFirstInterpretedIterator(voice + 1, remapBeamovers);
                             result; result = result.getNext()) {
                            processEntryInfo(result, curTrackIdx, measure, /*graceNotes*/ true, notesWithUnmanagedTies, tupletMap,
                                             bool(maxV1V2));
                        }

                        // add tremolos
                        processTremolos(tremoloMap, curTrackIdx, measure);

                        // create beams
                        for (EntryInfoPtr entryInfoPtr = entryFrame->getFirstInVoice(voice + 1); entryInfoPtr;
                             entryInfoPtr = entryInfoPtr.getNextInVoice(voice + 1)) {
                            processBeams(entryInfoPtr, curTrackIdx);
                        }
                        // m_entryNumber2CR.clear(); /// @todo use 2 maps, one of which clears itself, to make beaming more efficient
                    }
                }
            }
            // Avoid corruptions: fill in any gaps in existing voices...
            // logger()->logInfo(String(u"Fixing corruptions for measure at staff %1, tick %2").arg(String::number(curStaffIdx), currTick.toString()));
            measure->checkMeasure(curStaffIdx);
            // ...and make sure voice 1 exists.
            if (!measure->hasVoice(staffTrackIdx)) {
                MusxInstance<others::StaffComposite> currMusxStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId,
                                                                                                           musxStaffId, measureId, 0);
                Segment* segment = measure->getSegmentR(SegmentType::ChordRest, Fraction(0, 1));
                Rest* rest = Factory::createRest(segment, TDuration(DurationType::V_MEASURE));
                rest->setScore(m_score);
                rest->setTicks(measure->timesig() * curStaff->timeStretch(measure->tick()));
                rest->setTrack(staffTrackIdx);
                rest->setVisible(!currMusxStaff->hideRests && !currMusxStaff->blankMeasure && !measureHasVoices);
                segment->add(rest);
            }
        }

        // Ties can only be attached to notes within a single part (instrument).
        // In the last staff of an instrument, add the ties and clear the vector.
        if (curStaff == curStaff->part()->staves().back()) {
            for (engraving::Note* note : notesWithUnmanagedTies) {
                engraving::Note* possibleEndNote = searchTieNote(note);
                Tie* tie = possibleEndNote ? Factory::createTie(note) : Factory::createLaissezVib(note);
                tie->setStartNote(note);
                tie->setTick(note->tick());
                tie->setTrack(note->track());
                tie->setParent(note);
                note->setTieFor(tie);
                if (possibleEndNote) {
                    tie->setEndNote(possibleEndNote);
                    tie->setTick2(possibleEndNote->tick());
                    tie->setTrack2(possibleEndNote->track());
                    possibleEndNote->setTieBack(tie);
                }
            }
            notesWithUnmanagedTies.clear();
        }
    }

    if (!importCustomPositions()) {
        return;
    }

    // Set stem direction for unbeamed notes (requires all voices have been imported)
    for (auto [entryNumber, chordRest] : m_entryNumber2CR) {
        if (!chordRest->isChord() || chordRest->beam()) {
            continue;
        }
        Chord* chord = toChord(chordRest);

        // Stem direction
        if (chord->stemDirection() == DirectionV::AUTO) {
            DirectionV dir = getDirectionVForLayer(chordRest);
            chord->setStemDirection(dir);
            if (dir != DirectionV::AUTO) {
                m_fixedChords.insert(chord);
            }
        }
        bool up = chord->stemDirection() == DirectionV::UP;
        if (chord->stemDirection() == DirectionV::AUTO) {
            if (chord->isGrace()) {
                // Can't be set further up due to layers
                up = true;
            } else if (chord->staffMove() == 0) {
                int middleLine = chord->staffType()->lines() - 1;
                if (chord->staff()->isTabStaff(chord->segment()->tick())) {
                    up = chord->downNote()->string() + chord->upNote()->string() > middleLine;
                } else {
                    for (engraving::Note* n : chord->notes()) {
                        n->updateLine();
                    }
                    up = chord->downNote()->line() + chord->upNote()->line() > middleLine * 2;
                }
            } else {
                up = chord->staffMove() > 0;
            }
            chord->setStemDirection(up ? DirectionV::UP : DirectionV::DOWN);
        }

        // Stem details
        if (chord->stem()) {
            if (const auto& stemAlt = m_doc->getDetails()->get<details::StemAlterations>(m_currentMusxPartId, entryNumber)) {
                if (up) {
                    setAndStyleProperty(chord->stem(), Pid::OFFSET,
                                        PointF(doubleFromEvpu(stemAlt->upHorzAdjust) * chord->defaultSpatium(), 0.0));
                    setAndStyleProperty(chord->stem(), Pid::USER_LEN, absoluteSpatiumFromEvpu(stemAlt->upVertAdjust, chord->stem()));
                } else {
                    setAndStyleProperty(chord->stem(), Pid::OFFSET,
                                        PointF(doubleFromEvpu(stemAlt->downHorzAdjust) * chord->defaultSpatium(), 0.0));
                    setAndStyleProperty(chord->stem(), Pid::USER_LEN, absoluteSpatiumFromEvpu(-stemAlt->downVertAdjust, chord->stem()));
                }
            }

            // No support for custom stem shapes, but read if invisible
            if (up) {
                if (const auto& customStem = m_doc->getDetails()->get<details::CustomUpStem>(m_currentMusxPartId, entryNumber)) {
                    chord->stem()->setVisible(customStem->calcIsHiddenStem());
                }
            } else {
                if (const auto& customStem = m_doc->getDetails()->get<details::CustomDownStem>(m_currentMusxPartId, entryNumber)) {
                    chord->stem()->setVisible(customStem->calcIsHiddenStem());
                }
            }
        }
    }
}

static double systemPosByLine(ChordRest* cr, bool up)
{
    IF_ASSERT_FAILED(cr->measure()->system()) {
        return 0.0;
    }
    int line = 0;
    if (cr->isChord()) {
        engraving::Note* n = up ? toChord(cr)->upNote() : toChord(cr)->downNote();
        line = cr->staffType()->isTabStaff() ? n->string() * 2 : n->line();
    } else if (cr->isRest()) {
        line = std::round((cr->pos().y() - cr->offset().y()) * 2); // Beams in Finale are calculated using the default rest position
    }
    return cr->measure()->system()->staff(cr->vStaffIdx())->y() + cr->staffOffsetY()
           + (line * cr->spatium() * cr->staffType()->lineDistance().val() * 0.5);
}

DirectionV FinaleParser::calculateTieDirection(Tie* tie, EntryNumber entryNumber)
{
    // MuseScore requires all tie segments to have the same direction.
    // As such, if the direction has already been set, don't recalculate.
    if (tie->slurDirection() != DirectionV::AUTO) {
        return tie->slurDirection();
    }
    // If MS ever does allow it, pass the note as a parameter instead.
    // We should then only calculate tieEnds when there are more than one segment.
    engraving::Note* note = tie->startNote() ? tie->startNote() : tie->endNote();
    assert(note);

    Chord* c = note->chord();
    DirectionV stemDir = c->beam() ? c->beam()->direction() : c->stemDirection();
    if (stemDir == DirectionV::AUTO) {
        logger()->logWarning(String(
                                 u"The stem direction for ChordRest corresponding to EntryNumber %1 could not be determined. Getting it from EntryInfoPtr instead.").arg(
                                 entryNumber));
        EntryInfoPtr entryInfoPtr = EntryInfoPtr::fromEntryNumber(m_doc, m_currentMusxPartId, entryNumber);
        IF_ASSERT_FAILED(entryInfoPtr) {
            logger()->logWarning(String(
                                     u"The stem direction for ChordRest corresponding to EntryNumber %1 could not be deterimed at all. Returning AUTO.").arg(
                                     entryNumber));
            return DirectionV::AUTO;
        }
        stemDir = entryInfoPtr.calcUpStem() ? DirectionV::UP : DirectionV::DOWN;
    }

    // Inherit the stem direction only when the chord has a fixed direction, and the layer says to do so.
    const auto& layerInfo = layerAttributes(c->tick(), c->track());
    if (layerInfo && layerInfo->freezTiesToStems && muse::contains(m_fixedChords, c)) {
        DirectionV layerDir = getDirectionVForLayer(toChordRest(c));
        if (layerDir != DirectionV::AUTO) {
            return layerDir;
        }
    }
    // Check in both layers where possible
    if (tie->startNote() && tie->endNote()) {
        Chord* c2 = c == tie->startNote()->chord() ? tie->endNote()->chord() : tie->startNote()->chord();
        const auto& otherLayerInfo = layerAttributes(c2->tick(), c2->track());
        if (otherLayerInfo && otherLayerInfo->freezTiesToStems && muse::contains(m_fixedChords, c2)) {
            DirectionV layerDir = getDirectionVForLayer(toChordRest(c2));
            if (layerDir != DirectionV::AUTO) {
                return layerDir;
            }
        }
    }

    if (c->staffMove() != 0) {
        return c->staffMove() > 0 ? DirectionV::UP : DirectionV::DOWN;
    }

    const MusxInstance<options::TieOptions>& config = musxOptions().tieOptions;

    if (c->notes().size() > 1) {
        // Notes are sorted from lowest to highest
        const size_t noteIndex = muse::indexOf(c->notes(), note);
        const size_t noteCount = c->notes().size();

        // Outer notes ignore tie preferences
        if (noteIndex == 0) {
            return DirectionV::DOWN;
        }
        if (noteIndex == noteCount - 1) {
            return DirectionV::UP;
        }

        const bool tabStaff = c->staffType()->isTabStaff();
        const int line = tabStaff ? note->string() : note->line();

        if (!tabStaff && config->secondsPlacement == options::TieOptions::SecondsPlacement::ShiftForSeconds) {
            bool isUpper2nd = false;
            bool isLower2nd = false;
            for (const engraving::Note* n : c->notes()) {
                isLower2nd = isLower2nd || (line == n->line() + 1);
                isUpper2nd = isUpper2nd || (line == n->line() - 1);
            }

            if (!isUpper2nd && isLower2nd) {
                return DirectionV::DOWN;
            }
            if (isUpper2nd && !isLower2nd) {
                return DirectionV::UP;
            }
        }

        if (config->chordTieDirType != options::TieOptions::ChordTieDirType::StemReversal) {
            if (noteIndex < noteCount / 2) {
                return DirectionV::DOWN;
            }
            if (noteIndex >= (noteCount + 1) / 2) {
                return DirectionV::UP;
            }

            if (config->chordTieDirType == options::TieOptions::ChordTieDirType::OutsideInside) {
                return (stemDir == DirectionV::UP) ? DirectionV::DOWN : DirectionV::UP;
            }
        }

        const int middleLine = (c->staffType()->lines() - 1) * (!tabStaff ? 2 : 1);
        return (line * 2 > middleLine) ? DirectionV::DOWN : DirectionV::UP;
    }

    // Single-note chords
    if (config->mixedStemDirection != options::TieOptions::MixedStemDirection::OppositeFirst) {
        DirectionV adjacentStemDir = DirectionV::AUTO;
        const engraving::Note* startNote = tie->startNote();
        const engraving::Note* endNote = tie->endNote();

        if (note == startNote) {
            if (endNote) {
                const Chord* endChord = endNote->chord();
                DirectionV endDir = endChord->beam() ? endChord->beam()->direction() : endChord->stemDirection();
                adjacentStemDir = endDir;
            } else {
                const ChordRest* nextCR = nextChordRest(c);
                if (nextCR && nextCR->isChord()) {
                    const Chord* nextChord = toChord(nextCR);
                    DirectionV nextDir = nextChord->beam() ? nextChord->beam()->direction() : nextChord->stemDirection();
                    assert(nextDir != DirectionV::AUTO); // perhaps not necessary
                    adjacentStemDir = nextDir;

                    /// @todo Finale tests for a stem freeze and a V2Launch here, but we already freeze (most) stems elsewhere in the code.
                    /// We need to find a way around this extreme edge case.
                    /// Using tracks to detect a V2Launch doesn't work, because those could be a layer change instead.
                    // if (nextDir == DirectionV::AUTO && nextChordRest->v2Launch && adjacentStemDir == stemDir) {
                    //     nextChordRest = nextChordRest(nextChordRest);
                    //     if (nextChordRest) {
                    //         nextDir = nextChordRest->beam() ? nextChordRest->beam()->direction() : nextChordRest->stemDirection();
                    //         assert(nextDir != DirectionV::AUTO); // perhaps not necessary
                    //         adjacentStemDir = nextDir;
                    //     }
                    // }
                }
            }
        } else {
            if (startNote) {
                const Chord* startChord = startNote->chord();
                DirectionV startDir = startChord->beam() ? startChord->beam()->direction() : startChord->stemDirection();
                assert(startDir != DirectionV::AUTO); // perhaps not necessary
                adjacentStemDir = startDir;
            }
        }

        if (adjacentStemDir != DirectionV::AUTO && adjacentStemDir != stemDir) {
            if (config->mixedStemDirection == options::TieOptions::MixedStemDirection::Over) {
                return DirectionV::UP;
            } else {
                return DirectionV::DOWN;
            }
        }
    }

    return (stemDir == DirectionV::UP) ? DirectionV::DOWN : DirectionV::UP;
}

static TiePlacement calculateTiePlacement(Tie* tie, bool useOuterPlacement)
{
    engraving::Note* startN = tie->startNote();
    const Chord* startChord = startN ? startN->chord() : nullptr;
    engraving::Note* endN = tie->endNote();
    const Chord* endChord = endN ? endN->chord() : nullptr;

    const bool styleIsOuter = tie->style().styleV(Sid::tiePlacementSingleNote).value<TiePlacement>() == TiePlacement::OUTSIDE;
    const bool defaultPlacement = useOuterPlacement == styleIsOuter;
    const TiePlacement outsideValue = (styleIsOuter && defaultPlacement) ? TiePlacement::AUTO : TiePlacement::OUTSIDE;
    const TiePlacement insideValue = (!styleIsOuter && defaultPlacement) ? TiePlacement::AUTO : TiePlacement::INSIDE;

    // Single-note chords
    if ((!startChord || startChord->notes().size() <= 1) && (!endChord || endChord->notes().size() <= 1)) {
        return useOuterPlacement ? outsideValue : insideValue;
    }

    // Regardless of setting, Finale always sets ties within a chord (i.e. on not-outermost notes) to inner placement.
    const bool isOutside = tie->up()
                           ? (!startChord || startN == startChord->upNote()) && (!endChord || endN == endChord->upNote())
                           : (!startChord || startN == startChord->downNote()) && (!endChord || endN == endChord->downNote());
    return (useOuterPlacement && isOutside) ? outsideValue : insideValue;
}

void FinaleParser::importEntryAdjustments()
{
    logger()->logDebugTrace(String(u"Importing entry adjustments..."));

    // Rebase rest offsets (must happen after layout but before beaming)
    for (auto [entryNumber, chordRest] : m_entryNumber2CR) {
        if (chordRest->isRest() && !toRest(chordRest)->alignWithOtherRests() && chordRest->ldata()->isSetPos()) {
            toRest(chordRest)->ryoffset() -= chordRest->ldata()->pos().y();
        }
    }

    if (!importAllPositions()) {
        return;
    }

    // Beam positions
    for (auto [entryNumber, chordRest] : m_entryNumber2CR) {
        if (!chordRest->beam() || chordRest->beamMode() != BeamMode::BEGIN) {
            continue;
        }

        Beam* beam = chordRest->beam();
        // Invisible staves
        if (!beam->system()) {
            continue;
        }
        const double beamStaffY = beam->system()->staff(beam->staffIdx())->y() + beam->staffOffsetY();
        const double middleLinePos = beamStaffY + (beam->staffType()->lines() - 1) * beam->spatium()
                                     * beam->staffType()->lineDistance().val() * 0.5;

        // Set beam direction
        if (beam->direction() == DirectionV::AUTO) {
            beam->doSetDirection(getDirectionVForLayer(chordRest));
        }
        bool up = beam->direction() == DirectionV::UP;
        if (beam->direction() == DirectionV::AUTO) {
            if (beam->elements().front()->isGrace()) {
                up = true;
            } else {
                // This ugly calculation is needed for cross-staff beams.
                // But even for non-cross beams, Finale's auto direction can differ.
                double topPos = DBL_MAX;
                double bottomPos = -DBL_MAX;
                for (ChordRest* cr : beam->elements()) {
                    if (cr->isRest()) {
                        continue;
                    }
                    topPos = std::min(topPos, systemPosByLine(cr, true));
                    bottomPos = std::max(bottomPos, systemPosByLine(cr, false));
                }
                up = muse::RealIsEqualOrLess(middleLinePos - topPos, bottomPos - middleLinePos);
            }
            beam->doSetDirection(up ? DirectionV::UP : DirectionV::DOWN);
        } else {
            // This may not be correct behaviour for stem-reversed notes.
            for (ChordRest* cr : beam->elements()) {
                if (cr->isChord()) {
                    m_fixedChords.insert(toChord(cr));
                }
            }
        }
        for (ChordRest* cr : beam->elements()) {
            if (cr->isChord()) {
                toChord(cr)->setStemDirection(beam->direction());
            }
        }

        // Calculate non-adjusted position, in system coordinates
        ChordRest* startCr = beam->elements().front();
        ChordRest* endCr = beam->elements().back();
        double stemLengthAdjust =  (up ? -1.0 : 1.0) * doubleFromEvpu(musxOptions().stemOptions->stemLength)
                                  * (startCr->isGrace() ? m_score->style().styleD(Sid::graceNoteMag) : 1.0);
        double preferredStart = systemPosByLine(startCr, up) + stemLengthAdjust * startCr->spatium();
        double preferredEnd = systemPosByLine(endCr, up) + stemLengthAdjust * endCr->spatium();
        auto getInnermost = [&]() {
            return up ? std::max(preferredStart, preferredEnd) : std::min(preferredStart, preferredEnd); // farthest start/end note
        };
        auto getOutermost = [&]() {
            return up ? std::min(preferredStart, preferredEnd) : std::max(preferredStart, preferredEnd); // closest start/end note
        };
        double innermost;
        double outermost = getOutermost();

        // Flatten beams as needed
        bool forceFlatten = musxOptions().beamOptions->beamingStyle == options::BeamOptions::FlattenStyle::AlwaysFlat;
        setAndStyleProperty(beam, Pid::BEAM_NO_SLOPE, forceFlatten, true);
        if (!forceFlatten && !muse::RealIsEqual(preferredStart, preferredEnd) && beam->elements().size() > 2) {
            if (musxOptions().beamOptions->beamingStyle == options::BeamOptions::FlattenStyle::OnExtremeNote) {
                for (ChordRest* cr : beam->elements()) {
                    if (cr == startCr || cr == endCr || cr->isRest()) {
                        continue;
                    }
                    double beamPos = systemPosByLine(cr, up) + stemLengthAdjust * cr->spatium();
                    if (up ? muse::RealIsEqualOrLess(beamPos, outermost) : muse::RealIsEqualOrMore(beamPos, outermost)) {
                        forceFlatten = true;
                        break;
                    }
                }
            } else if (musxOptions().beamOptions->beamingStyle == options::BeamOptions::FlattenStyle::OnStandardNote) {
                double outermost2 = up ? DBL_MAX : -DBL_MAX;
                outermost -= stemLengthAdjust * beam->spatium();
                bool downwardsContour = preferredEnd > preferredStart;
                bool notesFollowContour = true; // Assume we can slope
                double prevPos = systemPosByLine(startCr, up);

                for (ChordRest* cr : beam->elements()) {
                    if (cr == startCr || cr->isRest()) {
                        continue;
                    }

                    double contourPos = systemPosByLine(cr, up);

                    // If the note doesn't follow the contour, don't slope.
                    // Notes having the same line is not good enough and disallows sloping.
                    if (notesFollowContour) {
                        if (downwardsContour) {
                            notesFollowContour = contourPos > prevPos;
                        } else {
                            notesFollowContour = contourPos < prevPos;
                        }
                    }
                    if (cr != endCr) {
                        outermost2 = up ? std::min(outermost2, contourPos) : std::max(outermost2, contourPos);
                        prevPos = contourPos;
                    }
                }

                // If the notes don't follow the contour, we flatten if the outermost not-start-end note
                // is closer to the middle staff line than the outermost.
                // If their distances are equal, we only flatten if the outermost note is lower (pitchwise) for up, or higher (pitchwise) for down.
                if (!notesFollowContour) {
                    double dist = std::abs(outermost - middleLinePos);
                    double dist2 = std::abs(outermost2 - middleLinePos);
                    if (up ? outermost2 > outermost : outermost2 < outermost) {
                        forceFlatten = dist2 < dist;
                    } else {
                        forceFlatten = muse::RealIsEqualOrLess(dist2, dist);
                    }
                }
                outermost = getOutermost();
            }
        }
        if (forceFlatten) {
            preferredStart = outermost;
            preferredEnd = preferredStart;
        }
        bool isFlat = forceFlatten || muse::RealIsEqual(preferredStart, preferredEnd);

        // Compute beam slope
        double slope = 0.0;
        if (!isFlat) {
            innermost = getInnermost();
            double totalX = beam->endAnchor().x() - beam->startAnchor().x();
            double maxSlope = doubleFromEvpu(musxOptions().beamOptions->maxSlope) * beam->spatium();
            double heightDifference = preferredEnd - preferredStart;
            double totalY = (heightDifference > 0) ? std::min(heightDifference, maxSlope) : std::max(heightDifference, -maxSlope);
            slope = totalY / totalX;
            if (muse::RealIsEqual(innermost, preferredEnd)) {
                preferredEnd = preferredStart + slope * totalX;
            } else {
                preferredStart = preferredEnd + slope * -totalX;
            }
        }

        // Ensure middle staff line distance is respected
        outermost = getOutermost();
        if (up ? outermost > middleLinePos : outermost < middleLinePos) {
            const double diff = middleLinePos - outermost;
            preferredStart += diff;
            preferredEnd += diff;
        }
        innermost = getInnermost();
        const double middleLineLimit = middleLinePos + beam->spatium() * beam->staffType()->lineDistance().val()
                                       * doubleFromEvpu(musxOptions().beamOptions->maxFromMiddle) * (up ? 1.0 : -1.0);
        if (up ? (middleLineLimit < innermost) : (middleLineLimit > innermost)) {
            const double middleLineAdjust = middleLineLimit - innermost;
            preferredStart += middleLineAdjust;
            preferredEnd += middleLineAdjust;
        }

        // Ensure minimum stem lengths
        outermost = getOutermost();
        for (ChordRest* cr : beam->elements()) {
            if (cr == startCr || cr == endCr || cr->isRest()) {
                continue;
            }
            const double minPos = systemPosByLine(cr, up) + stemLengthAdjust * cr->spatium();
            if (up ? muse::RealIsEqualOrMore(minPos, outermost) : muse::RealIsEqualOrLess(minPos, outermost)) {
                // Yes, this means that stem lengths won't effectively always be respected.
                // But this bug mimics Finale behaviour...
                continue;
            }
            double curPos = preferredStart;
            if (!isFlat) {
                const double startX = rendering::score::BeamTremoloLayout::chordBeamAnchorX(beam->ldata(), cr, ChordBeamAnchorType::Start);
                curPos += slope * (startX - beam->startAnchor().x());
            }
            if (up ? minPos < curPos : minPos > curPos) {
                double difference = minPos - curPos;
                preferredStart += difference;
                preferredEnd += difference;
            }
        }

        // Beam alterations
        auto getAlterPosition = [beam](const MusxInstance<details::BeamAlterations>& beamAlter) {
            if (!beamAlter || !beamAlter->isActive()) {
                return PointF();
            }
            beam->setVisible(beamAlter->calcEffectiveBeamWidth() != 0);
            return evpuToPointF(beamAlter->leftOffsetY, beamAlter->leftOffsetY + beamAlter->rightOffsetY) * beam->spatium();
        };
        /// @todo combine these two, one day
        auto getAlterFeatherU = [beam](const MusxInstanceList<details::SecondaryBeamAlterationsUpStem>& beamAlterList) {
            if (!beamAlterList.empty()) {
                for (const auto& beamAlter : beamAlterList) {
                    if (!beamAlter->isActive() || eduToFraction(beamAlter->dura) != Fraction(1, 16)) {
                        continue;
                    }
                    return evpuToPointF(beamAlter->leftOffsetY,
                                        beamAlter->leftOffsetY + beamAlter->rightOffsetY) * beam->spatium() / beam->beamDist();
                }
            }
            return PointF();
        };
        auto getAlterFeatherD = [beam](const MusxInstanceList<details::SecondaryBeamAlterationsDownStem>& beamAlterList) {
            if (!beamAlterList.empty()) {
                for (const auto& beamAlter : beamAlterList) {
                    if (!beamAlter->isActive() || eduToFraction(beamAlter->dura) != Fraction(1, 16)) {
                        continue;
                    }
                    return evpuToPointF(beamAlter->leftOffsetY,
                                        beamAlter->leftOffsetY + beamAlter->rightOffsetY) * beam->spatium() / beam->beamDist();
                }
            }
            return PointF();
        };
        PointF posAdjust;
        PointF feathering(1.0, 1.0);
        if (up) {
            posAdjust = getAlterPosition(m_doc->getDetails()->get<details::BeamAlterationsUpStem>(m_currentMusxPartId, entryNumber));
            feathering
                -= getAlterFeatherU(m_doc->getDetails()->getArray<details::SecondaryBeamAlterationsUpStem>(m_currentMusxPartId,
                                                                                                           entryNumber));
        } else {
            posAdjust = getAlterPosition(m_doc->getDetails()->get<details::BeamAlterationsDownStem>(m_currentMusxPartId, entryNumber));
            feathering
                += getAlterFeatherD(m_doc->getDetails()->getArray<details::SecondaryBeamAlterationsDownStem>(m_currentMusxPartId,
                                                                                                             entryNumber)); // -=?
        }

        // Smoothing
        if (!musxOptions().beamOptions->spanSpace && !muse::RealIsEqual(preferredStart, preferredEnd) && !startCr->isGrace()) {
            innermost = getInnermost();
            if (up ? muse::RealIsEqualOrMore(innermost, beamStaffY) : innermost < beamStaffY + beam->staff()->staffHeight(beam->tick())) {
                /// @todo figure out these calculations - they seem more complex than the rest of the code
                /// For now, set to default position and add offset
                logger()->logInfo(String(u"Beam at tick %1, track %2 should inherit default placement.").arg(beam->tick().toString(),
                                                                                                             String::number(beam->track())));
                if (beam->cross()) {
                    int crossStaffMove = (up ? beam->minCRMove() : beam->maxCRMove() + 1) - beam->defaultCrossStaffIdx();
                    setAndStyleProperty(beam, Pid::BEAM_CROSS_STAFF_MOVE, crossStaffMove);
                }
                preferredStart = beam->startAnchor().y() + beamStaffY + beam->beamWidth() * (up ? -0.5 : 0.5);
                preferredEnd = beam->endAnchor().y() + beamStaffY + beam->beamWidth() * (up ? -0.5 : 0.5);
            }
        }

        preferredStart -= posAdjust.x();
        preferredEnd -= posAdjust.y();

        const double staffWidthAdjustment = beamStaffY + beam->beamWidth() * (up ? -0.5 : 0.5);
        preferredStart -= staffWidthAdjustment;
        preferredEnd -= staffWidthAdjustment;

        setAndStyleProperty(beam, Pid::GROW_LEFT, feathering.x());
        setAndStyleProperty(beam, Pid::GROW_RIGHT, feathering.y());
        setAndStyleProperty(beam, Pid::USER_MODIFIED, true);
        setAndStyleProperty(beam, Pid::GENERATED, false);
        setAndStyleProperty(beam, Pid::BEAM_POS, PairF(preferredStart / beam->spatium(), preferredEnd / beam->spatium()));
    }

    for (auto [entryNumber, chordRest] : m_entryNumber2CR) {
        // Rebase dot offset
        /// @todo dot direction
        if (chordRest->dots() > 0 && chordRest->ldata()->isSetPos()) {
            const double dotDistance = m_score->style().styleMM(Sid::dotNoteDistance) * chordRest->staff()->staffMag(chordRest);
            if (chordRest->isChord()) {
                double rightmostNoteX = -DBL_MAX;
                for (engraving::Note* n : toChord(chordRest)->notes()) {
                    rightmostNoteX = std::max(rightmostNoteX, n->pos().x() + n->width());
                }
                rightmostNoteX += dotDistance;
                for (engraving::Note* n : toChord(chordRest)->notes()) {
                    // This can happen when dots are shared on layout
                    if (n->dots().empty()) {
                        continue;
                    }
                    double difference = rightmostNoteX - n->pos().x() - n->dots().front()->pos().x();
                    for (NoteDot* nd : n->dots()) {
                        nd->rxoffset() = difference;
                    }
                }
            } else if (chordRest->isRest()) {
                Rest* r = toRest(chordRest);
                if (!r->dotList().empty()) {
                    double difference = r->dotList().front()->pos().x() - (r->ldata()->bbox().right() + dotDistance); // offset to cr means no subtracting rest offset
                    for (NoteDot* nd : r->dotList()) {
                        nd->rxoffset() -= difference;
                    }
                } else {
                    logger()->logWarning(String(u"ChordRest for EntryNumber %1 has dots but Rest::dotList is empty").arg(entryNumber));
                }
            }
        }

        if (!chordRest->beam() || !chordRest->isChord()) {
            continue;
        }

        // Stems under beams (require beam direction)
        Chord* chord = toChord(chordRest);
        if (!chord->stem()) {
            continue;
        }
        if (const auto& stemAlt = m_doc->getDetails()->get<details::StemAlterationsUnderBeam>(m_currentMusxPartId, entryNumber)) {
            if (chord->beam()->direction() == DirectionV::UP) {
                setAndStyleProperty(chord->stem(), Pid::OFFSET,
                                    PointF(doubleFromEvpu(stemAlt->upHorzAdjust) * chord->defaultSpatium(), 0.0));
                setAndStyleProperty(chord->stem(), Pid::USER_LEN, absoluteSpatiumFromEvpu(stemAlt->upVertAdjust, chord->stem()));
            } else {
                setAndStyleProperty(chord->stem(), Pid::OFFSET,
                                    PointF(doubleFromEvpu(stemAlt->downHorzAdjust) * chord->defaultSpatium(), 0.0));
                setAndStyleProperty(chord->stem(), Pid::USER_LEN, absoluteSpatiumFromEvpu(-stemAlt->downVertAdjust, chord->stem()));
            }
        }
        if (chord->beam()->direction() == DirectionV::UP) {
            if (const auto& customStem = m_doc->getDetails()->get<details::CustomUpStem>(m_currentMusxPartId, entryNumber)) {
                chord->stem()->setVisible(customStem->calcIsHiddenStem());
            }
        } else {
            if (const auto& customStem = m_doc->getDetails()->get<details::CustomDownStem>(m_currentMusxPartId, entryNumber)) {
                chord->stem()->setVisible(customStem->calcIsHiddenStem());
            }
        }
    }

    // Ties
    logger()->logDebugTrace(String(u"Adjusting ties..."));
    for (auto [numbers, note] : m_entryNoteNumber2Note) {
        EntryNumber entryNumber = numbers.first;
        NoteNumber noteNumber = numbers.second;

        /// @todo offsets and contour
        auto positionTie = [this, entryNumber](Tie* tie, const MusxInstance<details::TieAlterBase>& tieAlt) {
            // Collect alterations
            logger()->logDebugTrace(String(u"Importing tie at tick %1...").arg(tie->tick().toString()));
            bool outside = musxOptions().tieOptions->useOuterPlacement;
            DirectionV direction = DirectionV::AUTO;
            if (tieAlt) {
                if (tieAlt->outerOn) {
                    logger()->logDebugTrace(String(u"Tie overrides default outer/inner placement"));
                    outside = tieAlt->outerLocal;
                }
                if (tieAlt->freezeDirection) {
                    logger()->logDebugTrace(String(u"Tie overrides default over/under placement"));
                    direction = tieAlt->down ? DirectionV::DOWN : DirectionV::UP;
                }
            } else {
                logger()->logInfo(String("Unable to to find tie details for tie at tick %1").arg(tie->tick().toString()));
            }

            // Tie direction (over/under)
            if (direction == DirectionV::AUTO) {
                direction = calculateTieDirection(tie, entryNumber);
            }
            setAndStyleProperty(tie, Pid::SLUR_DIRECTION, direction);

            // Tie placement (inner/outer)
            tie->setUp(direction == DirectionV::UP ? true : false);
            TiePlacement placement = calculateTiePlacement(tie, outside);
            setAndStyleProperty(tie, Pid::TIE_PLACEMENT, placement);
        };
        if (note->tieFor()) {
            MusxInstance<details::TieAlterBase> tieAlt = nullptr;
            for (const auto& startAlt : m_doc->getDetails()->getArray<details::TieAlterStart>(m_currentMusxPartId, entryNumber)) {
                if (startAlt->getNoteId() == noteNumber) {
                    tieAlt = startAlt;
                    break;
                }
            }
            positionTie(note->tieFor(), tieAlt);
        }
        if (note->tieBack() && !note->tieBack()->segmentsEmpty()) {
            MusxInstance<details::TieAlterBase> tieAlt = nullptr;
            for (const auto& endAlt : m_doc->getDetails()->getArray<details::TieAlterEnd>(m_currentMusxPartId, entryNumber)) {
                if (endAlt->getNoteId() == noteNumber) {
                    tieAlt = endAlt;
                    break;
                }
            }
            positionTie(note->tieBack(), tieAlt);
        }
    }

    // Staff system leading/trailing space will be imported as leading space
    for (const auto& staffSystem : m_doc->getOthers()->getArray<others::StaffSystem>(m_currentMusxPartId)) {
        if (staffSystem->extraStartSystemSpace != 0) {
            Fraction startTick = muse::value(m_meas2Tick, staffSystem->startMeas, Fraction(-1, 1));
            Measure* startMeasure = !startTick.negative() ? m_score->tick2measure(startTick) : nullptr;
            IF_ASSERT_FAILED(startMeasure) {
                logger()->logWarning(String(u"Unable to retrieve measure(s) by tick for StaffSystem"));
                continue;
            }
            Segment* s = staffSystem->placeEndSpaceBeforeBarline
                         ? startMeasure->first(SegmentType::ChordRest | SegmentType::StartRepeatBarLine)
                         : startMeasure->first(SegmentType::ChordRest);
            s->setExtraLeadingSpace(absoluteSpatiumFromEvpu(staffSystem->extraStartSystemSpace, s));
        }
        if (staffSystem->extraEndSystemSpace != 0) {
            Fraction endTick = muse::value(m_meas2Tick, staffSystem->getLastMeasure(), Fraction(-1, 1));
            Measure* endMeasure = !endTick.negative() ? m_score->tick2measure(endTick) : nullptr;
            IF_ASSERT_FAILED(endMeasure) {
                logger()->logWarning(String(u"Unable to retrieve measure(s) by tick for StaffSystem"));
                continue;
            }
            Segment* s = endMeasure->first(SegmentType::EndBarLine);
            s->setExtraLeadingSpace(absoluteSpatiumFromEvpu(staffSystem->extraEndSystemSpace, s));
        }
    }
}
}
