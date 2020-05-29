//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __DYNAMICHAIRPINGROUP_H__
#define __DYNAMICHAIRPINGROUP_H__

#include "elementgroup.h"

namespace Ms {
class Dynamic;
class Hairpin;
class HairpinSegment;

//-------------------------------------------------------------------
//   HairpinWithDynamicsDragGroup
///   Sequence of Dynamics and Hairpins
//-------------------------------------------------------------------

class HairpinWithDynamicsDragGroup : public ElementGroup
{
    Dynamic* startDynamic;
    HairpinSegment* hairpinSegment;
    Dynamic* endDynamic;

public:
    HairpinWithDynamicsDragGroup(Dynamic* start, HairpinSegment* hs, Dynamic* end)
        : startDynamic(start), hairpinSegment(hs), endDynamic(end) {}

    void startDrag(EditData&) override;
    QRectF drag(EditData&) override;
    void endDrag(EditData&) override;

    static std::unique_ptr<ElementGroup> detectFor(HairpinSegment* hs, std::function<bool(const Element*)> isDragged);
    static std::unique_ptr<ElementGroup> detectFor(Dynamic* d, std::function<bool(const Element*)> isDragged);
};

//-------------------------------------------------------------------
//   DynamicNearHairpinsDragGroup
//-------------------------------------------------------------------

class DynamicNearHairpinsDragGroup : public ElementGroup
{
    Hairpin* leftHairpin;
    Dynamic* dynamic;
    Hairpin* rightHairpin;

public:
    DynamicNearHairpinsDragGroup(Hairpin* left, Dynamic* d, Hairpin* right)
        : leftHairpin(left), dynamic(d), rightHairpin(right) {}

    void startDrag(EditData&) override;
    QRectF drag(EditData&) override;
    void endDrag(EditData&) override;

    static std::unique_ptr<ElementGroup> detectFor(Dynamic* d, std::function<bool(const Element*)> isDragged);
};
} // namespace Ms

#endif
