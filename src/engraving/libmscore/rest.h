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

#ifndef __REST_H__
#define __REST_H__

#include "chordrest.h"
#include "notedot.h"
#include "symid.h"

namespace Ms {
class TDuration;

//---------------------------------------------------------
//    @@ Rest
///     This class implements a rest.
//---------------------------------------------------------

class Rest : public ChordRest
{
public:
    Rest(Score* s = 0);
    Rest(Score*, const TDuration&);
    Rest(const Rest&, bool link = false);
    ~Rest() { qDeleteAll(m_dots); }

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;

    virtual ElementType type() const override { return ElementType::REST; }
    Rest& operator=(const Rest&) = delete;

    Rest* clone() const override { return new Rest(*this, false); }
    Element* linkedClone() override { return new Rest(*this, true); }
    Measure* measure() const override { return parent() ? toMeasure(parent()->parent()) : 0; }
    qreal mag() const override;

    void draw(mu::draw::Painter*) const override;
    void scanElements(void* data, void (* func)(void*, Element*), bool all = true) override;
    void setTrack(int val) override;

    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;
    void layout() override;

    bool isGap() const { return m_gap; }
    virtual void setGap(bool v) { m_gap = v; }

    void reset() override;

    virtual void add(Element*) override;
    virtual void remove(Element*) override;

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;

    SymId getSymbol(TDuration::DurationType type, int line, int lines,  int* yoffset);

    void checkDots();
    void layoutDots();
    NoteDot* dot(int n);
    int getDotline() const { return m_dotline; }
    static int getDotline(TDuration::DurationType durationType);
    SymId sym() const { return m_sym; }
    bool accent();
    void setAccent(bool flag);
    int computeLineOffset(int lines);

    int upLine() const override;
    int downLine() const override;
    mu::PointF stemPos() const override;
    qreal stemPosX() const override;
    mu::PointF stemPosBeam() const override;
    qreal rightEdge() const override;

    void localSpatiumChanged(qreal oldValue, qreal newValue) override;
    QVariant propertyDefault(Pid) const override;
    void resetProperty(Pid id) override;
    bool setProperty(Pid propertyId, const QVariant& v) override;
    QVariant getProperty(Pid propertyId) const override;
    void undoChangeDotsVisible(bool v);

    Element* nextElement() override;
    Element* prevElement() override;
    QString accessibleInfo() const override;
    QString screenReaderInfo() const override;
    Shape shape() const override;
    void editDrag(EditData& editData) override;

    bool shouldNotBeDrawn() const;

protected:
    Sid getPropertyStyle(Pid pid) const override;
    virtual mu::RectF numberRect() const { return mu::RectF(); } // TODO: add style to show number over 1-measure rests

private:

    // values calculated by layout:
    SymId m_sym;
    int m_dotline   { -1 };          // depends on rest symbol
    bool m_gap      { false };       // invisible and not selectable for user
    std::vector<NoteDot*> m_dots;

    mu::RectF drag(EditData&) override;
    qreal upPos() const override;
    qreal downPos() const override;
    void setOffset(const mu::PointF& o) override;
};
}     // namespace Ms
#endif
