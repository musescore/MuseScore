//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

enum class MagIdx : char {
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
      void textChanged();

   signals:
      void magChanged(MagIdx);

   public:
      MagBox(QWidget* parent = 0);
      void setMag(double);
      void setMagIdx(MagIdx);
      double getMag(ScoreView*) const;
      double getLMag(ScoreView*) const;
      void setEnabled(bool val) { QComboBox::setEnabled(val); }
      QString currentText() const { return QComboBox::currentText(); }
      int count() const { return QComboBox::count(); }
      void removeItem(int i) { QComboBox::removeItem(i); }
      };


} // namespace Ms

Q_DECLARE_METATYPE(Ms::MagIdx);

#endif



