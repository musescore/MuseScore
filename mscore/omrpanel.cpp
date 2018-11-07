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

#include "musescore.h"
#include "omrpanel.h"
#include "scoreview.h"
#include "omr/omrview.h"
#include "omr/omr.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   showOmrPanel
//---------------------------------------------------------

void MuseScore::showOmrPanel(bool visible)
      {
      QAction* a = getAction("omr");
      if (visible) {
            if (!omrPanel) {
                  omrPanel = new OmrPanel();
                  connect(omrPanel, SIGNAL(omrPanelVisible(bool)), a, SLOT(setChecked(bool)));
                  addDockWidget(Qt::RightDockWidgetArea, omrPanel);
                  }
            }
      if (omrPanel)
            omrPanel->setVisible(visible);
      if (omrPanel && visible) {
            if (cv && cv->omrView())
                  omrPanel->setOmrView(cv->omrView());
            else
                  omrPanel->setOmrView(0);
            }
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   OmrPanel
//---------------------------------------------------------

OmrPanel::OmrPanel(QWidget* parent)
   : QDockWidget(tr("PDF Transcribing Assistant"), parent)
      {
      setObjectName("omrpanel");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      QWidget* mainWidget = new QWidget;
      mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      setWidget(mainWidget);

      op.setupUi(mainWidget);

      setOmrView(0);

      connect(op.showBarlines, SIGNAL(toggled(bool)), SLOT(showBarlinesToggled(bool)));
      connect(op.showLines,    SIGNAL(toggled(bool)), SLOT(showLinesToggled(bool)));
      connect(op.showSlices,   SIGNAL(toggled(bool)), SLOT(showSlicesToggled(bool)));
      connect(op.showStaves,   SIGNAL(toggled(bool)), SLOT(showStavesToggled(bool)));
      connect(op.spatium,      SIGNAL(valueChanged(double)), SLOT(spatiumChanged(double)));
      connect(op.process,      SIGNAL(clicked()), SLOT(processClicked()));
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void OmrPanel::closeEvent(QCloseEvent* ev)
      {
      emit omrPanelVisible(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   showBarlinesToggled
//---------------------------------------------------------

void OmrPanel::showBarlinesToggled(bool val)
      {
      if (omrView)
            omrView->setShowBarlines(val);
      }

//---------------------------------------------------------
//   showLinesToggled
//---------------------------------------------------------

void OmrPanel::showLinesToggled(bool val)
      {
      if (omrView)
            omrView->setShowLines(val);
      }

//---------------------------------------------------------
//   showSlicesToggled
//---------------------------------------------------------

void OmrPanel::showSlicesToggled(bool val)
      {
      if (omrView)
            omrView->setShowSlices(val);
      }

//---------------------------------------------------------
//   showStavesToggled
//---------------------------------------------------------

void OmrPanel::showStavesToggled(bool val)
      {
      if (omrView)
            omrView->setShowStaves(val);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void OmrPanel::spatiumChanged(double val)
      {
      if (omrView) {
            omrView->omr()->setSpatium(val * omrView->omr()->dpmm());
            omrView->update();
            }
      }

//---------------------------------------------------------
//   blockSignals
//---------------------------------------------------------

void OmrPanel::blockSignals(bool val)
      {
      op.showBarlines->blockSignals(val);
      op.showLines->blockSignals(val);
      op.showSlices->blockSignals(val);
      op.showStaves->blockSignals(val);
      op.spatium->blockSignals(val);
      }

//---------------------------------------------------------
//   enableGui
//---------------------------------------------------------

void OmrPanel::enableGui(bool val)
      {
      op.showBarlines->setEnabled(val);
      op.showLines->setEnabled(val);
      op.showSlices->setEnabled(val);
      op.showStaves->setEnabled(val);
      op.spatium->setEnabled(val);
      op.process->setEnabled(val);
      }

//---------------------------------------------------------
//   setOmr
//---------------------------------------------------------

void OmrPanel::setOmrView(OmrView* v)
      {
      omrView = v;
      if (omrView) {
            enableGui(true);
            blockSignals(true);

            op.showBarlines->setChecked(omrView->showBarlines());
            op.showLines->setChecked(omrView->showLines());
            op.showSlices->setChecked(omrView->showSlices());
            op.showStaves->setChecked(omrView->showStaves());
            op.spatium->setValue(omrView->omr()->spatiumMM());

            blockSignals(false);
            }
      else {
            enableGui(false);
            }
      }

//---------------------------------------------------------
//   processClicked
//---------------------------------------------------------

void OmrPanel::processClicked()
      {
      if (omrView) {
            //omrView->omr()->process();
            omrView->update();
            setOmrView(omrView);    // update values
            }
      }

}


