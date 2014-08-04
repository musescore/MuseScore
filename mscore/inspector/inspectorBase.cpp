//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011-2013 Werner Schweer and others
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

namespace Ms {

//---------------------------------------------------------
//   InspectorBase
//---------------------------------------------------------

InspectorBase::InspectorBase(QWidget* parent)
   : QWidget(parent)
      {
      resetMapper  = new QSignalMapper(this);
      valueMapper  = new QSignalMapper(this);
      blockSignals = false;

      inspector = static_cast<Inspector*>(parent);
      _layout    = new QVBoxLayout;
      _layout->setSpacing(0);
      _layout->setContentsMargins(0, 0, 0, 0);
      _layout->addStretch(100);
      setLayout(_layout);
      }

//---------------------------------------------------------
//   getValue
//    get value from gui element
//---------------------------------------------------------

QVariant InspectorBase::getValue(const InspectorItem& ii) const
      {
      QWidget* w = ii.w;

      QVariant v;
      if (qobject_cast<QDoubleSpinBox*>(w))
            v = w->property("value");
      else if (qobject_cast<QSpinBox*>(w))
            v = w->property("value");
      else if (qobject_cast<QComboBox*>(w)) {
            QComboBox* cb = qobject_cast<QComboBox*>(w);
            int val = cb->currentIndex();
            if (cb->itemData(val).isValid())
                  val = cb->itemData(val).toInt();
            v = val;
            }
      else if (qobject_cast<QCheckBox*>(w))
            v = w->property("checked");
      else if (qobject_cast<QLineEdit*>(w))
            v =  w->property("text");
      else if (qobject_cast<Awl::ColorLabel*>(w))
            v = static_cast<Awl::ColorLabel*>(w)->color();
      else
            qFatal("not supported widget %s", w->metaObject()->className());

      switch(propertyType(ii.t)) {
            case P_TYPE::POINT:
            case P_TYPE::SP_REAL:
                  v = v.toDouble() * inspector->element()->score()->spatium();
                  break;
            case P_TYPE::TEMPO:
                  v = v.toDouble() / 60.0;
                  break;
            case P_TYPE::POINT_MM:
            case P_TYPE::SIZE_MM:
                  v = v.toDouble() * MScore::DPMM;
                  break;
            default:
                  break;
            }
      return v;
      }

//---------------------------------------------------------
//   setValue
//    set gui element value
//---------------------------------------------------------

void InspectorBase::setValue(const InspectorItem& ii, QVariant val)
      {
      QWidget* w = ii.w;

      P_ID id  = ii.t;

      P_TYPE t = propertyType(id);
      if (t == P_TYPE::POINT || t == P_TYPE::SP_REAL)
            val = val.toDouble() / inspector->element()->score()->spatium();
      else if (t == P_TYPE::TEMPO)
            val = val.toDouble() * 60.0;
      else if (t == P_TYPE::POINT_MM)
            val = val.toDouble() / MScore::DPMM;
      else if (t == P_TYPE::SIZE_MM)
            val = val.toDouble() / MScore::DPMM;

      if (qobject_cast<QDoubleSpinBox*>(w))
            static_cast<QDoubleSpinBox*>(w)->setValue(val.toDouble());
      else if (qobject_cast<QSpinBox*>(w))
            static_cast<QSpinBox*>(w)->setValue(val.toInt());
      else if (qobject_cast<QComboBox*>(w)) {
            int ival = val.toInt();
            QComboBox* cb = qobject_cast<QComboBox*>(w);
            if (cb->itemData(0).isValid()) {
                  for (int i = 0; i < cb->count(); ++i) {
                        if (cb->itemData(i).toInt() == ival) {
                              ival = i;
                              break;
                              }
                        }
                  }
            cb->setCurrentIndex(ival);
            }
      else if (qobject_cast<QCheckBox*>(w))
            static_cast<QCheckBox*>(w)->setChecked(val.toBool());
      else if (qobject_cast<QLineEdit*>(w))
            static_cast<QLineEdit*>(w)->setText(val.toString());
      else if (qobject_cast<Awl::ColorLabel*>(w))
            static_cast<Awl::ColorLabel*>(w)->setColor(val.value<QColor>());
      else
            qFatal("not supported widget %s", w->metaObject()->className());
      }

//---------------------------------------------------------
//   isDefault
//---------------------------------------------------------

bool InspectorBase::isDefault(const InspectorItem& ii)
      {
      Element* e = inspector->element();
      for (int i = 0; i < ii.parent; ++i)
            e = e->parent();

      P_ID id      = ii.t;
      P_TYPE t     = propertyType(id);
      QVariant val = getValue(ii);
      QVariant def = e->propertyDefault(id);
      if (t == P_TYPE::SIZE || t == P_TYPE::SCALE || t == P_TYPE::SIZE_MM) {
            QSizeF sz = def.toSizeF();
            qreal v = ii.sv == 0 ? sz.width() : sz.height();
            return val.toDouble() == v;
            }
      if (t == P_TYPE::POINT || t == P_TYPE::POINT_MM) {
            QPointF sz = def.toPointF();
            qreal v = ii.sv == 0 ? sz.x() : sz.y();
            return val.toDouble() == v;
            }
      if (t == P_TYPE::FRACTION) {
            Fraction f = def.value<Fraction>();
            int v = ii.sv == 0 ? f.numerator() : f.denominator();
            return val.toInt() == v;
            }
      return val == def;
      }

//---------------------------------------------------------
//   dirty
//    return true if a property has changed
//---------------------------------------------------------

bool InspectorBase::dirty() const
      {
      foreach(const InspectorItem& ii, iList) {
            Element* e = inspector->element();
            for (int i = 0; i < ii.parent; ++i)
                  e = e->parent();
            if (e->getProperty(ii.t) != getValue(ii))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBase::setElement()
      {
      foreach (const InspectorItem& ii, iList) {
            P_ID id    = ii.t;
            P_TYPE pt  = propertyType(id);

            Element* e = inspector->element();
            for (int k = 0; k < ii.parent; ++k)
                  e = e->parent();
            QVariant val = e->getProperty(id);
            if (pt == P_TYPE::SIZE || pt == P_TYPE::SCALE || pt == P_TYPE::SIZE_MM) {
                  QSizeF sz = val.toSizeF();
                  if (ii.sv == 0)
                        val = QVariant(sz.width());
                  else
                        val = QVariant(sz.height());
                  }
            else if (pt == P_TYPE::POINT || pt == P_TYPE::POINT_MM) {
                  QPointF sz = val.toPointF();
                  if (ii.sv == 0)
                        val = QVariant(sz.x());
                  else
                        val = QVariant(sz.y());
                  }
            else if (pt == P_TYPE::FRACTION) {
                  Fraction f = val.value<Fraction>();
                  if (ii.sv == 0)
                        val = QVariant(f.numerator());
                  else
                        val = QVariant(f.denominator());
                  }

            blockSignals = true;
            setValue(ii, val);
            blockSignals = false;
            checkDifferentValues(ii);
            }
      postInit();
      }

//---------------------------------------------------------
//   checkDifferentValues
//    enable/disable reset button
//    enable/disable value widget for multi selection
//---------------------------------------------------------

void InspectorBase::checkDifferentValues(const InspectorItem& ii)
      {
      bool valuesAreDifferent = false;
      if (inspector->el().size() > 1) {
            P_ID id      = ii.t;
            P_TYPE pt    = propertyType(id);
            QVariant val = getValue(ii);

            foreach(Element* e, inspector->el()) {
                  for (int k = 0; k < ii.parent; ++k)
                        e = e->parent();
                  if (pt == P_TYPE::SIZE || pt == P_TYPE::SCALE || pt == P_TYPE::SIZE_MM) {
                        QSizeF sz = e->getProperty(id).toSizeF();
                        if (ii.sv == 0)
                              valuesAreDifferent = sz.width() != val.toDouble();
                        else
                              valuesAreDifferent = sz.height() != val.toDouble();
                        }
                  else if (pt == P_TYPE::POINT || pt == P_TYPE::POINT_MM) {
                        QPointF sz = e->getProperty(id).toPointF();
                        if (ii.sv == 0)
                              valuesAreDifferent = sz.x() != val.toDouble();
                        else
                              valuesAreDifferent = sz.y() != val.toDouble();
                        }
                  else if (pt == P_TYPE::FRACTION) {
                        Fraction f = e->getProperty(id).value<Fraction>();
                        if (ii.sv == 0)
                              valuesAreDifferent = f.numerator() != val.toInt();
                        else
                              valuesAreDifferent = f.denominator() != val.toInt();
                        }
                  else
                        valuesAreDifferent = e->getProperty(id) != val;
                  if (valuesAreDifferent)
                        break;
                  }
            ii.w->setStyleSheet( valuesAreDifferent ? QString("* { color: %1 }").arg(MScore::selectColor[0].name()) : "");
            }
      
      //deal with reset if only one element, or if values are the same
      if (!valuesAreDifferent){
            PropertyStyle styledValue = inspector->el().front()->propertyStyle(ii.t);
            bool reset;
            if (styledValue == PropertyStyle::STYLED) {
                  // does not work for QComboBox:
                  ii.w->setStyleSheet("* { color: gray }");
                  reset = false;
                  }
            else if (styledValue == PropertyStyle::UNSTYLED) {
                  ii.w->setStyleSheet("");
                  reset = true;
                  }
            else
                  reset = !isDefault(ii);
            if (ii.r)
                  ii.r->setEnabled(reset);
            }
      else {
            if (ii.r)
                  ii.r->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorBase::valueChanged(int idx, bool reset)
      {
      if (blockSignals)
            return;

      const InspectorItem& ii = iList[idx];
      P_ID id       = ii.t;
      P_TYPE pt     = propertyType(id);
      QVariant val2 = getValue(ii);
      Score* score  = inspector->element()->score();

      score->startCmd();
      foreach (Element* e, inspector->el()) {
            for (int i = 0; i < ii.parent; ++i)
                  e = e->parent();

            // reset sets property style UNSTYLED to STYLED

            PropertyStyle ps = e->propertyStyle(id);
            if (reset && ps == PropertyStyle::UNSTYLED)
                  ps = PropertyStyle::STYLED;
            else if (ps == PropertyStyle::STYLED)
                  ps = PropertyStyle::UNSTYLED;

            QVariant val1 = e->getProperty(id);
            if (pt == P_TYPE::SIZE || pt == P_TYPE::SCALE || pt == P_TYPE::SIZE_MM) {
                  qreal v   = val2.toDouble();
                  QSizeF sz = val1.toSizeF();
                  if (ii.sv == 0) {
                        if (sz.width() != v)
                              score->undoChangeProperty(e, id, QVariant(QSizeF(v, sz.height())), ps);
                        }
                  else {
                        if (sz.height() != v)
                              score->undoChangeProperty(e, id, QVariant(QSizeF(sz.width(), v)), ps);
                        }
                  }
            else if (pt == P_TYPE::POINT || pt == P_TYPE::POINT_MM) {
                  qreal v    = val2.toDouble();
                  QPointF sz = val1.toPointF();
                  if (ii.sv == 0) {
                        if (sz.x() != v)
                              score->undoChangeProperty(e, id, QVariant(QPointF(v, sz.y())), ps);
                        }
                  else {
                        if (sz.y() != v)
                              score->undoChangeProperty(e, id, QVariant(QPointF(sz.x(), v)), ps);
                        }
                  }
            else if (pt == P_TYPE::FRACTION) {
                  int v      = val2.toInt();
                  Fraction f = val1.value<Fraction>();
                  if (ii.sv == 0) {
                        if (f.numerator() != v) {
                              QVariant va;
                              va.setValue(Fraction(v, f.denominator()));
                              score->undoChangeProperty(e, id, va, ps);
                              }
                        }
                  else {
                        if (f.denominator() != v) {
                              QVariant va;
                              va.setValue(Fraction(f.numerator(), v));
                              score->undoChangeProperty(e, id, va, ps);
                              }
                        }
                  }
            else {
                  if (val1 != val2 || (reset && ps != PropertyStyle::NOSTYLE))
                        score->undoChangeProperty(e, id, val2, ps);
                  }
            }
      inspector->setInspectorEdit(true);
      checkDifferentValues(ii);
      score->endCmd();
      mscore->endCmd();
      inspector->setInspectorEdit(false);
      postInit();
      }

//---------------------------------------------------------
//   resetClicked
//---------------------------------------------------------

void InspectorBase::resetClicked(int i)
      {
      Element* e   = inspector->element();
      const InspectorItem& ii = iList[i];
      P_ID id      = ii.t;
      for (int i = 0; i < ii.parent; ++i)
            e = e->parent();
      QVariant def = e->propertyDefault(id);
      if (!def.isValid())
            return;
      QWidget* w   = ii.w;

      blockSignals = true;
      if (qobject_cast<QDoubleSpinBox*>(w))
            static_cast<QDoubleSpinBox*>(w)->setValue(def.toDouble());
      else if (qobject_cast<QSpinBox*>(w))
            static_cast<QSpinBox*>(w)->setValue(def.toInt());
      else if (qobject_cast<QComboBox*>(w)) {
            int ival = def.toInt();
            QComboBox* cb = qobject_cast<QComboBox*>(w);
            if (cb->itemData(0).isValid()) {
                  for (int i = 0; i < cb->count(); ++i) {
                        if (cb->itemData(i).toInt() == ival) {
                              ival = i;
                              break;
                              }
                        }
                  }
            cb->setCurrentIndex(ival);
            }
      else if (qobject_cast<QCheckBox*>(w))
            static_cast<QCheckBox*>(w)->setChecked(def.toBool());
      else if (qobject_cast<QLineEdit*>(w))
            static_cast<QLineEdit*>(w)->setText(def.toString());
      else if (qobject_cast<Awl::ColorLabel*>(w))
            static_cast<Awl::ColorLabel*>(w)->setColor(def.value<QColor>());
      else
            qFatal("not supported widget %s", w->metaObject()->className());
      blockSignals = false;

      valueChanged(i, true);
      }

//---------------------------------------------------------
//   mapSignals
//    initialize inspector panel
//---------------------------------------------------------

void InspectorBase::mapSignals()
      {
      int i = 0;
      foreach (const InspectorItem& ii, iList) {
            QToolButton* resetButton = ii.r;
            if (resetButton) {
                  connect(resetButton, SIGNAL(clicked()), resetMapper, SLOT(map()));
                  resetMapper->setMapping(resetButton, i);
                  }
            QWidget* w = ii.w;
            valueMapper->setMapping(w, i);
            if (qobject_cast<QDoubleSpinBox*>(w))
                  connect(w, SIGNAL(valueChanged(double)), valueMapper, SLOT(map()));
            else if (qobject_cast<QSpinBox*>(w))
                  connect(w, SIGNAL(valueChanged(int)), valueMapper, SLOT(map()));
            else if (qobject_cast<QComboBox*>(w))
                  connect(w, SIGNAL(currentIndexChanged(int)), valueMapper, SLOT(map()));
            else if (qobject_cast<QCheckBox*>(w))
                  connect(w, SIGNAL(toggled(bool)), valueMapper, SLOT(map()));
            else if (qobject_cast<QLineEdit*>(w))
                  connect(w, SIGNAL(textChanged(const QString&)), valueMapper, SLOT(map()));
            else if (qobject_cast<Awl::ColorLabel*>(w))
                  connect(w, SIGNAL(colorChanged(QColor)), valueMapper, SLOT(map()));
            else
                  qFatal("not supported widget %s", w->metaObject()->className());
            ++i;
            }
      connect(resetMapper, SIGNAL(mapped(int)), SLOT(resetClicked(int)));
      connect(valueMapper, SIGNAL(mapped(int)), SLOT(valueChanged(int)));
      }

//---------------------------------------------------------
//   addWidget
//---------------------------------------------------------

QWidget* InspectorBase::addWidget()
      {
      QWidget* w = new QWidget;
      _layout->insertWidget(_layout->count()-1, w);
      _layout->insertSpacing(_layout->count()-1, 20);
      return w;
      }

//---------------------------------------------------------
//   setupLineStyle
//---------------------------------------------------------

void InspectorBase::setupLineStyle(QComboBox* cb)
      {
      cb->setItemData(0, int(Qt::SolidLine));
      cb->setItemData(1, int(Qt::DashLine));
      cb->setItemData(2, int(Qt::DotLine));
      cb->setItemData(3, int(Qt::DashDotLine));
      cb->setItemData(4, int(Qt::DashDotDotLine));
      }
}

