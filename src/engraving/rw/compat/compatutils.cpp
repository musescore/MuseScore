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

#include "compatutils.h"

#include "libmscore/articulation.h"
#include "libmscore/chord.h"
#include "libmscore/dynamic.h"
#include "libmscore/expression.h"
#include "libmscore/masterscore.h"
#include "libmscore/score.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/stem.h"
#include "libmscore/tremolo.h"
#include "libmscore/linkedobjects.h"
#include "libmscore/measure.h"
#include "libmscore/factory.h"
#include "libmscore/ornament.h"
#include "libmscore/rest.h"
#include "libmscore/stafftext.h"
#include "libmscore/stafftextbase.h"
#include "libmscore/playtechannotation.h"
#include "libmscore/capo.h"

#include "types/string.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::compat;

const std::set<SymId> CompatUtils::ORNAMENT_IDS {
    SymId::ornamentTurn,
    SymId::ornamentTurnInverted,
    SymId::ornamentTurnSlash,
    SymId::ornamentTrill,
    SymId::brassMuteClosed,
    SymId::ornamentMordent,
    SymId::ornamentShortTrill,
    SymId::ornamentTremblement,
    SymId::ornamentPrallMordent,
    SymId::ornamentLinePrall,
    SymId::ornamentUpPrall,
    SymId::ornamentUpMordent,
    SymId::ornamentPrecompMordentUpperPrefix,
    SymId::ornamentDownMordent,
    SymId::ornamentPrallUp,
    SymId::ornamentPrallDown,
    SymId::ornamentPrecompSlide,
    SymId::ornamentShake3,
    SymId::ornamentShakeMuffat1,
    SymId::ornamentTremblementCouperin,
    SymId::ornamentPinceCouperin
};

void CompatUtils::doCompatibilityConversions(MasterScore* masterScore)
{
    TRACEFUNC;

    if (!masterScore) {
        return;
    }

    // TODO: collect all compatibility conversions here
    if (masterScore->mscVersion() < 400) {
        replaceStaffTextWithPlayTechniqueAnnotation(masterScore);
    }
    if (masterScore->mscVersion() < 410) {
        reconstructTypeOfCustomDynamics(masterScore);
        replaceOldWithNewExpressions(masterScore);
        replaceOldWithNewOrnaments(masterScore);
        resetRestVerticalOffset(masterScore);
        splitArticulations(masterScore);
        resetArticulationOffsets(masterScore);
        resetStemLengthsForTwoNoteTrems(masterScore);
        replaceStaffTextWithCapo(masterScore);
    }
}

void CompatUtils::replaceStaffTextWithPlayTechniqueAnnotation(MasterScore* score)
{
    TRACEFUNC;

    //! NOTE: Before MU4, they were available in the Staff Text Properties dialog (Change Channel page)
    static const std::unordered_map<mu::String, PlayingTechniqueType> textToPlayTechniqueType {
        { u"natural", PlayingTechniqueType::Natural },
        { u"normal", PlayingTechniqueType::Natural },

        { u"open", PlayingTechniqueType::Open },
        { u"mute", PlayingTechniqueType::Mute },

        { u"distortion", PlayingTechniqueType::Distortion },
        { u"overdriven", PlayingTechniqueType::Overdrive },
        { u"harmonics", PlayingTechniqueType::Harmonics },
        { u"jazz", PlayingTechniqueType::JazzTone },

        { u"arco", PlayingTechniqueType::Natural },
        { u"pizzicato", PlayingTechniqueType::Pizzicato },
        { u"tremolo", PlayingTechniqueType::Tremolo },
    };

    std::map<StaffTextBase*, PlayingTechniqueType> oldPlayTechniquesAndNewTypes;

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            std::vector<EngravingItem*> annotations = segment->annotations();

            for (EngravingItem* annotation : annotations) {
                if (!annotation || !annotation->isStaffTextBase()) {
                    continue;
                }

                StaffTextBase* text = toStaffTextBase(annotation);
                PlayingTechniqueType type
                    = mu::value(textToPlayTechniqueType, text->plainText().toLower(), PlayingTechniqueType::Undefined);

                if (type == PlayingTechniqueType::Undefined) {
                    mu::String channelName = text->channelName(0).toLower();

                    if (!channelName.isEmpty()) {
                        type = mu::value(textToPlayTechniqueType, channelName, PlayingTechniqueType::Undefined);
                    }
                }

                if (type == PlayingTechniqueType::Undefined) {
                    continue;
                }

                oldPlayTechniquesAndNewTypes.emplace(text, type);
                LinkedObjects* links = text->links();
                if (!links || links->empty()) {
                    continue;
                }
                for (EngravingObject* linked : *links) {
                    if (linked != text) {
                        oldPlayTechniquesAndNewTypes.emplace(toStaffTextBase(linked), type);
                    }
                }
            }
        }
    }

    for (auto& pair : oldPlayTechniquesAndNewTypes) {
        StaffTextBase* oldPlayTech = pair.first;
        PlayingTechniqueType type = pair.second;
        Segment* parentSegment = oldPlayTech->segment();

        PlayTechAnnotation* newPlayTech = Factory::createPlayTechAnnotation(parentSegment, type, oldPlayTech->textStyleType());
        newPlayTech->setXmlText(oldPlayTech->xmlText());
        newPlayTech->setTrack(oldPlayTech->track());

        LinkedObjects* links = oldPlayTech->links();
        newPlayTech->setLinks(links);
        if (links) {
            links->push_back(newPlayTech);
        }
        parentSegment->add(newPlayTech);
        parentSegment->removeAnnotation(oldPlayTech);

        delete oldPlayTech;
    }
}

void CompatUtils::assignInitialPartToExcerpts(const std::vector<Excerpt*>& excerpts)
{
    TRACEFUNC;

    std::set<ID> assignedPartIdSet;

    auto assignInitialPartId = [&assignedPartIdSet](Excerpt* excerpt, const ID& initialPartId) {
        excerpt->setInitialPartId(initialPartId);
        assignedPartIdSet.insert(initialPartId);
    };

    for (Excerpt* excerpt : excerpts) {
        for (const Part* part : excerpt->excerptScore()->parts()) {
            if (excerpt->name() == part->partName()) {
                assignInitialPartId(excerpt, part->id());
                break;
            }
        }
    }

    for (Excerpt* excerpt : excerpts) {
        if (excerpt->initialPartId().isValid()) {
            continue;
        }

        for (Part* part : excerpt->excerptScore()->parts()) {
            if (mu::contains(assignedPartIdSet, part->id())) {
                continue;
            }

            if (excerpt->name().contains(part->partName()) || excerpt->excerptScore()->parts().size() == 1) {
                assignInitialPartId(excerpt, part->id());
                break;
            }
        }
    }
}

void CompatUtils::replaceOldWithNewOrnaments(MasterScore* score)
{
    std::set<Articulation*> oldOrnaments;     // ornaments used to be articulations

    for (Measure* meas = score->firstMeasure(); meas; meas = meas->nextMeasure()) {
        for (Segment& seg : meas->segments()) {
            if (!seg.isChordRestType()) {
                continue;
            }
            for (EngravingItem* item : seg.elist()) {
                if (!item || !item->isChord()) {
                    continue;
                }
                for (Articulation* articulation : toChord(item)->articulations()) {
                    if (articulation->isOrnament()) {
                        continue;
                    }
                    if (ORNAMENT_IDS.find(articulation->symId()) != ORNAMENT_IDS.end()) {
                        oldOrnaments.insert(articulation);
                        LinkedObjects* links = articulation->links();
                        if (!links || links->empty()) {
                            continue;
                        }
                        for (EngravingObject* linked : *links) {
                            if (linked != articulation) {
                                oldOrnaments.insert(toArticulation(linked));
                            }
                        }
                    }
                }
            }
        }
    }

    for (Articulation* oldOrnament : oldOrnaments) {
        Chord* parentChord = toChord(oldOrnament->parentItem());

        Ornament* newOrnament = Factory::createOrnament(score->dummy()->chord());
        newOrnament->setParent(parentChord);
        newOrnament->setTrack(oldOrnament->track());
        newOrnament->setSymId(oldOrnament->symId());
        newOrnament->setPos(oldOrnament->pos());
        newOrnament->setOrnamentStyle(oldOrnament->ornamentStyle());
        newOrnament->setDirection(oldOrnament->direction());

        LinkedObjects* links = oldOrnament->links();
        newOrnament->setLinks(links);
        if (links) {
            links->push_back(newOrnament);
        }
        parentChord->add(newOrnament);
        parentChord->remove(oldOrnament);
        delete oldOrnament;
    }
}

void CompatUtils::replaceOldWithNewExpressions(MasterScore* score)
{
    std::set<StaffText*> oldExpressions; // Expressions used to be staff text

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment& seg : measure->segments()) {
            for (EngravingItem* item : seg.annotations()) {
                if (item && item->isStaffText() && toStaffText(item)->textStyleType() == TextStyleType::EXPRESSION) {
                    oldExpressions.insert(toStaffText(item));
                    LinkedObjects* links = item->links();
                    if (!links || links->empty()) {
                        continue;
                    }
                    for (EngravingObject* linkedItem : *links) {
                        if (linkedItem != item) {
                            oldExpressions.insert(toStaffText(linkedItem));
                        }
                    }
                }
            }
        }
    }

    for (StaffText* oldExpression : oldExpressions) {
        Segment* parentSegment = toSegment(oldExpression->parentItem(true));

        Expression* newExpression = Factory::createExpression(score->dummy()->segment());
        newExpression->setParent(parentSegment);
        newExpression->setTrack(oldExpression->track());
        newExpression->setXmlText(oldExpression->xmlText());
        newExpression->mapPropertiesFromOldExpressions(oldExpression);

        LinkedObjects* links = oldExpression->links();
        newExpression->setLinks(links);
        if (links) {
            links->push_back(newExpression);
        }

        parentSegment->add(newExpression);
        parentSegment->removeAnnotation(oldExpression);
        delete oldExpression;
    }
}

void CompatUtils::reconstructTypeOfCustomDynamics(MasterScore* score)
{
    // Before version 4.1, Dynamics containing custom text were saved as type "other"
    // We check the string to see if we can reconstruct their true type.
    std::set<Dynamic*> otherTypeDynamic;

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment& seg : measure->segments()) {
            for (EngravingItem* item : seg.annotations()) {
                if (item && item->isDynamic() && toDynamic(item)->dynamicType() == DynamicType::OTHER) {
                    otherTypeDynamic.insert(toDynamic(item));
                    LinkedObjects* links = item->links();
                    if (!links || links->empty()) {
                        continue;
                    }
                    for (EngravingObject* linkedItem : *links) {
                        if (linkedItem != item) {
                            otherTypeDynamic.insert(toDynamic(linkedItem));
                        }
                    }
                }
            }
        }
    }

    for (Dynamic* dynamic : otherTypeDynamic) {
        DynamicType dynType = reconstructDynamicTypeFromString(dynamic);
        dynamic->setDynamicType(dynType);
    }
}

void CompatUtils::splitArticulations(MasterScore* masterScore)
{
    std::set<Articulation*> toRemove;
    for (Measure* meas = masterScore->firstMeasure(); meas; meas = meas->nextMeasure()) {
        for (Segment& seg : meas->segments()) {
            if (!seg.isChordRestType()) {
                continue;
            }
            for (EngravingItem* item : seg.elist()) {
                if (!item || !item->isChord()) {
                    continue;
                }
                Chord* chord = toChord(item);
                for (Articulation* a : chord->articulations()) {
                    if (a->isLinked()) {
                        continue; // only worry about main artics, links will be done later
                    }
                    std::set<SymId> ids = mu::engraving::splitArticulations({ a->symId() });
                    if (ids.size() <= 1) {
                        continue;
                    }
                    toRemove.insert(a);
                }
            }
        }
    }
    // separate into individual articulations
    for (Articulation* combinedArtic : toRemove) {
        auto components = mu::engraving::splitArticulations({ combinedArtic->symId() });
        Chord* parentChord = toChord(combinedArtic->parentItem());
        for (SymId id : components) {
            Articulation* newArtic = Factory::createArticulation(masterScore->dummy()->chord());
            newArtic->setSymId(id);
            if (parentChord->hasArticulation(newArtic)) {
                delete newArtic;
                continue;
            }
            newArtic->setParent(parentChord);
            newArtic->setTrack(combinedArtic->track());
            newArtic->setPos(combinedArtic->pos());
            newArtic->setDirection(combinedArtic->direction());
            newArtic->setAnchor(combinedArtic->anchor());
            newArtic->setColor(combinedArtic->color());
            newArtic->setPlayArticulation(combinedArtic->playArticulation());
            newArtic->setVisible(combinedArtic->visible());
            newArtic->setOrnamentStyle(combinedArtic->ornamentStyle());
            LinkedObjects* links = new LinkedObjects(masterScore);
            links->push_back(newArtic);
            newArtic->setLinks(links);
            parentChord->add(newArtic);

            // newArtic is the main articulation
            LinkedObjects* oldLinks = combinedArtic->links();
            if (!oldLinks || oldLinks->empty()) {
                continue;
            }
            for (EngravingObject* linkedItem : *oldLinks) {
                IF_ASSERT_FAILED(linkedItem && linkedItem->isArticulation()) {
                    continue;
                }
                if (linkedItem == combinedArtic) {
                    continue;
                }
                Articulation* oldArtic = toArticulation(linkedItem);
                Chord* oldParent = toChord(oldArtic->parentItem());
                oldParent->add(newArtic->linkedClone());
            }
        }
    }
    // finally, remove the combined articulations
    for (Articulation* combinedArtic : toRemove) {
        LinkedObjects* links = combinedArtic->links();
        if (!links || links->empty()) {
            Chord* parentChord = toChord(combinedArtic->parentItem());
            parentChord->remove(combinedArtic);
            delete combinedArtic;
            continue;
        }
        std::set<Articulation*> removeLinks;
        for (auto linked : *links) {
            IF_ASSERT_FAILED(linked && linked->isArticulation()) {
                continue;
            }
            removeLinks.insert(toArticulation(linked));
        }
        for (Articulation* linkedArtic : removeLinks) {
            if (linkedArtic != combinedArtic) {
                Chord* linkedParent = toChord(linkedArtic->parentItem());
                linkedParent->remove(linkedArtic);
                delete linkedArtic;
            }
        }
        Chord* parentChord = toChord(combinedArtic->parentItem());
        parentChord->remove(combinedArtic);
        delete combinedArtic;
    }
}

DynamicType CompatUtils::reconstructDynamicTypeFromString(Dynamic* dynamic)
{
    static std::vector<Dyn> sortedDynList; // copy of dynList sorted by string length

    if (sortedDynList.empty()) {
        sortedDynList = Dynamic::dynamicList();
        std::sort(sortedDynList.begin(), sortedDynList.end(), [](const Dyn& a, const Dyn& b) {
            String stringA = String::fromUtf8(a.text);
            String stringB = String::fromUtf8(b.text);
            return stringA.size() > stringB.size();
        });
    }

    for (Dyn dyn : sortedDynList) {
        String dynText = String::fromUtf8(dyn.text);
        if (dynText.size() == 0) {
            return DynamicType::OTHER;
        }
        if (dynamic->xmlText().contains(dynText)) {
            return dyn.type;
        }
    }
    return DynamicType::OTHER;
}

ArticulationAnchor CompatUtils::translateToNewArticulationAnchor(int anchor)
{
    switch (anchor) {
    case 0: // ArticulationAnchor::TOP_STAFF
    case 3: // ArticulationAnchor::TOP_CHORD
        return ArticulationAnchor::TOP;
        break;
    case 1: // ArticulationAnchor::BOTTOM_STAFF
    case 4: // ArticulationAnchor::BOTTOM_CHORD
        return ArticulationAnchor::BOTTOM;
        break;
    case 2: // ArticulationAnchor::CHORD
    default:
        return ArticulationAnchor::AUTO;
        break;
    }
}

void CompatUtils::resetRestVerticalOffset(MasterScore* masterScore)
{
    for (Score* score : masterScore->scoreList()) {
        for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            for (Segment& segment : measure->segments()) {
                if (!segment.isChordRestType()) {
                    continue;
                }
                for (EngravingItem* item : segment.elist()) {
                    if (!item || !item->isRest()) {
                        continue;
                    }
                    Rest* rest = toRest(item);
                    if (rest->offset().y() != 0) {
                        PointF newOffset = PointF(rest->offset().x(), 0.0);
                        rest->setProperty(Pid::OFFSET, newOffset);
                    }
                }
            }
        }
    }
}

void CompatUtils::resetArticulationOffsets(MasterScore* masterScore)
{
    for (Score* score : masterScore->scoreList()) {
        for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            for (Segment& segment : measure->segments()) {
                if (!segment.isChordRestType()) {
                    continue;
                }
                for (EngravingItem* item : segment.elist()) {
                    if (!item || !item->isChord()) {
                        continue;
                    }
                    Chord* chord = toChord(item);
                    for (Articulation* artic : chord->articulations()) {
                        if (!artic) {
                            continue;
                        }
                        artic->setProperty(Pid::OFFSET, PointF());
                    }
                }
            }
        }
    }
}

void CompatUtils::resetStemLengthsForTwoNoteTrems(MasterScore* masterScore)
{
    for (Score* score : masterScore->scoreList()) {
        for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            for (Segment& segment : measure->segments()) {
                if (!segment.isChordRestType()) {
                    continue;
                }
                for (EngravingItem* item : segment.elist()) {
                    if (!item || !item->isChord()) {
                        continue;
                    }
                    Chord* chord = toChord(item);
                    Tremolo* trem = chord->tremolo();
                    Stem* stem = chord->stem();
                    if (stem && trem && trem->twoNotes()) {
                        if (stem->userLength() != Millimetre(0)) {
                            stem->setUserLength(Millimetre(0));
                        }
                    }
                }
            }
        }
    }
}

void CompatUtils::replaceStaffTextWithCapo(MasterScore* score)
{
    TRACEFUNC;

    std::set<StaffTextBase*> oldCapoSet;

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            for (EngravingItem* annotation : segment->annotations()) {
                if (!annotation || !annotation->isStaffTextBase()) {
                    continue;
                }

                StaffTextBase* text = toStaffTextBase(annotation);

                if (text->capo() > 0) {
                    oldCapoSet.insert(text);
                } else {
                    continue;
                }

                LinkedObjects* links = text->links();
                if (!links || links->empty()) {
                    continue;
                }

                for (EngravingObject* linked : *links) {
                    if (linked != text && linked && linked->isStaffTextBase()) {
                        oldCapoSet.insert(toStaffTextBase(linked));
                    }
                }
            }
        }
    }

    for (StaffTextBase* oldCapo : oldCapoSet) {
        Segment* parentSegment = oldCapo->segment();
        Capo* newCapo = Factory::createCapo(parentSegment);

        int capoFretPosition = oldCapo->capo() - 1;

        CapoParams params;
        params.active = capoFretPosition > 0;
        params.fretPosition = capoFretPosition;

        newCapo->setTrack(oldCapo->track());
        newCapo->setParams(params);
        newCapo->setProperty(Pid::PLACEMENT, oldCapo->placement());

        LinkedObjects* links = oldCapo->links();
        newCapo->setLinks(links);
        if (links) {
            links->push_back(newCapo);
        }

        parentSegment->add(newCapo);
        parentSegment->removeAnnotation(oldCapo);

        delete oldCapo;
    }
}
