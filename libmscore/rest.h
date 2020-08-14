//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __REST_H__
#define __REST_H__

#include "chordrest.h"
#include "notedot.h"

namespace Ms {
class TDuration;
enum class SymId;

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

    void draw(QPainter*) const override;
    void scanElements(void* data, void (* func)(void*, Element*), bool all = true) override;
    void setTrack(int val);

    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;
    void layout() override;

    bool isGap() const { return m_gap; }
    virtual void setGap(bool v) { m_gap = v; }

    void reset() override;

    virtual void add(Element*);
    virtual void remove(Element*);

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

    virtual int upLine() const;
    virtual int downLine() const;
    virtual QPointF stemPos() const;
    virtual qreal stemPosX() const;
    virtual QPointF stemPosBeam() const;
    virtual qreal rightEdge() const override;

    void localSpatiumChanged(qreal oldValue, qreal newValue) override;
    QVariant propertyDefault(Pid) const override;
    void resetProperty(Pid id);
    bool setProperty(Pid propertyId, const QVariant& v) override;
    QVariant getProperty(Pid propertyId) const override;
    void undoChangeDotsVisible(bool v);

    Element* nextElement() override;
    Element* prevElement() override;
    QString accessibleInfo() const override;
    QString screenReaderInfo() const override;
    Shape shape() const override;
    void editDrag(EditData& editData) override;

protected:
    Sid getPropertyStyle(Pid pid) const override;
    bool shouldNotBeDrawn() const;

private:
    // values calculated by layout:
    SymId m_sym;
    int m_dotline   { -1 };          // depends on rest symbol
    bool m_gap      { false };       // invisible and not selectable for user
    std::vector<NoteDot*> m_dots;

    QRectF drag(EditData&) override;
    qreal upPos() const override;
    qreal downPos() const override;
    void setOffset(const QPointF& o) override;
};
}     // namespace Ms
#endif
