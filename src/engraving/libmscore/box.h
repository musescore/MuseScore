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

#ifndef __BOX_H__
#define __BOX_H__

/**
 \file
 Definition of HBox and VBox classes.
*/

#include "measurebase.h"
#include "property.h"

namespace Ms {
//---------------------------------------------------------
//   @@ Box
///    virtual base class for frames "boxes"
//---------------------------------------------------------

class Box : public MeasureBase
{
    Spatium _boxWidth             { Spatium(0) };      // only valid for HBox
    Spatium _boxHeight            { Spatium(0) };      // only valid for VBox
    Millimetre _topGap            { Millimetre(0.0) }; // distance from previous system (left border for hbox)
                                                       // initialized with Sid::systemFrameDistance
    Millimetre _bottomGap         { Millimetre(0.0) }; // distance to next system (right border for hbox)
                                                       // initialized with Sid::frameSystemDistance
    qreal _leftMargin             { 0.0 };
    qreal _rightMargin            { 0.0 };             // inner margins in metric mm
    qreal _topMargin              { 0.0 };
    qreal _bottomMargin           { 0.0 };
    bool _isAutoSizeEnabled       { true };

public:
    Box(const ElementType& type, System* parent);

    virtual void draw(mu::draw::Painter*) const override;
    virtual bool isEditable() const override { return true; }

    virtual bool isEditAllowed(EditData&) const override;
    virtual bool edit(EditData&) override;
    virtual void startEditDrag(EditData&) override;
    virtual void editDrag(EditData&) override;
    virtual void endEdit(EditData&) override;

    virtual void layout() override;
    virtual void write(XmlWriter&) const override;
    virtual void write(XmlWriter& xml, int, bool, bool) const override { write(xml); }
    virtual void writeProperties(XmlWriter&) const override;
    virtual bool readProperties(XmlReader&) override;
    virtual void read(XmlReader&) override;
    virtual bool acceptDrop(EditData&) const override;
    virtual EngravingItem* drop(EditData&) override;
    virtual void add(EngravingItem* e) override;

    mu::RectF contentRect() const;
    Spatium boxWidth() const { return _boxWidth; }
    void setBoxWidth(Spatium val) { _boxWidth = val; }
    Spatium boxHeight() const { return _boxHeight; }
    void setBoxHeight(Spatium val) { _boxHeight = val; }
    qreal leftMargin() const { return _leftMargin; }
    qreal rightMargin() const { return _rightMargin; }
    qreal topMargin() const { return _topMargin; }
    qreal bottomMargin() const { return _bottomMargin; }
    void setLeftMargin(qreal val) { _leftMargin = val; }
    void setRightMargin(qreal val) { _rightMargin = val; }
    void setTopMargin(qreal val) { _topMargin = val; }
    void setBottomMargin(qreal val) { _bottomMargin = val; }
    Millimetre topGap() const { return _topGap; }
    void setTopGap(Millimetre val) { _topGap = val; }
    Millimetre bottomGap() const { return _bottomGap; }
    void setBottomGap(Millimetre val) { _bottomGap = val; }
    bool isAutoSizeEnabled() const { return _isAutoSizeEnabled; }
    void setAutoSizeEnabled(const bool val) { _isAutoSizeEnabled = val; }
    void copyValues(Box* origin);

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;

    QString accessibleExtraInfo() const override;

    // TODO: add a grip for moving the entire box
    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override { return { mu::PointF() }; }   // overriden in descendants
};

//---------------------------------------------------------
//   @@ HBox
///    horizontal frame
//---------------------------------------------------------

class HBox final : public Box
{
    bool _createSystemHeader { true };

public:
    HBox(System* parent);
    virtual ~HBox() {}

    HBox* clone() const override { return new HBox(*this); }

    void layout() override;
    void writeProperties(XmlWriter&) const override;
    bool readProperties(XmlReader&) override;

    mu::RectF drag(EditData&) override;
    void endEditDrag(EditData&) override;
    void layout2();
    bool isMovable() const override;
    void computeMinWidth() override;

    bool createSystemHeader() const { return _createSystemHeader; }
    void setCreateSystemHeader(bool val) { _createSystemHeader = val; }

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;

    std::vector<mu::PointF> gripsPositions(const EditData&) const override;
};

//---------------------------------------------------------
//   @@ VBox
///    vertical frame
//---------------------------------------------------------

class VBox : public Box
{
public:
    VBox(const ElementType& type, System* parent);
    VBox(System* parent);
    virtual ~VBox() {}

    VBox* clone() const override { return new VBox(*this); }

    qreal minHeight() const;
    qreal maxHeight() const;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    void layout() override;

    void startEditDrag(EditData&) override;

    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

private:
    void adjustLayoutWithoutImages();
};

//---------------------------------------------------------
//   @@ FBox
///    frame containing fret diagrams
//---------------------------------------------------------

class FBox : public VBox
{
public:
    FBox(System* parent)
        : VBox(ElementType::FBOX, parent) {}
    virtual ~FBox() {}

    FBox* clone() const override { return new FBox(*this); }

    void layout() override;
    void add(EngravingItem*) override;
};
}     // namespace Ms
#endif
