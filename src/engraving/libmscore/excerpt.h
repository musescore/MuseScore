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
#include "mscore.h"

namespace mu::engraving {
class EngravingItem;
class MasterScore;
class Part;
class Score;
class Staff;
class XmlReader;

class Excerpt
{
public:
    Excerpt(MasterScore* masterScore = nullptr) { m_masterScore = masterScore; }
    Excerpt(const Excerpt& ex, bool copyPartScore = true);

    ~Excerpt();

    MasterScore* masterScore() const { return m_masterScore; }
    Score* excerptScore() const { return m_excerptScore; }
    void setExcerptScore(Score* s);

    String name() const { return m_name; }
    void setName(const QString& title) { m_name = title; }

    std::vector<Part*>& parts() { return m_parts; }
    const std::vector<Part*>& parts() const { return m_parts; }
    void setParts(const std::vector<Part*>& parts) { m_parts = parts; }

    bool containsPart(const Part* part) const;

    void removePart(const ID& id);

    size_t nstaves() const;
    bool isEmpty() const;

    TracksMap& tracksMapping() { return m_tracksMapping; }
    void setTracksMapping(const TracksMap& tracksMapping);

    void updateTracksMapping();

    void setVoiceVisible(Staff* staff, int voiceIndex, bool visible);

    void read(XmlReader&);

    bool operator==(const Excerpt& other) const;
    bool operator!=(const Excerpt& other) const;

    static std::vector<Excerpt*> createExcerptsFromParts(const std::vector<Part*>& parts);
    static Excerpt* createExcerptFromPart(Part* part);

    static void createExcerpt(Excerpt*);
    static void cloneStaves(Score* sourceScore, Score* dstScore, const std::vector<staff_idx_t>& sourceStavesIndexes,
                            const TracksMap& allTracks);
    static void cloneMeasures(Score* oscore, Score* score);
    static void cloneStaff(Staff* ostaff, Staff* nstaff);
    static void cloneStaff2(Staff* ostaff, Staff* nstaff, const Fraction& startTick, const Fraction& endTick);

private:
    static QString formatName(const QString& partName, const std::vector<Excerpt*>&);

    MasterScore* m_masterScore = nullptr;
    Score* m_excerptScore = nullptr;
    String m_name;
    std::vector<Part*> m_parts;
    TracksMap m_tracksMapping;
};
}

#endif // MU_ENGRAVING_EXCERPT_H
