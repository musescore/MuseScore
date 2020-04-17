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

#ifndef __ELEMENTGROUP_H__
#define __ELEMENTGROUP_H__

namespace Ms {

class Element;
class EditData;

//-------------------------------------------------------------------
//   ElementGroup
///   Base class for implementing logic to handle groups of elements
///   together in certain operations.
//-------------------------------------------------------------------

class ElementGroup {
   public:
      virtual ~ElementGroup() {}

      virtual void startDrag(EditData&) = 0;
      virtual QRectF drag(EditData&) = 0;
      virtual void endDrag(EditData&) = 0;

      virtual bool enabled() const { return true; }
      };

//-------------------------------------------------------------------
//   DisabledElementGroup
//-------------------------------------------------------------------

class DisabledElementGroup final : public ElementGroup {
   public:
      bool enabled() const override { return false; }

      void startDrag(EditData&) override {}
      QRectF drag(EditData&) override { return QRectF(); }
      void endDrag(EditData&) override {}
      };

//-------------------------------------------------------------------
//   SingleElementGroup
///   Element group for single element.
//-------------------------------------------------------------------

class SingleElementGroup final : public ElementGroup {
      Element* e;
   public:
      SingleElementGroup(Element* el) : e(el) {}

      void startDrag(EditData& ed) override;
      QRectF drag(EditData& ed) override;
      void endDrag(EditData& ed) override;
      };

} // namespace Ms

#endif
