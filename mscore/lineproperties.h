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

#ifndef __LINEPROPERTIES_H__
#define __LINEPROPERTIES_H__

#include "globals.h"
#include "libmscore/line.h"
#include "libmscore/style.h"
#include "ui_lineproperties.h"

namespace Ms {

class TextLine;
class Text;

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

class LineProperties : public QDialog, public Ui::LinePropertiesDialog {
      Q_OBJECT

      TextLine* tl;
      Text* _beginText;
      Text* _continueText;

   private slots:
      virtual void accept();
      void beginTextToggled(bool);
      void beginSymbolToggled(bool);
      void continueTextToggled(bool);
      void continueSymbolToggled(bool);
      void beginTextProperties();
      void continueTextProperties();

   public:
      LineProperties(TextLine*, QWidget* parent = 0);
      };


} // namespace Ms
#endif

