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

#ifndef __EXCERPT_H__
#define __EXCERPT_H__

#include <QMultiMap>

#include "fraction.h"

namespace Ms {
class MasterScore;
class Score;
class Part;
class Measure;
class XmlWriter;
class Staff;
class XmlReader;
class Element;

//---------------------------------------------------------
//   @@ Excerpt
//---------------------------------------------------------

class Excerpt : public QObject
{
    MasterScore* _oscore;

    Score* _partScore           { 0 };
    QString _title;
    QList<Part*> _parts;
    QMultiMap<int, int> _tracks;

public:
    Excerpt(MasterScore* s = 0) { _oscore = s; }
    Excerpt(const Excerpt& ex, bool copyPartScore = true);

    ~Excerpt();

    QList<Part*>& parts() { return _parts; }
    const QList<Part*>& parts() const { return _parts; }
    bool containsPart(const Part* part) const;

    void removePart(const QString& id);

    void setParts(const QList<Part*>& p) { _parts = p; }

    int nstaves() const;
    bool isEmpty() const;

    QMultiMap<int, int>& tracks() { return _tracks; }
    void setTracks(const QMultiMap<int, int>& t) { _tracks = t; }

    MasterScore* oscore() const { return _oscore; }
    Score* partScore() const { return _partScore; }
    void setPartScore(Score* s);

    void read(XmlReader&);

    bool operator!=(const Excerpt&) const;
    bool operator==(const Excerpt&) const;

    QString title() const { return _title; }
    void setTitle(const QString& s) { _title = s; }

    static QList<Excerpt*> createExcerptsFromParts(const QList<Part*>& parts);
    static Excerpt* createExcerptFromPart(Part* part);

    static void createExcerpt(Excerpt*);
    static void cloneStaves(Score* oscore, Score* score, const QList<int>& sourceStavesIndexes, QMultiMap<int, int>& allTracks);
    static void cloneStaff(Staff* ostaff, Staff* nstaff);
    static void cloneStaff2(Staff* ostaff, Staff* nstaff, const Fraction& startTick, const Fraction& endTick);

private:
    static QString formatTitle(const QString& partName, const QList<Excerpt*>&);
    static void processLinkedClone(Element* ne, Score* score, int strack);
};
}     // namespace Ms
#endif
