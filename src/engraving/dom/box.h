/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_BOX_H
#define MU_ENGRAVING_BOX_H

#include "measurebase.h"
#include "property.h"

namespace mu::engraving {
//---------------------------------------------------------
//   Box
//   virtual base class for frames "boxes"
//---------------------------------------------------------

class Box : public MeasureBase
{
    OBJECT_ALLOCATOR(engraving, Box)

public:
    Box(const ElementType& type, System* parent);

    virtual bool isEditable() const override { return true; }

    virtual bool isEditAllowed(EditData&) const override;
    virtual bool edit(EditData&) override;
    virtual void startEditDrag(EditData&) override;
    virtual void editDrag(EditData&) override;

    virtual bool acceptDrop(EditData&) const override;
    virtual EngravingItem* drop(EditData&) override;
    virtual void add(EngravingItem* e) override;
    virtual double absoluteFromSpatium(const Spatium& val) const override;

    RectF contentRect() const;
    Spatium boxWidth() const { return m_boxWidth; }
    void setBoxWidth(Spatium val) { m_boxWidth = val; }
    Spatium boxHeight() const { return m_boxHeight; }
    void setBoxHeight(Spatium val) { m_boxHeight = val; }
    double leftMargin() const { return m_leftMargin; }
    double rightMargin() const { return m_rightMargin; }
    double topMargin() const { return m_topMargin; }
    double bottomMargin() const { return m_bottomMargin; }
    void setLeftMargin(double val) { m_leftMargin = val; }
    void setRightMargin(double val) { m_rightMargin = val; }
    void setTopMargin(double val) { m_topMargin = val; }
    void setBottomMargin(double val) { m_bottomMargin = val; }
    Spatium topGap() const { return m_topGap; }
    void setTopGap(Spatium val) { m_topGap = val; }
    Spatium bottomGap() const { return m_bottomGap; }
    void setBottomGap(Spatium val) { m_bottomGap = val; }
    bool isAutoSizeEnabled() const { return m_isAutoSizeEnabled; }
    void setAutoSizeEnabled(const bool val) { m_isAutoSizeEnabled = val; }
    void copyValues(Box* origin);
    bool isTitleFrame() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    String accessibleExtraInfo() const override;

    // TODO: add a grip for moving the entire box
    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<PointF> gripsPositions(const EditData&) const override { return { PointF() }; }   // overridden in descendants

    bool canBeExcludedFromOtherParts() const override { return true; }
    void manageExclusionFromParts(bool exclude) override;

private:
    Spatium m_boxWidth;         // only valid for HBox
    Spatium m_boxHeight;        // only valid for VBox
    Spatium m_topGap;           // distance from previous system (left border for hbox)
                                // initialized with Sid::systemFrameDistance
    Spatium m_bottomGap;        // distance to next system (right border for hbox)
                                // initialized with Sid::frameSystemDistance
    double m_leftMargin = 0.0;
    double m_rightMargin = 0.0; // inner margins in metric mm
    double m_topMargin = 0.0;
    double m_bottomMargin = 0.0;
    bool m_isAutoSizeEnabled = true;
};

//---------------------------------------------------------
//   HBox
//   horizontal frame
//---------------------------------------------------------

class HBox final : public Box
{
    OBJECT_ALLOCATOR(engraving, HBox)
    DECLARE_CLASSOF(ElementType::HBOX)

public:
    HBox(System* parent);

    HBox* clone() const override { return new HBox(*this); }

    RectF drag(EditData&) override;

    bool isMovable() const override;
    void computeMinWidth() override;

    bool createSystemHeader() const { return m_createSystemHeader; }
    void setCreateSystemHeader(bool val) { m_createSystemHeader = val; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    std::vector<PointF> gripsPositions(const EditData&) const override;

private:

    bool m_createSystemHeader = true;
};

//---------------------------------------------------------
//   VBox
//   vertical frame
//---------------------------------------------------------

class VBox : public Box
{
    OBJECT_ALLOCATOR(engraving, VBox)
    DECLARE_CLASSOF(ElementType::VBOX)

public:
    VBox(const ElementType& type, System* parent);
    VBox(System* parent);

    VBox* clone() const override { return new VBox(*this); }

    double minHeight() const;
    double maxHeight() const;

    PropertyValue getProperty(Pid propertyId) const override;
    PropertyValue propertyDefault(Pid) const override;

    void startEditDrag(EditData&) override;

    std::vector<PointF> gripsPositions(const EditData&) const override;
};

//---------------------------------------------------------
//   FBox
//   frame containing fret diagrams
//---------------------------------------------------------

class FBox : public VBox
{
    OBJECT_ALLOCATOR(engraving, FBox)
    DECLARE_CLASSOF(ElementType::FBOX)

public:
    FBox(System* parent);

    FBox* clone() const override { return new FBox(*this); }

    void add(EngravingItem*) override;

    double textScale() const { return m_textScale; }
    void setTextScale(double scale) { m_textScale = scale; }

    double diagramScale() const { return m_diagramScale; }
    void setDiagramScale(double scale) { m_diagramScale = scale; }

    Spatium columnGap() const { return m_columnGap; }
    void setColumnGap(Spatium gap) { m_columnGap = gap; }

    Spatium rowGap() const { return m_rowGap; }
    void setRowGap(Spatium gap) { m_rowGap = gap; }

    int chordsPerRow() const { return m_chordsPerRow; }
    void setChordsPerRow(int chords) { m_chordsPerRow = chords; }

    AlignH contentHorizontallAlignment() const { return m_contentAlignmentH; }
    void setContentHorizontallAlignment(AlignH alignment) { m_contentAlignmentH = alignment; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& val) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    void init();

    void undoReorderElements(const std::vector<EID>& newOrderElementsIds);

    FretDiagram* makeFretDiagram(const EngravingItem* item);

    struct LayoutData : public VBox::LayoutData {
        double cellWidth = 0.0;
        double cellHeight = 0.0;

        double totalTableHeight = 0.0;
        double totalTableWidth = 0.0;

        double defaultMargins = 0.0;
    };

    DECLARE_LAYOUTDATA_METHODS(FBox)

private:
    void resolveContentRect();

    double m_textScale = 0.0;
    double m_diagramScale = 0.0;
    Spatium m_columnGap;
    Spatium m_rowGap;
    int m_chordsPerRow = 0;

    AlignH m_contentAlignmentH = AlignH::HCENTER;
};

//---------------------------------------------------------
//   TBox
//   Text frame.
//---------------------------------------------------------

class Text;
class TBox : public VBox
{
    OBJECT_ALLOCATOR(engraving, TBox)
    DECLARE_CLASSOF(ElementType::TBOX)

public:
    TBox(System* parent);
    TBox(const TBox&);
    ~TBox() override;

    Text* text() const { return m_text; }
    void resetText(Text* text);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all = true) override;

    TBox* clone() const override { return new TBox(*this); }

    EngravingItem* drop(EditData&) override;
    void add(EngravingItem* e) override;
    void remove(EngravingItem* el) override;

    PropertyValue propertyDefault(Pid) const override;

    String accessibleExtraInfo() const override;

    int gripsCount() const override;
    Grip initialEditModeGrip() const override;
    Grip defaultGrip() const override;

    bool needStartEditingAfterSelecting() const override { return false; }

private:
    Text* m_text = nullptr;
};
} // namespace mu::engraving
#endif
