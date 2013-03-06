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
      QWidget* w = item(idx).w;

      if (qobject_cast<QDoubleSpinBox*>(w))
            return w->property("value");
      else if (qobject_cast<QComboBox*>(w))
            return w->property("currentIndex");
      else if (qobject_cast<QCheckBox*>(w))
            return w->property("checked");
      else if (qobject_cast<QLineEdit*>(w))
            return w->property("text");
      else if (qobject_cast<Awl::ColorLabel*>(w))
            return static_cast<Awl::ColorLabel*>(w)->color();
      else  {
            qDebug("not supported widget");
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

      if (qobject_cast<QDoubleSpinBox*>(w))
            static_cast<QDoubleSpinBox*>(w)->setValue(val.toDouble());
      else if (qobject_cast<QComboBox*>(w))
            static_cast<QComboBox*>(w)->setCurrentIndex(val.toInt());
      else if (qobject_cast<QCheckBox*>(w))
            static_cast<QCheckBox*>(w)->setChecked(val.toBool());
      else if (qobject_cast<QLineEdit*>(w))
            static_cast<QLineEdit*>(w)->setText(val.toString());
      else if (qobject_cast<Awl::ColorLabel*>(w))
            static_cast<Awl::ColorLabel*>(w)->setColor(val.value<QColor>());
      else  {
            qDebug("not supported widget");
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
      if (ii.parent)
            e = e->parent();

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
            Element* ee = e;
            if (item(i).parent)
                  ee = ee->parent();
            if (ee->getProperty(id) != getValue(i))
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
      apply();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBase::setElement()
      {
      Element* ee = inspector->element();
      for (int i = 0; i < inspectorItems(); ++i) {
            QWidget* w = item(i).w;
            P_ID id    = item(i).t;
            P_TYPE pt  = propertyType(id);
            QVariant val;
            Element* e = ee;
            if (item(i).parent)
                  e = ee->parent();
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

            QToolButton* r = item(i).r;
            if (r)
                  r->setEnabled(!isDefault(i));
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorBase::apply()
      {
      Score* score = inspector->element()->score();

      score->startCmd();
      foreach(Element* ee, inspector->el()) {
            for (int i = 0; i < inspectorItems(); ++i) {
                  P_ID id   = item(i).t;
                  P_TYPE pt = propertyType(id);
                  Element* e = ee;
                  if (item(i).parent)
                        e = ee->parent();

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
      if (item(i).parent)
            e = e->parent();
      QVariant def = e->propertyDefault(id);
      QWidget* w   = item(i).w;

      if (qobject_cast<QDoubleSpinBox*>(w))
            static_cast<QDoubleSpinBox*>(w)->setValue(def.toDouble());
      else if (qobject_cast<QComboBox*>(w))
            static_cast<QComboBox*>(w)->setCurrentIndex(def.toInt());
      else if (qobject_cast<QCheckBox*>(w))
            static_cast<QCheckBox*>(w)->setChecked(def.toBool());
      else if (qobject_cast<QLineEdit*>(w))
            static_cast<QLineEdit*>(w)->setText(def.toString());
      else if (qobject_cast<Awl::ColorLabel*>(w)) {
            static_cast<Awl::ColorLabel*>(w)->setColor(def.value<QColor>());
            valueChanged(i);
            }
      else  {
            qDebug("not supported widget");
            abort();
            }
      }

//---------------------------------------------------------
//   mapSignals
//---------------------------------------------------------

void InspectorBase::mapSignals()
      {
      for (int i = 0; i < inspectorItems(); ++i) {
            QToolButton* resetButton = item(i).r;
            if (resetButton) {
                  connect(resetButton, SIGNAL(clicked()), resetMapper, SLOT(map()));
                  resetMapper->setMapping(resetButton, i);
                  }
            QWidget* w = item(i).w;
            valueMapper->setMapping(w, i);
            if (qobject_cast<QDoubleSpinBox*>(w))
                  connect(w, SIGNAL(valueChanged(double)), valueMapper, SLOT(map()));
            else if (qobject_cast<QComboBox*>(w))
                  connect(w, SIGNAL(currentIndexChanged(int)), valueMapper, SLOT(map()));
            else if (qobject_cast<QCheckBox*>(w))
                  connect(w, SIGNAL(toggled(bool)), valueMapper, SLOT(map()));
            else if (qobject_cast<QLineEdit*>(w))
                  connect(w, SIGNAL(textChanged(const QString&)), valueMapper, SLOT(map()));
            else if (qobject_cast<Awl::ColorLabel*>(w))
                  connect(w, SIGNAL(colorChanged(QColor)), valueMapper, SLOT(map()));
            else  {
                  qDebug("not supported widget");
                  abort();
                  }
            }
      connect(resetMapper, SIGNAL(mapped(int)), SLOT(resetClicked(int)));
      connect(valueMapper, SIGNAL(mapped(int)), SLOT(valueChanged(int)));
      }
