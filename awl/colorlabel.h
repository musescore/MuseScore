//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2002-2007 by Werner Schweer and others
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

#ifndef __AWLCOLORLABEL_H__
#define __AWLCOLORLABEL_H__

namespace Awl {

//---------------------------------------------------------
//   ColorLabel
//---------------------------------------------------------

class ColorLabel : public QFrame {
      Q_OBJECT
      Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
      Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
      Q_PROPERTY(QPixmap* pixmap READ pixmap WRITE setPixmap NOTIFY pixmapChanged)

      QColor _color;
      QString _text;
      QPixmap* _pixmap;

      virtual void paintEvent(QPaintEvent*) override;
      virtual void mousePressEvent(QMouseEvent*) override;
      virtual void keyPressEvent(QKeyEvent*) override;

   signals:
      void colorChanged(const QColor&);
      void pixmapChanged(const QPixmap*);
      void textChanged(const QString&);

   public:
      ColorLabel(QWidget* parent = nullptr);
      ~ColorLabel();

      virtual QSize sizeHint() const override;
      const QColor color() const;
      QPixmap* pixmap() const;
      const QString& text() const;

   public slots:
      void setText(const QString& text);
      void setPixmap(QPixmap*);
      void setColor(const QColor&);
      void getColor();

};

}  // namespace Awl
#endif

