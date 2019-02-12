//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "config.h"

#ifdef OMR

#ifndef __OMRPANEL_H__
#define __OMRPANEL_H__

#include "ui_omrpanel.h"

namespace Ms {

class OmrView;

//---------------------------------------------------------
//   OmrPanel
//---------------------------------------------------------

class OmrPanel : public QDockWidget {
      Q_OBJECT

      Ui::OmrPanel op;

      OmrView* omrView;

      virtual void closeEvent(QCloseEvent*);
      void blockSignals(bool);
      void enableGui(bool);

   private slots:
      void showBarlinesToggled(bool);
      void showLinesToggled(bool);
      void showSlicesToggled(bool);
      void showStavesToggled(bool);
      void processClicked();
      void spatiumChanged(double);

   signals:
      void omrPanelVisible(bool);

   public slots:

   public:
      OmrPanel(QWidget* parent = 0);
      void setOmrView(OmrView*);
   };


}// namespace Ms
#endif
#endif
