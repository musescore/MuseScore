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

#include "repeatlist.h"

#include "jump.h"
#include "marker.h"
#include "measure.h"
#include "score.h"
#include "segment.h"
#include "tempo.h"
#include "types.h"
#include "volta.h"

#include <algorithm>
#include <list>
#include <utility> // std::pair

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

RepeatSegment::RepeatSegment(int playbackCount)
    : tick(0), utick(0), utime(0.0), timeOffset(0.0), pause(0.0), playbackCount(playbackCount)
{
}

void RepeatSegment::addMeasure(Measure const* const m)
{
    if (m != nullptr) {
        if (measureList.empty()) {
            tick = m->tick().ticks();
        }
        if ((measureList.empty()) || (measureList.back() != m)) {
            measureList.push_back(m);
        }
    }
}

/// \brief Expands or clips my measureList up to m (including m)
void RepeatSegment::addMeasures(Measure const* const m)
{
    if (!measureList.empty()) {
        // Add up to the current measure, final measure is added outside of this condition
        Measure const* lastMeasure = measureList.back()->nextMeasure();
        if (lastMeasure && (lastMeasure->tick() < m->tick())) {     // Ensure provided reference is later than current last
            while (lastMeasure != m) {
                measureList.push_back(lastMeasure);
                lastMeasure = lastMeasure->nextMeasure();
            }
        }
        //else { // Possibly clip compared to current last measure }
        while (!measureList.empty() && (measureList.back()->tick() >= m->tick())) {
            measureList.pop_back();
        }
    }
    addMeasure(m);
}

bool RepeatSegment::containsMeasure(Measure const* const m) const
{
    for (Measure const* const measure : measureList) {
        if (measure == m) {
            return true;
        }
    }
    return false;
}

bool RepeatSegment::isEmpty() const
{
    return measureList.empty();
}

int RepeatSegment::len() const
{
    return (measureList.empty()) ? 0 : (measureList.last()->endTick().ticks() - tick);
}

void RepeatSegment::popMeasure()
{
    if (!measureList.empty()) {
        measureList.pop_back();
    }
}

//---------------------------------------------------------
//   RepeatList
//---------------------------------------------------------

RepeatList::RepeatList(Score* s)
{
    _score = s;
    idx1  = 0;
    idx2  = 0;
}

//---------------------------------------------------------
//   ~RepeatList
//---------------------------------------------------------

RepeatList::~RepeatList()
{
    qDeleteAll(*this);
}

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int RepeatList::ticks() const
{
    if (length() > 0) {
        const RepeatSegment* s = last();
        return s->utick + s->len();
    }
    return 0;
}

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void RepeatList::update(bool expand)
{
    if (!_scoreChanged && expand == _expanded) {
        return;
    }

    if (expand) {
        unwind();
    } else {
        flatten();
    }

    _scoreChanged = false;
}

//---------------------------------------------------------
//   updateTempo
//---------------------------------------------------------

void RepeatList::updateTempo()
{
    const TempoMap* tl = _score->tempomap();

    int utick = 0;
    qreal t  = 0;

    for (RepeatSegment* s : *this) {
        s->utick      = utick;
        s->utime      = t;
        qreal ct      = tl->tick2time(s->tick);
        s->timeOffset = t - ct;
        utick        += s->len();
        t            += tl->tick2time(s->tick + s->len()) - ct;
    }
}

//---------------------------------------------------------
//   utick2tick
//---------------------------------------------------------

int RepeatList::utick2tick(int tick) const
{
    unsigned n = size();
    if (n == 0) {
        return tick;
    }
    if (tick < 0) {
        return 0;
    }
    unsigned ii = (idx1 < n) && (tick >= at(idx1)->utick) ? idx1 : 0;
    for (unsigned i = ii; i < n; ++i) {
        if ((tick >= at(i)->utick) && ((i + 1 == n) || (tick < at(i + 1)->utick))) {
            idx1 = i;
            return tick - (at(i)->utick - at(i)->tick);
        }
    }
    if (MScore::debugMode) {
        qFatal("tick %d not found in RepeatList", tick);
    }
    return 0;
}

//---------------------------------------------------------
//   tick2utick
//---------------------------------------------------------

int RepeatList::tick2utick(int tick) const
{
    if (empty()) {
        return 0;
    }
    for (const RepeatSegment* s : *this) {
        if (tick >= s->tick && tick < (s->tick + s->len())) {
            return s->utick + (tick - s->tick);
        }
    }
    return last()->utick + (tick - last()->tick);
}

//---------------------------------------------------------
//   utick2utime
//---------------------------------------------------------

qreal RepeatList::utick2utime(int tick) const
{
    unsigned n = size();
    unsigned ii = (idx1 < n) && (tick >= at(idx1)->utick) ? idx1 : 0;
    for (unsigned i = ii; i < n; ++i) {
        if ((tick >= at(i)->utick) && ((i + 1 == n) || (tick < at(i + 1)->utick))) {
            int t     = tick - (at(i)->utick - at(i)->tick);
            qreal tt = _score->tempomap()->tick2time(t) + at(i)->timeOffset;
            return tt;
        }
    }
    return 0.0;
}

//---------------------------------------------------------
//   utime2utick
//---------------------------------------------------------

int RepeatList::utime2utick(qreal secs) const
{
    unsigned repeatSegmentsCount = size();
    unsigned ii = (idx2 < repeatSegmentsCount) && (secs >= at(idx2)->utime) ? idx2 : 0;
    for (unsigned i = ii; i < repeatSegmentsCount; ++i) {
        if ((secs >= at(i)->utime) && ((i + 1 == repeatSegmentsCount) || (secs < at(i + 1)->utime))) {
            idx2 = i;
            return _score->tempomap()->time2tick(secs - at(i)->timeOffset) + (at(i)->utick - at(i)->tick);
        }
    }
    if (MScore::debugMode) {
        qFatal("time %f not found in RepeatList", secs);
    }
    return 0;
}

///
/// \brief Lookup the RepeatSegment containing the given utick
///
QList<RepeatSegment*>::const_iterator RepeatList::findRepeatSegmentFromUTick(int utick) const
{
    return std::lower_bound(this->cbegin(), this->cend(), utick, [](RepeatSegment const* rs, int utick) {
        // Skip RS where endtick is less than us
        return utick > (rs->utick + rs->len());
    });
}

//---------------------------------------------------------
//   flatten
///   Make this repeat list flat (don't expand repeats)
//---------------------------------------------------------

void RepeatList::flatten()
{
    qDeleteAll(*this);
    clear();

    Measure* m = _score->firstMeasure();
    if (!m) {
        return;
    }

    RepeatSegment* s = new RepeatSegment(1);
    do {
        s->addMeasure(m);
        m = m->nextMeasure();
    } while (m);
    push_back(s);

    _expanded = false;
}

//---------------------------------------------------------
//   unwind
//    implements:
//          - repeats
//          - volta
//          - d.c. al fine
//          - d.s. al fine
//          - d.s. al coda
//---------------------------------------------------------
enum class RepeatListElementType {
    SECTION_BREAK,
    VOLTA_START,
    VOLTA_END,
    REPEAT_START,
    REPEAT_END,
    JUMP,
    MARKER
};

class RepeatListElement
{
public:
    RepeatListElementType const repeatListElementType;
    Element const* const element;
    Measure const* const measure;    // convenience, should be measure containing element
private:
    int repeatCount;

public:
    RepeatListElement(RepeatListElementType type, Element const* const el, Measure const* const m)
        : repeatListElementType(type), element(el), measure(m)
    {
        repeatCount = (type == RepeatListElementType::REPEAT_START) ? 1 : 0;
    }

    ~RepeatListElement()
    {
        // Voltas are cloned elements, we need to cleanup ourselves
        if (repeatListElementType == RepeatListElementType::VOLTA_START) {
            delete element;
        }
    }

    int getRepeatCount() const { return repeatCount; }
    void addToRepeatCount(int add) { repeatCount += add; }
};

///---------------------------------------------------------
/// \brief Collects all RepeatListElementType elements in the score
/// \details Those are the only type of elements that actually influence playback order
///---------------------------------------------------------
void RepeatList::collectRepeatListElements()
{
    QList<RepeatListElement*>* sectionRLElements = new QList<RepeatListElement*>();

    // Clear out previous listing
    for (QList<RepeatListElement*>* srle : qAsConst(_rlElements)) {
        qDeleteAll(*srle);
    }
    qDeleteAll(_rlElements);
    _rlElements.clear();

    RepeatListElement* startFromRepeatMeasure = nullptr;

    // Section breaks may occur on non-Measure frames, so must search list of all MeasureBases
    // unwinding itself will only use actual Measures
    MeasureBase* mb = _score->firstMeasure();
    // First measure of a section/score is always used as a reference REPEAT_START point
    // even if it doesn't have a start repeat
    startFromRepeatMeasure = new RepeatListElement(RepeatListElementType::REPEAT_START, mb, toMeasure(mb));
    sectionRLElements->push_back(startFromRepeatMeasure);
    // Also trace down the final actual measure of this section (in case a frame follows)
    MeasureBase* sectionEndMeasureBase = nullptr;
    // Used to track voltas; overlappings and real endings
    Volta* volta;
    std::list<Volta*> preProcessedVoltas;

    // Voltas might overlap (duplicate entries on multiple staves or "real" overlaps)
    // so we will pre-process them into cloned versions that handle those overlaps.
    // This assumes that spanners are ordered from first to last tick-wise
    for (const auto& spannerEntry : _score->spanner()) {
        if ((spannerEntry.second)->isVolta()) {
            volta = toVolta(spannerEntry.second)->clone();
            if (preProcessedVoltas.empty()) { // First entry
                preProcessedVoltas.push_back(volta);
            } else { // Compare
                std::list<Volta*> voltasToMerge;
                // List all overlapping voltas
                while ((!preProcessedVoltas.empty())
                       && (volta->startMeasure()->tick() <= preProcessedVoltas.back()->endMeasure()->tick())
                       ) {
                    voltasToMerge.push_back(preProcessedVoltas.back());
                    preProcessedVoltas.pop_back();
                }

                while (!voltasToMerge.empty()) {
                    // We'll have to shorten the already stored volta and split its remainder for merging
                    Volta* remainder = voltasToMerge.back()->clone();
                    if (volta->startMeasure() != remainder->startMeasure()) {
                        // First part is not empty
                        voltasToMerge.back()->setEndElement(volta->startMeasure()->prevMeasure());
                        remainder->setStartElement(volta->startMeasure());
                        // Store it
                        preProcessedVoltas.push_back(voltasToMerge.back());
                    }
                    //else { New volta and existing one start at the same moment, there is no first part, only a remainder }
                    voltasToMerge.pop_back();

                    // remainder and volta now have the same start point
                    // Compare the end points and make remainder end first
                    if (volta->endMeasure()->tick() < remainder->endMeasure()->tick()) {
                        Volta* swap = volta;
                        volta = remainder;
                        remainder = swap;
                    }
                    // Cross-section of the repeatList
                    std::list<int> endings(remainder->endings().begin(), remainder->endings().end());
                    endings.remove_if([&volta](const int& ending) {
                        return !(volta->hasEnding(ending));
                    });
                    remainder->setEndings(QList<int>(endings.begin(), endings.end()));
                    // Split and merge done
                    preProcessedVoltas.push_back(remainder);
                    if (volta->endMeasure() != remainder->endMeasure()) {
                        // volta extends past the end of remainder -> move its startpoint after remainder
                        volta->setStartElement(remainder->endMeasure()->nextMeasure());
                    } else {         // volta matched remainder endpoint, nothing left to merge from
                        preProcessedVoltas.splice(preProcessedVoltas.cend(), voltasToMerge);
                        delete volta;
                        volta = nullptr;
                    }
                } // !voltasToMerge.empty()

                if (volta != nullptr) {
                    preProcessedVoltas.push_back(volta);
                }
            }
        } // spanner->isVolta
    }

    volta = nullptr;
    for (; mb; mb = mb->next()) {
        if (mb->isMeasure()) {
            sectionEndMeasureBase = mb; // ending measure of section is the most recently encountered actual Measure

            // Volta ?
            if ((!preProcessedVoltas.empty()) && (preProcessedVoltas.front()->startMeasure() == mb)) {
                if (volta != nullptr) {
                    //if (volta->endMeasure()->tick() < mb->tick()) {
                    // The previous volta was supposed to end before us (open volta case) -> insert the end
                    sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::VOLTA_END, volta, toMeasure(
                                                                           mb->prevMeasure())));
                    //volta = nullptr; // No need, replaced immediately further down
                    //      }
                    //else { // Overlapping voltas; this should not happen as preProcessedVoltas should've dealt with this already }
                }
                // Now insert the start of the current volta
                volta = preProcessedVoltas.front();
                sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::VOLTA_START, volta, toMeasure(mb)));
                // Look for start of next volta
                preProcessedVoltas.pop_front();
            }
            // Start
            if (mb->repeatStart()) {
                if (volta != nullptr) {
                    if (volta->startMeasure() != toMeasure(mb)) {
                        // Volta and Start repeat are not on the same measure
                        // assume the previous volta was supposed to end before us (open volta case) -> insert the end
                        // Warning: This might "break" a volta prematurely if its explicit notated end is later than this point
                        //          Consider splitting the volta or ignoring this repeat all together
                        sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::VOLTA_END, volta,
                                                                           toMeasure(mb->prevMeasure())));
                        volta = nullptr;
                    }
                    //else { // Volta and Start Repeat coincide on the same measure, see test::repeat56.mscx }
                }
                startFromRepeatMeasure = new RepeatListElement(RepeatListElementType::REPEAT_START, mb, toMeasure(mb));
                sectionRLElements->push_back(startFromRepeatMeasure);
            }
            // Jumps and Markers
            for (Element* e : mb->el()) {
                if (e->isJump()) {
                    sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::JUMP, e, toMeasure(mb)));
                    if (volta != nullptr) {
                        if ((volta->endMeasure()->tick() < mb->tick())
                            || ((volta->endMeasure()->tick() == mb->tick())
                                && (volta->getProperty(Pid::END_HOOK_TYPE).value<HookType>() == HookType::NONE)
                                )
                            ) {
                            // The previous volta was supposed to end before us
                            // or open volta ends together with us -> insert the end
                            sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::VOLTA_END, volta, toMeasure(mb)));
                            volta = nullptr;
                        }
                        //else { // Volta is spanning past this jump instruction }
                    }
                } else if (e->isMarker()) {
                    RepeatListElement* markerRLE = new RepeatListElement(RepeatListElementType::MARKER, e, toMeasure(mb));
                    // There may be multiple markers in the same measure and there is no guarantee we're reading
                    // them from left to right. The only way available to guess their order is to look at their
                    // text alignment and order them left to right
                    // At the same time, we should ensure Markers are evaluated before Jumps
                    Align markerRLEalignmentH
                        = static_cast<Align>(static_cast<char>(toMarker(e)->align()) & static_cast<char>(Align::HMASK));
                    auto insertionIt = sectionRLElements->end() - 1;
                    while ((*insertionIt)->measure == markerRLE->measure) {
                        bool markerShouldGoBefore = false;
                        if (((*insertionIt)->repeatListElementType == RepeatListElementType::MARKER)
                            && (markerRLEalignmentH != Align::RIGHT) // We can be the end when right aligned
                            ) {
                            Align storedMarkerAlignmentH
                                = static_cast<Align>(static_cast<char>(toMarker((*insertionIt)->element)->align())
                                                     & static_cast<char>(Align::HMASK));
                            if (markerRLEalignmentH == Align::HCENTER) {
                                markerShouldGoBefore = (storedMarkerAlignmentH == Align::RIGHT);
                            } else { //(markerRLEalignmentH == Align::LEFT)
                                markerShouldGoBefore = (storedMarkerAlignmentH != Align::LEFT);
                            }
                        }
                        if (markerShouldGoBefore
                            || ((*insertionIt)->repeatListElementType == RepeatListElementType::JUMP)
                            ) {
                            // Decrease position
                            // This should always be possible as the list always starts with a REPEAT_START element
                            Q_ASSERT(insertionIt != sectionRLElements->begin());
                            --insertionIt;
                        } else {
                            // Found location after which we should go
                            break;
                        }
                    }
                    // We should be inserted right after insertionIt
                    sectionRLElements->insert(insertionIt + 1, markerRLE);
                }
            }
            // End
            if (mb->repeatEnd()) {
                sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::REPEAT_END, mb, toMeasure(mb)));
                if (startFromRepeatMeasure != nullptr) {
                    startFromRepeatMeasure->addToRepeatCount(toMeasure(mb)->repeatCount() - 1);
                }
                if (volta != nullptr) {
                    //if (volta->endMeasure()->tick() < mb->tick()) {
                    // The previous volta was supposed to end before us (open volta case) -> insert the end
                    sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::VOLTA_END, volta, toMeasure(mb)));
                    volta = nullptr;
                    //} else {
                    //    // Volta is spanning over this end repeat, consider splitting the volta
                    //    // or ignoring this repeat all together
                    //}
                }
            }
            // Volta end
            if ((volta != nullptr)
                && (volta->endMeasure()->tick() == mb->tick())
                && (volta->getProperty(Pid::END_HOOK_TYPE).value<HookType>() != HookType::NONE)) {
                // end of closed volta
                sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::VOLTA_END, volta, toMeasure(mb)));
                volta = nullptr;
            }
        }
        // Section break (or end of score)
        if (mb->sectionBreak() || !mb->nextMeasure()) {
            if (sectionEndMeasureBase != nullptr) {
                if (volta != nullptr) {
                    //if (volta->endMeasure()->tick() < mb->tick()) {
                    // The previous volta was supposed to end before us (open volta case) -> insert the end
                    sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::VOLTA_END, volta, toMeasure(mb)));
                    volta = nullptr;
                    //} else {
                    //    // Volta is spanning over this section break, consider splitting the volta
                    //    // and adding it again at the start of the next section
                    //}
                }
                sectionRLElements->push_back(new RepeatListElement(RepeatListElementType::SECTION_BREAK, mb,
                                                                   toMeasure(sectionEndMeasureBase)));
                sectionEndMeasureBase = nullptr; // reset to indicate not having found the end for the next section
                // store section
                _rlElements.push_back(sectionRLElements);
            }
            // prepare for new section
            if (mb->nextMeasure()) {
                sectionRLElements = new QList<RepeatListElement*>();
                // First measure of a section/score is always used as a reference REPEAT_START point
                // even if it doesn't have a start repeat
                startFromRepeatMeasure
                    = new RepeatListElement(RepeatListElementType::REPEAT_START, mb->nextMeasure(), toMeasure(mb->nextMeasure()));
                sectionRLElements->push_back(startFromRepeatMeasure);
                // Loop will forward one measureBase, so return one now
                // this logic aids in skipping multiple frames between sections
                mb = mb->nextMeasure()->prev();
            } else {
                // no more measures -> done
                break;
            }
        }
    }
}

///
/// \brief Finds a matching marker in the score (section first)
/// \remark Special case labels:
///         "start" will result in start of current section
///         "end" will result in end of current section
///
std::pair<QList<QList<RepeatListElement*>*>::const_iterator, QList<RepeatListElement*>::const_iterator> RepeatList::findMarker(
    QString label, QList<QList<RepeatListElement*>*>::const_iterator referenceSectionIt,
    QList<RepeatListElement*>::const_iterator referenceRepeatListElementIt) const
{
    bool found = false;
    QList<QList<RepeatListElement*>*>::const_iterator foundSectionIt;
    QList<RepeatListElement*>::const_iterator foundRepeatListElementIt;

    // Start in the current section
    foundSectionIt = referenceSectionIt;
    // Handle special label casing first
    if (label == "start") {
        foundRepeatListElementIt = (*referenceSectionIt)->cbegin();
        found = true;
    } else if (label == "end") {
        foundRepeatListElementIt = (*referenceSectionIt)->cend() - 1;
        found = true;
    } else {
        foundRepeatListElementIt = referenceRepeatListElementIt;
    }

    // Search backwards in this section
    while (!found && (foundRepeatListElementIt != (*referenceSectionIt)->cbegin())) {
        --foundRepeatListElementIt;
        if (((*foundRepeatListElementIt)->repeatListElementType == RepeatListElementType::MARKER)
            && ((toMarker((*foundRepeatListElementIt)->element))->label() == label)
            ) {
            found = true;
        }
    }

    // Search forwards in this section
    if (!found) {
        foundRepeatListElementIt = referenceRepeatListElementIt + 1;
        while (!found && (foundRepeatListElementIt != (*referenceSectionIt)->cend())) {
            if (((*foundRepeatListElementIt)->repeatListElementType == RepeatListElementType::MARKER)
                && ((toMarker((*foundRepeatListElementIt)->element))->label() == label)
                ) {
                found = true;
                break;
            }
            ++foundRepeatListElementIt;
        }
    }

    // Search backwards through all previous sections
    if (!found) {
        while (!found && (foundSectionIt != _rlElements.cbegin())) {
            --foundSectionIt;
            foundRepeatListElementIt = (*foundSectionIt)->cend();
            while (!found && (foundRepeatListElementIt != (*foundSectionIt)->cbegin())) {
                --foundRepeatListElementIt;
                if (((*foundRepeatListElementIt)->repeatListElementType == RepeatListElementType::MARKER)
                    && ((toMarker((*foundRepeatListElementIt)->element))->label() == label)
                    ) {
                    found = true;
                }
            }
        }
    }

    // Search forwards through all following sections
    if (!found) {
        foundSectionIt = referenceSectionIt + 1;
        while (foundSectionIt != _rlElements.cend()) {
            foundRepeatListElementIt = (*foundSectionIt)->cbegin();
            while (!found && (foundRepeatListElementIt != (*foundSectionIt)->cend())) {
                if (((*foundRepeatListElementIt)->repeatListElementType == RepeatListElementType::MARKER)
                    && ((toMarker((*foundRepeatListElementIt)->element))->label() == label)
                    ) {
                    found = true;
                    break;
                }
                ++foundRepeatListElementIt;
            }
            if (found) {
                break;
            }
            ++foundSectionIt;
        }
    }

    return std::make_pair(foundSectionIt, foundRepeatListElementIt);
}

///
/// \brief RepeatList::performJump
/// \param sectionIt                 [in]   Section of the jump target
/// \param repeatListElementTargetIt [in]   RepeatListElement of the jump target within the section
/// \param withRepeats               [in]   Whether first or last playtrough of the target is the actual target, influences playbackCount
/// \param playbackCount             [out]  Will contain the resulting playbackCount value
/// \param activeVolta               [out]  Contains a reference to the active Volta for jump target
/// \param startRepeatReference      [out]  Reference point to return to and compare against for jump target
///
void RepeatList::performJump(QList<QList<RepeatListElement*>*>::const_iterator sectionIt,
                             QList<RepeatListElement*>::const_iterator repeatListElementTargetIt,
                             bool withRepeats, int* const playbackCount,
                             Volta const** const activeVolta, RepeatListElement const** const startRepeatReference) const
{
    QList<RepeatListElement*>::const_iterator repeatListElementIt;
    // Fast forward processing up to our desired marker
    *activeVolta = nullptr;
    *startRepeatReference = *((*sectionIt)->cbegin());
    for (repeatListElementIt = (*sectionIt)->cbegin(); repeatListElementIt != repeatListElementTargetIt; ++repeatListElementIt) {
        switch ((*repeatListElementIt)->repeatListElementType) {
        case RepeatListElementType::VOLTA_START: {
            *activeVolta = toVolta((*repeatListElementIt)->element);
        } break;
        case RepeatListElementType::VOLTA_END: {
            *activeVolta = nullptr;
        } break;
        case RepeatListElementType::REPEAT_START: {
            *startRepeatReference = *repeatListElementIt;
        } break;
        default: {
            // nothing to do
        } break;
        }
    }
    // Set the correct playbackCount
    if (withRepeats) {
        if (*activeVolta != nullptr) {
            *playbackCount = (*activeVolta)->firstEnding();
            if (*playbackCount == 0) {
                *playbackCount = 1;
            }
        } else {
            *playbackCount = 1;
        }
    } else { // Final repeat after jump
        if (*activeVolta != nullptr) {
            *playbackCount = (*activeVolta)->lastEnding();
            if (*playbackCount == 0) {
                *playbackCount = (*startRepeatReference)->getRepeatCount();
            }
        } else {
            *playbackCount = (*startRepeatReference)->getRepeatCount();
        }
    }
}

///
/// \brief RepeatList::unwind
///
void RepeatList::unwind()
{
    qDeleteAll(*this);
    clear();
    _jumpsTaken.clear();

    if (!_score->firstMeasure()) {
        return;
    }

    collectRepeatListElements();

    // Following variables are used during unwinding, but may be altered when following jumps
    // Therefor they are declared outside of the loop
    RepeatSegment* rs = nullptr;
    int playbackCount;
    Volta const* activeVolta = nullptr;
    std::pair<QList<QList<RepeatListElement*>*>::const_iterator, QList<RepeatListElement*>::const_iterator> playUntil = std::make_pair(
        _rlElements.cend(), _rlElements[0]->cend());
    std::pair<QList<QList<RepeatListElement*>*>::const_iterator, QList<RepeatListElement*>::const_iterator> continueAt = std::make_pair(
        _rlElements.cend(), _rlElements[0]->cend());
    Jump const* activeJump = nullptr;
    bool forceFinalRepeat = false;   // Used during jump processing
    QList<RepeatListElement*>::const_iterator repeatListElementIt;

    for (QList<QList<RepeatListElement*>*>::const_iterator sectionIt = _rlElements.cbegin(); sectionIt != _rlElements.cend(); ++sectionIt) {
        // Unwind this section
        RepeatListElement const* startRepeatReference;
        playbackCount = 1;
        repeatListElementIt = (*sectionIt)->cbegin();     // Should always be a REPEAT_START indicator
        startRepeatReference = *repeatListElementIt;
        activeVolta = nullptr;
        playUntil.first = _rlElements.cend();
        continueAt.first = _rlElements.cend();
        forceFinalRepeat = false;

        rs = new RepeatSegment(playbackCount);
        rs->addMeasure((*repeatListElementIt)->measure);
        ++repeatListElementIt;

        while (repeatListElementIt != (*sectionIt)->cend()) {
            if (rs) {
                rs->addMeasures((*repeatListElementIt)->measure);
            }
            switch ((*repeatListElementIt)->repeatListElementType) {
            case RepeatListElementType::SECTION_BREAK: {
                if ((rs != nullptr) && (!rs->isEmpty())) {
                    push_back(rs);
                }
            } break;
            case RepeatListElementType::VOLTA_START: {
                activeVolta = toVolta((*repeatListElementIt)->element);
                if (!(activeVolta->hasEnding(playbackCount))) {
                    // Should be skipped, remove our measure from rs
                    rs->popMeasure();
                    if (!rs->isEmpty()) {
                        push_back(rs);
                    }
                    // Skip the volta
                    do {
                        ++repeatListElementIt;
                    } while ((*repeatListElementIt)->repeatListElementType != RepeatListElementType::VOLTA_END);
                    activeVolta = nullptr;
                    // Start next rs on the following measure
                    Measure const* const possibleNextMeasure = (*repeatListElementIt)->measure->nextMeasure();
                    if (possibleNextMeasure == nullptr) {
                        rs = nullptr;                   // end of score, but will still encounter section break, notify it
                    } else {
                        rs = new RepeatSegment(playbackCount);
                        rs->addMeasure(possibleNextMeasure);
                    }
                }
                // else { take the volta, it's already set as activeVolta, so just continue }
            } break;
            case RepeatListElementType::VOLTA_END: {
                activeVolta = nullptr;
            } break;
            case RepeatListElementType::REPEAT_START: {
                if (rs == nullptr) {               // Sent here by an end-repeat
                    rs = new RepeatSegment(playbackCount);
                    rs->addMeasure((*repeatListElementIt)->measure);
                } else {
                    int desiredPlaybackCount = (forceFinalRepeat) ? (*repeatListElementIt)->getRepeatCount() : 1;
                    if (rs->playbackCount != desiredPlaybackCount) {
                        // Restart a segment as we have a new playbackCount reference
                        rs->popMeasure();
                        if (!rs->isEmpty()) {
                            push_back(rs);
                        }
                        playbackCount = desiredPlaybackCount;
                        rs = new RepeatSegment(playbackCount);
                        rs->addMeasure((*repeatListElementIt)->measure);
                    }
                    startRepeatReference = *repeatListElementIt;
                }
            } break;
            case RepeatListElementType::REPEAT_END: {
                (*repeatListElementIt)->addToRepeatCount(1);
                if ((playbackCount < startRepeatReference->getRepeatCount())
                    && ((*repeatListElementIt)->getRepeatCount() < (*repeatListElementIt)->measure->repeatCount())
                    ) {
                    // Honor the repeat
                    push_back(rs);
                    rs = nullptr;
                    do {                 // rewind
                        --repeatListElementIt;
                        if ((*repeatListElementIt)->repeatListElementType == RepeatListElementType::VOLTA_START) {
                            activeVolta = nullptr;
                        } else if ((*repeatListElementIt)->repeatListElementType == RepeatListElementType::VOLTA_END) {
                            activeVolta = toVolta((*repeatListElementIt)->element);
                        }
                    } while ((*repeatListElementIt) != startRepeatReference);
                    ++playbackCount;
                    continue;                 // Force evaluation of the start repeat
                }
                //else { // Exhausted repeats, just continue }
            } break;
            case RepeatListElementType::JUMP: {
                if (activeJump == toJump((*repeatListElementIt)->element)) {
                    // We've reached this jump again after just having taken it
                    // If this jump forced final repeat playthrough of measures
                    // it only forces so up to this point - see test::repeat23
                    forceFinalRepeat = false;
                }
                // TODO future improvement, use the repeatList property from the Jump to verify if it should be honored
                // For now (or in case of an empty repeatList property) a Jump is only honored on final playthrough
                // of the measure, and only taken once
                if ((playbackCount >= startRepeatReference->getRepeatCount())
                    || ((activeVolta != nullptr) && (playbackCount == activeVolta->lastEnding()))
                    ) {
                    std::pair<Jump const* const,
                              int> jumpOccurence = std::make_pair(toJump((*repeatListElementIt)->element), playbackCount);
                    if (_jumpsTaken.find(jumpOccurence) == _jumpsTaken.end()) {                 // Not yet processed
                        // Processing it now
                        _jumpsTaken.insert(jumpOccurence);
                        // Find the jump targets
                        std::pair<QList<QList<RepeatListElement*>*>::const_iterator,
                                  QList<RepeatListElement*>::const_iterator> jumpTo = findMarker(
                            jumpOccurence.first->jumpTo(), sectionIt, repeatListElementIt);
                        playUntil = findMarker(jumpOccurence.first->playUntil(), sectionIt, repeatListElementIt);
                        continueAt = findMarker(jumpOccurence.first->continueAt(), sectionIt, repeatListElementIt);

                        // Execute
                        if (jumpTo.first != _rlElements.cend()) {
                            push_back(rs);
                            rs = nullptr;

                            activeJump = jumpOccurence.first;
                            performJump(jumpTo.first, jumpTo.second,
                                        activeJump->playRepeats(), &playbackCount, &activeVolta, &startRepeatReference);
                            sectionIt = jumpTo.first;
                            repeatListElementIt = jumpTo.second;
                            forceFinalRepeat = !(activeJump->playRepeats());

                            // Re-evaluate our repeat count for end repeat measures
                            if (playUntil.first != _rlElements.cend()) {                     // Only required if we have an end target
                                for (auto rleIt = repeatListElementIt + 1; rleIt != (*sectionIt)->cend(); ++rleIt) {
                                    if (((*rleIt)->repeatListElementType == RepeatListElementType::REPEAT_END)
                                        && ((*rleIt)->getRepeatCount() != 0)
                                        ) {                        // We've been played before - see test::repeat22 for why only then
                                        // Clear out our current repeat count
                                        (*rleIt)->addToRepeatCount(-((*rleIt)->getRepeatCount()));
                                        if (forceFinalRepeat) {
                                            // Set repeat count to the maximum number for this repeat
                                            (*rleIt)->addToRepeatCount((*rleIt)->measure->repeatCount());
                                        }
                                    }
                                }
                            }
                            if (activeVolta != nullptr) {
                                // We jumped into a volta, which might not be the only one of this startRepeatReference
                                // If our jump didn't take us to the last playthrough, we must re-evaluate
                                // all voltas related to this startRepeatReference
                                if (playbackCount < startRepeatReference->getRepeatCount()) {
                                    // test::repeat52 and test::repeat53
                                    QList<RepeatListElement*>::const_iterator findRepeatIt = repeatListElementIt;
                                    do {
                                        --findRepeatIt;
                                    } while ((*findRepeatIt) != startRepeatReference);
                                    ++findRepeatIt;                         // Start volta analysis past this start repeat
                                    Volta const* voltaReference = nullptr;
                                    int processedRepeatCount = 1;
                                    while ((findRepeatIt != (*sectionIt)->cend())
                                           && ((*findRepeatIt)->repeatListElementType != RepeatListElementType::REPEAT_START)
                                           && (processedRepeatCount < startRepeatReference->getRepeatCount())
                                           ) {
                                        if ((*findRepeatIt)->repeatListElementType == RepeatListElementType::VOLTA_START) {
                                            voltaReference = toVolta((*findRepeatIt)->element);
                                            if (voltaReference->lastEnding() < playbackCount) {
                                                // We won't pass here anymore - no need to adjust end repeat to this volta
                                                voltaReference = nullptr;
                                            }
                                        } else if ((*findRepeatIt)->repeatListElementType == RepeatListElementType::REPEAT_END) {
                                            if (voltaReference != nullptr) {
                                                int remainingRepeatsCount = 0;
                                                for (int ending : voltaReference->endings()) {
                                                    if (ending >= playbackCount) {
                                                        ++remainingRepeatsCount;
                                                    }
                                                }
                                                // Apply new remainingRepeatsCount
                                                (*findRepeatIt)->addToRepeatCount(-((*findRepeatIt)->getRepeatCount()));
                                                (*findRepeatIt)->addToRepeatCount(
                                                    (*findRepeatIt)->measure->repeatCount() - remainingRepeatsCount - 1);
                                            }
                                            processedRepeatCount += (*findRepeatIt)->measure->repeatCount() - 1;
                                        }
                                        ++findRepeatIt;
                                    }
                                }
                            }

                            // We've arrived at the target marker
                            rs = new RepeatSegment(playbackCount);
                            rs->addMeasure((*repeatListElementIt)->measure);
                        }                         // End of jump execution
                    }
                }
            } break;
            case RepeatListElementType::MARKER: {
                if (((sectionIt == playUntil.first) && (repeatListElementIt == playUntil.second))
                    && ((playbackCount == startRepeatReference->getRepeatCount())
                        || ((activeVolta != nullptr) && (playbackCount == activeVolta->lastEnding()))
                        )
                    ) {               // Found final playThrough of this Marker
                    push_back(rs);
                    rs = nullptr;
                    playUntil.first = _rlElements.cend();                 // Clear this reference - processed
                    forceFinalRepeat = false;

                    if (continueAt.first != _rlElements.cend()) {
                        performJump(continueAt.first, continueAt.second, true, &playbackCount, &activeVolta, &startRepeatReference);
                        sectionIt = continueAt.first;
                        repeatListElementIt = continueAt.second;
                        if (startRepeatReference->measure != (*(continueAt.second))->measure) {
                            // Use the continueAt target as repeat reference if it wasn't this measure already
                            // Check for measure is required as REPEAT_START is processed before a marker
                            // see test::repeat23 m12
                            startRepeatReference = *(continueAt.second);
                        }

                        // We've arrived at the target marker
                        rs = new RepeatSegment(playbackCount);
                        rs->addMeasure((*repeatListElementIt)->measure);

                        continueAt.first = _rlElements.cend();                   // Clear this reference - processed
                    } else { // Nowhere to go to, break out of this section loop and onto the next section
                        repeatListElementIt = (*sectionIt)->cend();
                        continue;
                    }
                }
            } break;
            }
            ++repeatListElementIt;
        }

        // Reached the end of this section
        // Inform the last RepeatSegment that the Section Break pause property should be honored now
        rs = this->back();
        repeatListElementIt = (*sectionIt)->cend() - 1;
        Q_ASSERT((*repeatListElementIt)->repeatListElementType == RepeatListElementType::SECTION_BREAK);

        LayoutBreak const* const sectionBreak = toMeasureBase((*repeatListElementIt)->element)->sectionBreakElement();
        if (sectionBreak != nullptr) {
            rs->pause = sectionBreak->pause();
        }
    }

    updateTempo();
    _expanded = true;
}
}
