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
#include "libmscore/masterscore.h"
#include "libmscore/score.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/linkedobjects.h"
#include "libmscore/measure.h"
#include "libmscore/factory.h"
#include "libmscore/ornament.h"
#include "libmscore/stafftextbase.h"
#include "libmscore/playtechannotation.h"

#include "rw/xmlreader.h"
#include "rw/400/readcontext.h"
#include "rw/400/tread.h"

#include "types/string.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::compat;

void CompatUtils::doCompatibilityConversions(MasterScore* masterScore)
{
    if (!masterScore) {
        return;
    }

    // TODO: collect all compatibility conversions here
    if (masterScore->mscVersion() <= 400) {
        replaceOldWithNewOrnaments(masterScore);
    }
}

void CompatUtils::replaceStaffTextWithPlayTechniqueAnnotation(Score* score)
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

                PlayTechAnnotation* playTechAnnotation = Factory::createPlayTechAnnotation(segment, type, text->textStyleType());
                playTechAnnotation->setXmlText(text->xmlText());
                playTechAnnotation->setTrack(annotation->track());

                segment->removeAnnotation(annotation);
                segment->add(playTechAnnotation);

                delete annotation;
            }
        }
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

            if (excerpt->name().contains(part->partName())) {
                assignInitialPartId(excerpt, part->id());
                break;
            }
        }
    }
}

void CompatUtils::replaceOldWithNewOrnaments(MasterScore* score)
{
    static const std::set<SymId> ornamentIds {
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

    std::vector<Articulation*> oldOrnaments;     // ornaments used to be articulations

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
                    if (ornamentIds.find(articulation->symId()) != ornamentIds.end()) {
                        oldOrnaments.push_back(articulation);
                        LinkedObjects* links = articulation->links();
                        if (!links || links->empty()) {
                            continue;
                        }
                        for (EngravingObject* linked : *links) {
                            if (linked != articulation) {
                                oldOrnaments.push_back(toArticulation(linked));
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
