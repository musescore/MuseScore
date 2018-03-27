//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "inspectorBarline.h"
#include "libmscore/score.h"
#include "libmscore/barline.h"
#include "libmscore/staff.h"

namespace Ms {
//---------------------------------------------------------
//   InspectorBarLine
//---------------------------------------------------------

InspectorBarLine::InspectorBarLine(QWidget* parent)
   : InspectorElementBase(parent)
      {
      s.setupUi(addWidget());
      b.setupUi(addWidget());

      for (auto i : BarLine::barLineTable)
            b.type->addItem(qApp->translate("Palette", i.userName), int(i.type));

      const std::vector<InspectorItem> il = {
            { Pid::LEADING_SPACE,     1, s.leadingSpace,  s.resetLeadingSpace  },
            { Pid::BARLINE_TYPE,      0, b.type,     b.resetType     },
            { Pid::BARLINE_SPAN,      0, b.span,     b.resetSpan     },
            { Pid::BARLINE_SPAN_FROM, 0, b.spanFrom, b.resetSpanFrom },
            { Pid::BARLINE_SPAN_TO,   0, b.spanTo,   b.resetSpanTo   },
            };
      const std::vector<InspectorPanel> ppList = {
            { s.title, s.panel },
            { b.title, b.panel }
            };

      mapSignals(il, ppList);

#if 0
      // when any of the span parameters is changed, span data need to be managed
      connect(b.span,          SIGNAL(valueChanged(int)), SLOT(manageSpanData()));
      connect(b.spanFrom,      SIGNAL(valueChanged(int)), SLOT(manageSpanData()));
      connect(b.spanTo,        SIGNAL(valueChanged(int)), SLOT(manageSpanData()));
#endif
      connect(b.presetDefault, SIGNAL(clicked()),         SLOT(presetDefaultClicked()));
      connect(b.presetTick1,   SIGNAL(clicked()),         SLOT(presetTick1Clicked()));
      connect(b.presetTick2,   SIGNAL(clicked()),         SLOT(presetTick2Clicked()));
      connect(b.presetShort1,  SIGNAL(clicked()),         SLOT(presetShort1Clicked()));
      connect(b.presetShort2,  SIGNAL(clicked()),         SLOT(presetShort2Clicked()));

      connect(b.setAsStaffDefault, SIGNAL(clicked()),     SLOT(setAsStaffDefault()));
      }

//---------------------------------------------------------
//   setAsStaffDefault
//---------------------------------------------------------

void InspectorBarLine::setAsStaffDefault()
      {
      BarLine* bl = toBarLine(inspector->element());
      Staff* staff = bl->staff();
      Score* score = bl->score();
      score->startCmd();
      staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN,      bl->getProperty(Pid::BARLINE_SPAN));
      staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_FROM, bl->getProperty(Pid::BARLINE_SPAN_FROM));
      staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_TO,   bl->getProperty(Pid::BARLINE_SPAN_TO));
      if (bl->barLineType() == BarLineType::NORMAL)
            bl->setGenerated(true);
      score->endCmd();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

#include <QStandardItem>

void InspectorBarLine::setElement()
      {
      InspectorElementBase::setElement();
      BarLine* bl = toBarLine(inspector->element());

      // enable / disable individual type combo items according to score and selected bar line status
      bool bMultiStaff  = bl->score()->nstaves() > 1;
      BarLineType blt   = bl->barLineType();
//      bool isRepeat     = blt & (BarLineType::START_REPEAT | BarLineType::END_REPEAT | BarLineType::END_START_REPEAT);
      bool isRepeat     = blt & (BarLineType::START_REPEAT | BarLineType::END_REPEAT);

      const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(b.type->model());
      int i = 0;
      for (auto& k : BarLine::barLineTable) {
            QStandardItem* item = model->item(i);
            // if combo item is repeat type, should be disabled for multi-staff scores
//            if (k.type & (BarLineType::START_REPEAT | BarLineType::END_REPEAT | BarLineType::END_START_REPEAT)) {
            if (k.type & (BarLineType::START_REPEAT | BarLineType::END_REPEAT)) {
                  // disable / enable
                  item->setFlags(bMultiStaff ?
                        item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled) :
                        item->flags() | (Qt::ItemFlags)(Qt::ItemIsSelectable|Qt::ItemIsEnabled) );
                  }
            // if combo item is NOT repeat type, should be disabled if selected bar line is a repeat
            else {
                  item->setFlags(isRepeat ?
                        item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled) :
                        item->flags() | (Qt::ItemFlags)(Qt::ItemIsSelectable|Qt::ItemIsEnabled) );
                  }
            ++i;
            }
#if 0
      blockSpanDataSignals(true);
      manageSpanData();
      blockSpanDataSignals(false);
#endif
      }

//---------------------------------------------------------
//   presetDefaultClicked
//---------------------------------------------------------

void InspectorBarLine::presetDefaultClicked()
      {
      Score* score = inspector->element()->score();
      score->startCmd();

      BarLine* bl;
      for (Element* e : *inspector->el()) {
            if (e->isBarLine()) {
                  bl = toBarLine(e);
                  bl->undoResetProperty(Pid::BARLINE_SPAN);
                  bl->undoResetProperty(Pid::BARLINE_SPAN_FROM);
                  bl->undoResetProperty(Pid::BARLINE_SPAN_TO);
                  }
            }

      score->endCmd();
      }

//---------------------------------------------------------
//   presetTick1Clicked
//---------------------------------------------------------

void InspectorBarLine::presetTick1Clicked()
      {
      Score* score = inspector->element()->score();
      score->startCmd();

      BarLine* bl;
      for (Element* e : *inspector->el()) {
            if (e->isBarLine()) {
                  bl = toBarLine(e);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN, false);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN_FROM, BARLINE_SPAN_TICK1_FROM);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN_TO,   BARLINE_SPAN_TICK1_TO);
                  }
            }

      score->endCmd();
      }

//---------------------------------------------------------
//   presetTick2Clicked
//---------------------------------------------------------

void InspectorBarLine::presetTick2Clicked()
      {
      Score* score = inspector->element()->score();
      score->startCmd();

      BarLine* bl;
      for (Element* e : *inspector->el()) {
            if (e->isBarLine()) {
                  bl = toBarLine(e);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN, false);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN_FROM, BARLINE_SPAN_TICK2_FROM);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN_TO,   BARLINE_SPAN_TICK2_TO);
                  }
            }

      score->endCmd();
      }

//---------------------------------------------------------
//   presetShort1Clicked
//---------------------------------------------------------

void InspectorBarLine::presetShort1Clicked()
      {
      Score* score = inspector->element()->score();
      score->startCmd();

      BarLine* bl;
      for (Element* e : *inspector->el()) {
            if (e->isBarLine()) {
                  bl = toBarLine(e);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN, false);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN_FROM, BARLINE_SPAN_SHORT1_FROM);
                  int shortDelta = bl->staff() ? (bl->staff()->lines(bl->tick()) - 5) * 2 : 0;
                  bl->undoChangeProperty(Pid::BARLINE_SPAN_TO,   BARLINE_SPAN_SHORT1_TO + shortDelta);
                  }
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   presetShort2Clicked
//---------------------------------------------------------

void InspectorBarLine::presetShort2Clicked()
      {
      Score* score = inspector->element()->score();
      score->startCmd();

      BarLine* bl;
      for (Element* e : *inspector->el()) {
            if (e->isBarLine()) {
                  bl = toBarLine(e);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN, false);
                  bl->undoChangeProperty(Pid::BARLINE_SPAN_FROM, BARLINE_SPAN_SHORT2_FROM);
                  int shortDelta = bl->staff() ? (bl->staff()->lines(bl->tick()) - 5) * 2 : 0;
                  bl->undoChangeProperty(Pid::BARLINE_SPAN_TO,   BARLINE_SPAN_SHORT2_TO + shortDelta);
                  }
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   manageSpanData
//
//    Makes sure span data are legal and consistent
//---------------------------------------------------------

void InspectorBarLine::manageSpanData()
      {
#if 0
      BarLine* bl = toBarLine(inspector->element());

      // determine MIN and MAX for SPANFROM and SPANTO
      Staff* staffFrom  = bl->staff();
      Staff* staffTo    = bl->score()->staff(bl->staffIdx() + bl->span() - 1);
      int staffFromLines= (staffFrom ? staffFrom->lines(bl->tick()) : 5);
      int staffToLines  = (staffTo   ? staffTo->lines(bl->tick())   : 5);

      // From:    min = minimum possible according to number of staff lines
      //          max = if same as To, at least 1sp (2 units) above To; if not, max possible according to num.of lines

      int min     = staffFromLines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
      int max     = bl->span() < 2 ? bl->spanTo() - MIN_BARLINE_FROMTO_DIST
                        : (staffFromLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (staffFromLines-1) * 2 + 2);
      b.spanFrom->setMinimum(min);
      b.spanFrom->setMaximum(max);

      // To:      min = if same as From, at least 1sp (2 units) below From; if not, min possible according to num.of lines
      //          max = max possible according to number of staff lines
      min   = bl->span() < 2 ? bl->spanFrom() + MIN_BARLINE_FROMTO_DIST
                  : (staffToLines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO);
      max   = staffToLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (staffToLines-1) * 2 + 2;

      b.spanTo->setMinimum(min);
      b.spanTo->setMaximum(max);

      // determine MAX for SPAN
      max = bl->score()->nstaves() - bl->staffIdx();
      b.span->setMaximum(max);
#endif
      }

//---------------------------------------------------------
//   blockSpanDataSignals
//
//    block/unblok signals for span value controls
//---------------------------------------------------------

void InspectorBarLine::blockSpanDataSignals(bool val)
      {
      b.span->blockSignals(val);
      b.spanFrom->blockSignals(val);
      b.spanTo->blockSignals(val);
      }

}


