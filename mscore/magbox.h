//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: magbox.h 2460 2009-12-15 18:01:39Z wschweer $
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef __MAGBOX_H__
#define __MAGBOX_H__

namespace Ms {

class ScoreView;

//---------------------------------------------------------
//   magTable
//    list of strings shown in QComboBox "MagBox"
//---------------------------------------------------------

enum {
       MAG_25, MAG_50, MAG_75, MAG_100, MAG_150, MAG_200, MAG_400, MAG_800, MAG_1600,
       MAG_PAGE_WIDTH, MAG_PAGE, MAG_DBL_PAGE,
       MAG_FREE
      };

//---------------------------------------------------------
//   MagValidator
//---------------------------------------------------------

class MagValidator : public QValidator {
      Q_OBJECT

      virtual State validate(QString&, int&) const;

   public:
      MagValidator(QObject* parent = 0);
      };

//---------------------------------------------------------
//   MagBox
//---------------------------------------------------------

class MagBox : public QComboBox {
      Q_OBJECT

      double freeMag;

   private slots:
      void indexChanged(int);

   signals:
      void magChanged(int idx);

   public:
      MagBox(QWidget* parent = 0);
      void setMag(double);
      void setMagIdx(int);
      double getMag(ScoreView*);
      };




} // namespace Ms
#endif



