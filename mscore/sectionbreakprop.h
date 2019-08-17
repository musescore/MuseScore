//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __SECTIONBREAKPROP_H__
#define __SECTIONBREAKPROP_H__

#include "ui_sectionbreak.h"

namespace Ms {

class LayoutBreak;

//---------------------------------------------------------
//   SectionBreakProperties
//---------------------------------------------------------

class SectionBreakProperties : public QDialog, public Ui::SectionBreakProperties {
      Q_OBJECT

      virtual void hideEvent(QHideEvent*);
   public:
      SectionBreakProperties(LayoutBreak*, QWidget* parent = 0);
      double pause() const;
      bool startWithLongNames() const;
      bool startWithMeasureOne() const;
      };


} // namespace Ms
#endif
