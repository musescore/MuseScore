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

#include <QMultiMap>

#include "types/fraction.h"
#include "mscore.h"

namespace Ms {
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

    QString name() const { return m_name; }
    void setName(const QString& title) { m_name = title; }

    QList<Part*>& parts() { return m_parts; }
    const QList<Part*>& parts() const { return m_parts; }
    void setParts(const QList<Part*>& parts) { m_parts = parts; }

    bool containsPart(const Part* part) const;

    void removePart(const ID& id);

    int nstaves() const;
    bool isEmpty() const;

    QMultiMap<int, int>& tracksMapping() { return m_tracksMapping; }
    void setTracksMapping(const QMultiMap<int, int>& tracksMapping);

    void updateTracksMapping();

    void setVoiceVisible(Staff* staff, int voiceIndex, bool visible);

    void read(XmlReader&);

    bool operator==(const Excerpt& other) const;
    bool operator!=(const Excerpt& other) const;

    static QList<Excerpt*> createExcerptsFromParts(const QList<Part*>& parts);
    static Excerpt* createExcerptFromPart(Part* part);

    static void createExcerpt(Excerpt*);
    static void cloneStaves(Score* sourceScore, Score* destinationScore, const QList<int>& sourceStavesIndexes, const QMultiMap<int,
                                                                                                                                int>& allTracks);
    static void cloneMeasures(Score* oscore, Score* score);
    static void cloneStaff(Staff* ostaff, Staff* nstaff);
    static void cloneStaff2(Staff* ostaff, Staff* nstaff, const Fraction& startTick, const Fraction& endTick);

private:
    static QString formatName(const QString& partName, const QList<Excerpt*>&);

    MasterScore* m_masterScore = nullptr;
    Score* m_excerptScore = nullptr;
    QString m_name;
    QList<Part*> m_parts;
    QMultiMap<int, int> m_tracksMapping;
};
}

#endif // MU_ENGRAVING_EXCERPT_H
