//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __STICKING_H__
#define __STICKING_H__

#include "textbase.h"

namespace Ms {

//-----------------------------------------------------------------------------
//   @@ Sticking
///    Drum sticking
//-----------------------------------------------------------------------------

class Sticking final : public TextBase {
      virtual Sid getPropertyStyle(Pid) const override;

   public:
      Sticking(Score*);
      virtual Sticking* clone() const override    { return new Sticking(*this); }
      virtual ElementType type() const override   { return ElementType::STICKING; }
      Segment* segment() const                    { return (Segment*)parent(); }
      Measure* measure() const                    { return (Measure*)parent()->parent(); }

      virtual void layout() override;
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      };

}     // namespace Ms
#endif
