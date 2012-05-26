//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/element.h"
#include "libmscore/beam.h"
#include "musescore.h"
#include "inspectorBase.h"
#include "inspector.h"

//---------------------------------------------------------
//   InspectorBase
//---------------------------------------------------------

InspectorBase::InspectorBase(QWidget* parent)
   : QWidget(parent)
      {
      resetMapper = new QSignalMapper(this);
      valueMapper = new QSignalMapper(this);

      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      inspector = static_cast<Inspector*>(parent);
      layout    = new QVBoxLayout;
      layout->setSizeConstraint(QLayout::SetNoConstraint);
      setLayout(layout);
      }

//---------------------------------------------------------
//   getValue
//---------------------------------------------------------

QVariant InspectorBase::getValue(int idx) const
      {
      const InspectorItem& ii = item(idx);
      QWidget* w              = ii.w;

      switch (propertyType(ii.t)) {
            case T_SIZE:
            case T_POINT:
            case T_SCALE:
            case T_SREAL:
            case T_REAL:      return w->property("value");
            case T_DIRECTION: return w->property("currentIndex");
            case T_BOOL:      return w->property("checked");
            default:
                  abort();
            }
      return QVariant();
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void InspectorBase::setValue(int idx, const QVariant& val)
      {
      const InspectorItem& ii = item(idx);
      QWidget* w        = ii.w;

      switch (propertyType(ii.t)) {
            case T_SIZE:
            case T_POINT:
            case T_SCALE:
            case T_SREAL:
            case T_REAL:
                  static_cast<QDoubleSpinBox*>(w)->setValue(val.toDouble());
                  break;
            case T_DIRECTION:
                  static_cast<QComboBox*>(w)->setCurrentIndex(val.toInt());
                  break;
            case T_BOOL:
                  static_cast<QCheckBox*>(w)->setChecked(val.toBool());
                  break;
            default:
                  abort();
            }
      }

//---------------------------------------------------------
//   isDefault
//---------------------------------------------------------

bool InspectorBase::isDefault(int idx)
      {
      Element* e = inspector->element();
      const InspectorItem& ii = item(idx);

      P_ID id      = ii.t;
      P_TYPE t     = propertyType(id);
      QVariant val = getValue(idx);
      QVariant def = e->propertyDefault(id);
      if (t == T_SIZE || t == T_SCALE) {
            QSizeF sz = def.toSizeF();
            qreal v = ii.sv == 0 ? sz.width() : sz.height();
            return val.toDouble() == v;
            }
      if (t == T_POINT) {
            QPointF sz = def.toPointF();
            qreal v = ii.sv == 0 ? sz.x() : sz.y();
            return val.toDouble() == v;
            }
      return val == def;
      }

//---------------------------------------------------------
//   dirty
//    return true if a property has changed
//---------------------------------------------------------

bool InspectorBase::dirty() const
      {
      Element* e = inspector->element();
      for (int i = 0; i < inspectorItems(); ++i) {
            P_ID id = item(i).t;
            if (e->getProperty(id) != getValue(i))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorBase::valueChanged(int idx)
      {
      const InspectorItem& ii = item(idx);
      if (ii.r)
            ii.r->setEnabled(!isDefault(idx));
//      inspector->enableApply(dirty());
      printf("valueChanged %d\n", idx);

      apply();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBase::setElement(Element* e)
      {
      for (int i = 0; i < inspectorItems(); ++i) {
            QWidget*     w = item(i).w;
            QToolButton* r = item(i).r;
            P_ID id        = item(i).t;
            P_TYPE pt      = propertyType(id);
            QVariant val;
            if (pt == T_SIZE || pt == T_SCALE) {
                  QSizeF sz = e->getProperty(id).toSizeF();
                  if (item(i).sv == 0)
                        val = QVariant(sz.width());
                  else
                        val = QVariant(sz.height());
                  }
            else if (pt == T_POINT) {
                  QPointF sz = e->getProperty(id).toPointF();
                  if (item(i).sv == 0)
                        val = QVariant(sz.x());
                  else
                        val = QVariant(sz.y());
                  }
            else
                  val = e->getProperty(id);

            w->blockSignals(true);
            setValue(i, val);
            w->blockSignals(false);

            if (r)
                  r->setEnabled(!isDefault(i));
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorBase::apply()
      {
      Element* e   = inspector->element();
      Score* score = e->score();

      score->startCmd();
      for (int i = 0; i < inspectorItems(); ++i) {
            P_ID id   = item(i).t;
            P_TYPE pt = propertyType(id);

            QVariant val1 = e->getProperty(id);
            if (pt == T_SIZE || pt == T_SCALE) {
                  qreal v = getValue(i).toDouble();
                  QSizeF sz = val1.toSizeF();
                  if (item(i).sv == 0) {
                        if (sz.width() != v)
                              score->undoChangeProperty(e, id, QVariant(QSizeF(v, sz.height())));
                        }
                  else {
                        if (sz.height() != v)
                              score->undoChangeProperty(e, id, QVariant(QSizeF(sz.width(), v)));
                        }
                  }
            else if (pt == T_POINT) {
                  qreal v = getValue(i).toDouble();
                  QPointF sz = val1.toPointF();
                  if (item(i).sv == 0) {
                        if (sz.x() != v)
                              score->undoChangeProperty(e, id, QVariant(QPointF(v, sz.y())));
                        }
                  else {
                        if (sz.y() != v)
                              score->undoChangeProperty(e, id, QVariant(QPointF(sz.x(), v)));
                        }
                  }
            else {
                  QVariant val2 = getValue(i);
                  if (val1 != val2)
                        score->undoChangeProperty(e, id, val2);
                  }
            }
      score->setLayoutAll(true);
      score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   resetClicked
//---------------------------------------------------------

void InspectorBase::resetClicked(int i)
      {
      Element* e   = inspector->element();
      P_ID id      = item(i).t;
      QVariant def = e->propertyDefault(id);
      QWidget* w   = item(i).w;

       switch (propertyType(id)) {
            case T_SREAL:
            case T_REAL:
                  static_cast<QDoubleSpinBox*>(w)->setValue(def.toDouble());
                  break;
            case T_DIRECTION:
                  static_cast<QComboBox*>(w)->setCurrentIndex(def.toInt());
                  break;
            case T_BOOL:
                  static_cast<QCheckBox*>(w)->setChecked(def.toBool());
                  break;
            default:
                  abort();
            }
      }

//---------------------------------------------------------
//   mapSignals
//---------------------------------------------------------

void InspectorBase::mapSignals()
      {
      for (int i = 0; i < inspectorItems(); ++i) {
            QToolButton* b = item(i).r;
            if (b) {
                  connect(b, SIGNAL(clicked()), resetMapper, SLOT(map()));
                  resetMapper->setMapping(b, i);
                  }
            QWidget* w = item(i).w;
            valueMapper->setMapping(w, i);
            switch (propertyType(item(i).t)) {
                  case T_REAL:
                  case T_SREAL:
                  case T_SIZE:
                  case T_POINT:
                  case T_SCALE:
                        connect(w, SIGNAL(valueChanged(double)), valueMapper, SLOT(map()));
                        break;
                  case T_DIRECTION:
                        connect(w, SIGNAL(currentIndexChanged(int)), valueMapper, SLOT(map()));
                        break;
                  case T_BOOL:
                        connect(w, SIGNAL(toggled(bool)), valueMapper, SLOT(map()));
                        break;
                  default:
                        abort();
                  }
            }
      connect(resetMapper, SIGNAL(mapped(int)), SLOT(resetClicked(int)));
      connect(valueMapper, SIGNAL(mapped(int)), SLOT(valueChanged(int)));
      }
//
//  dummy
//
const InspectorItem& InspectorBase::item(int idx) const
      {
      static InspectorItem item;
      return item;
      }

