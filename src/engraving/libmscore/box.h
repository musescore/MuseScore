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

namespace mu::engraving {
//---------------------------------------------------------
//   @@ Box
///    virtual base class for frames "boxes"
//---------------------------------------------------------

class Box : public MeasureBase
{
    OBJECT_ALLOCATOR(engraving, Box)

    Spatium _boxWidth             { Spatium(0) };      // only valid for HBox
    Spatium _boxHeight            { Spatium(0) };      // only valid for VBox
    Millimetre _topGap            { Millimetre(0.0) }; // distance from previous system (left border for hbox)
                                                       // initialized with Sid::systemFrameDistance
    Millimetre _bottomGap         { Millimetre(0.0) }; // distance to next system (right border for hbox)
                                                       // initialized with Sid::frameSystemDistance
    double _leftMargin             { 0.0 };
    double _rightMargin            { 0.0 };             // inner margins in metric mm
    double _topMargin              { 0.0 };
    double _bottomMargin           { 0.0 };
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

    virtual bool acceptDrop(EditData&) const override;
    virtual EngravingItem* drop(EditData&) override;
    virtual void add(EngravingItem* e) override;

    mu::RectF contentRect() const;
    Spatium boxWidth() const { return _boxWidth; }
    void setBoxWidth(Spatium val) { _boxWidth = val; }
    Spatium boxHeight() const { return _boxHeight; }
    void setBoxHeight(Spatium val) { _boxHeight = val; }
    double leftMargin() const { return _leftMargin; }
    double rightMargin() const { return _rightMargin; }
    double topMargin() const { return _topMargin; }
    double bottomMargin() const { return _bottomMargin; }
    void setLeftMargin(double val) { _leftMargin = val; }
    void setRightMargin(double val) { _rightMargin = val; }
    void setTopMargin(double val) { _topMargin = val; }
    void setBottomMargin(double val) { _bottomMargin = val; }
    Millimetre topGap() const { return _topGap; }
    void setTopGap(Millimetre val) { _topGap = val; }
    Millimetre bottomGap() const { return _bottomGap; }
    void setBottomGap(Millimetre val) { _bottomGap = val; }
    bool isAutoSizeEnabled() const { return _isAutoSizeEnabled; }
    void setAutoSizeEnabled(const bool val) { _isAutoSizeEnabled = val; }
    void copyValues(Box* origin);

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    String accessibleExtraInfo() const override;

    // TODO: add a grip for moving the entire box
    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override { return { mu::PointF() }; }   // overridden in descendants
};

//---------------------------------------------------------
//   @@ HBox
///    horizontal frame
//---------------------------------------------------------

class HBox final : public Box
{
    OBJECT_ALLOCATOR(engraving, HBox)
    DECLARE_CLASSOF(ElementType::HBOX)

    bool _createSystemHeader { true };

public:
    HBox(System* parent);
    virtual ~HBox() {}

    HBox* clone() const override { return new HBox(*this); }

    mu::RectF drag(EditData&) override;
    void layout2();
    bool isMovable() const override;
    void computeMinWidth() override;

    bool createSystemHeader() const { return _createSystemHeader; }
    void setCreateSystemHeader(bool val) { _createSystemHeader = val; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    std::vector<mu::PointF> gripsPositions(const EditData&) const override;
};

//---------------------------------------------------------
//   @@ VBox
///    vertical frame
//---------------------------------------------------------

class VBox : public Box
{
    OBJECT_ALLOCATOR(engraving, VBox)
    DECLARE_CLASSOF(ElementType::VBOX)

public:
    VBox(const ElementType& type, System* parent);
    VBox(System* parent);
    virtual ~VBox() {}

    VBox* clone() const override { return new VBox(*this); }

    double minHeight() const;
    double maxHeight() const;

    PropertyValue getProperty(Pid propertyId) const override;

    void startEditDrag(EditData&) override;

    std::vector<mu::PointF> gripsPositions(const EditData&) const override;
};

//---------------------------------------------------------
//   @@ FBox
///    frame containing fret diagrams
//---------------------------------------------------------

class FBox : public VBox
{
    OBJECT_ALLOCATOR(engraving, FBox)
    DECLARE_CLASSOF(ElementType::FBOX)

public:
    FBox(System* parent)
        : VBox(ElementType::FBOX, parent) {}
    virtual ~FBox() {}

    FBox* clone() const override { return new FBox(*this); }

    void add(EngravingItem*) override;
};
} // namespace mu::engraving
#endif
