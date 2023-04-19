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

#ifndef MU_ENGRAVING_EXCERPT_H
#define MU_ENGRAVING_EXCERPT_H

#include <map>

#include "types/fraction.h"
#include "types/types.h"
#include "types/string.h"

#include "async/notification.h"

namespace mu::engraving {
class MasterScore;
class Part;
class Score;
class Staff;
class Spanner;
class XmlReader;

class Excerpt
{
public:
    Excerpt(MasterScore* masterScore = nullptr) { m_masterScore = masterScore; }
    Excerpt(const Excerpt& ex, bool copyContents = true);

    ~Excerpt();

    bool inited() const;

    bool custom() const;
    void markAsCustom();

    const ID& initialPartId() const;
    void setInitialPartId(const ID& id);

    MasterScore* masterScore() const { return m_masterScore; }
    Score* excerptScore() const { return m_excerptScore; }
    void setExcerptScore(Score* s);

    const String& name() const;
    void setName(const String& name);
    async::Notification nameChanged() const;

    std::vector<Part*>& parts() { return m_parts; }
    const std::vector<Part*>& parts() const { return m_parts; }
    void setParts(const std::vector<Part*>& parts) { m_parts = parts; }

    bool containsPart(const Part* part) const;

    void removePart(const ID& id);

    size_t nstaves() const;
    bool isEmpty() const;

    const TracksMap& tracksMapping();
    void setTracksMapping(const TracksMap& tracksMapping);

    void setVoiceVisible(Staff* staff, int voiceIndex, bool visible);

    static std::vector<Excerpt*> createExcerptsFromParts(const std::vector<Part*>& parts);
    static Excerpt* createExcerptFromPart(Part* part);

    static void createExcerpt(Excerpt*);
    static void cloneStaves(Score* sourceScore, Score* dstScore, const std::vector<staff_idx_t>& sourceStavesIndexes,
                            const TracksMap& allTracks);
    static void cloneMeasures(Score* oscore, Score* score);
    static void cloneStaff(Staff* ostaff, Staff* nstaff, bool cloneSpanners = true);
    static void cloneStaff2(Staff* ostaff, Staff* nstaff, const Fraction& startTick, const Fraction& endTick);
    static void cloneSpanner(Spanner* s, Score* score, track_idx_t dstTrack, track_idx_t dstTrack2);

    static String formatName(const String& partName, const std::vector<Excerpt*>& allExcerpts);

private:
    friend class MasterScore;

    void setInited(bool inited);
    void writeNameToMetaTags();

    void updateTracksMapping();

    MasterScore* m_masterScore = nullptr;
    Score* m_excerptScore = nullptr;
    String m_name;
    async::Notification m_nameChanged;
    std::vector<Part*> m_parts;
    TracksMap m_tracksMapping;
    bool m_inited = false;
    ID m_initialPartId;
};
}

#endif // MU_ENGRAVING_EXCERPT_H
