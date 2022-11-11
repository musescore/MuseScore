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

#include "libmscore/score.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/factory.h"
#include "libmscore/stafftextbase.h"
#include "libmscore/playtechannotation.h"

#include "types/string.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::compat;

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
