//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __JIANPUREST_H__
#define __JIANPUREST_H__

#include "rest.h"
#include "chordrest.h"

namespace Ms {

class Rest;
class TDuration;
enum class SymId;

//---------------------------------------------------------
//    @@ JianpuRest
///     This class implements a Jianpu (numbered notation) rest.
//---------------------------------------------------------

class JianpuRest : public Rest {
      Q_GADGET

   private:
      int _durationDashCount;               ///< Number of duration dashes ("-")
      std::vector<QLineF*> _durationBeams;  ///< Duration beams for hooks.

   public:
      JianpuRest(Score* s = 0);
      JianpuRest(Score*, const TDuration&);
      JianpuRest(const Rest&, bool link = false);
      JianpuRest(const JianpuRest&, bool link = false);
      ~JianpuRest();

      JianpuRest &operator=(const JianpuRest&) = delete;
      virtual JianpuRest* clone() const override { return new JianpuRest(*this, false); }
      virtual Element* linkedClone() override    { return new JianpuRest(*this, true); }

      virtual void read(XmlReader& xml) override;
      virtual void write(XmlWriter& xml) const override;

      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      };

} // namespace Ms
#endif

