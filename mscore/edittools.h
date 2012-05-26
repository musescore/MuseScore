//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __EDITTOOLS_H__
#define __EDITTOOLS_H__

class Element;

//---------------------------------------------------------
//   EditTools
//---------------------------------------------------------

class EditTools : public QDockWidget {
      Q_OBJECT

      Element* _element;
      QDoubleSpinBox* _editX;
      QDoubleSpinBox* _editY;
      QLabel* xLabel;
      QLabel* yLabel;

   private slots:
      void editXChanged(double);
      void editYChanged(double);

   public:
      EditTools(QWidget* parent = 0);
      void setElement(Element*);
      void updateTools();
      void setEditPos(const QPointF&);
      };

#endif

