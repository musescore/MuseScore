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

//---------------------------------------------------------
//   InspectorBase
//---------------------------------------------------------

InspectorBase::InspectorBase(QWidget* parent)
   : QWidget(parent)
      {
      resetMapper = new QSignalMapper(this);
      valueMapper = new QSignalMapper(this);

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

      if (qobject_cast<QDoubleSpinBox*>(w))
            return w->property("value");
      else if (qobject_cast<QSpinBox*>(w))
            return w->property("value");
      else if (qobject_cast<QComboBox*>(w)) {
            QComboBox* cb = qobject_cast<QComboBox*>(w);
            int val = cb->currentIndex();
            if (cb->itemData(val).isValid())
                  val = cb->itemData(val).toInt();
            return val;
            }
      else if (qobject_cast<QCheckBox*>(w))
            return w->property("checked");
      else if (qobject_cast<QLineEdit*>(w))
            return w->property("text");
      else if (qobject_cast<Awl::ColorLabel*>(w))
            return static_cast<Awl::ColorLabel*>(w)->color();
      else
            qFatal("not supported widget %s", w->metaObject()->className());
      return QVariant();
      }

//---------------------------------------------------------
//   setValue
//    set gui element value
//---------------------------------------------------------

void InspectorBase::setValue(const InspectorItem& ii, const QVariant& val)
      {
      QWidget* w = ii.w;

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
            QWidget* w = ii.w;
            P_ID id    = ii.t;
            P_TYPE pt  = propertyType(id);

            Element* e = inspector->element();
            for (int k = 0; k < ii.parent; ++k)
                  e = e->parent();

            QVariant val = e->getProperty(id);
            if (pt == T_SIZE || pt == T_SCALE) {
                  QSizeF sz = val.toSizeF();
                  if (ii.sv == 0)
                        val = QVariant(sz.width());
                  else
                        val = QVariant(sz.height());
                  }
            else if (pt == T_POINT) {
                  QPointF sz = val.toPointF();
                  if (ii.sv == 0)
                        val = QVariant(sz.x());
                  else
                        val = QVariant(sz.y());
                  }

            w->blockSignals(true);
            setValue(ii, val);
            w->blockSignals(false);

            bool valuesAreDifferent = false;
            if (inspector->el().size() > 1) {
                  foreach(Element* e, inspector->el()) {
                        for (int k = 0; k < ii.parent; ++k)
                              e = e->parent();
                        if (pt == T_SIZE || pt == T_SCALE) {
                              QSizeF sz = e->getProperty(id).toSizeF();
                              if (ii.sv == 0)
                                    valuesAreDifferent = sz.width() != val.toDouble();
                              else
                                    valuesAreDifferent = sz.height() != val.toDouble();
                              }
                        else if (pt == T_POINT) {
                              QPointF sz = e->getProperty(id).toPointF();
                              if (ii.sv == 0)
                                    valuesAreDifferent = sz.x() != val.toDouble();
                              else
                                    valuesAreDifferent = sz.y() != val.toDouble();
                              }
                        else
                              valuesAreDifferent = e->getProperty(id) != val;
                        if (valuesAreDifferent)
                              break;
                        }
                  }
            w->setEnabled(!valuesAreDifferent);

            if (ii.r)
                  ii.r->setEnabled(!isDefault(ii) || valuesAreDifferent);
            }
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorBase::valueChanged(int idx)
      {
      const InspectorItem& ii = iList[idx];
      if (ii.r)
            ii.r->setEnabled(!isDefault(ii));

      P_ID id       = ii.t;
      P_TYPE pt     = propertyType(id);
      QVariant val2 = getValue(ii);
      Score* score  = inspector->element()->score();

      score->startCmd();
      foreach (Element* e, inspector->el()) {
            for (int i = 0; i < ii.parent; ++i)
                  e = e->parent();

            QVariant val1 = e->getProperty(id);
            if (pt == T_SIZE || pt == T_SCALE) {
                  qreal v   = val2.toDouble();
                  QSizeF sz = val1.toSizeF();
                  if (ii.sv == 0) {
                        if (sz.width() != v)
                              score->undoChangeProperty(e, id, QVariant(QSizeF(v, sz.height())));
                        }
                  else {
                        if (sz.height() != v)
                              score->undoChangeProperty(e, id, QVariant(QSizeF(sz.width(), v)));
                        }
                  }
            else if (pt == T_POINT) {
                  qreal v    = val2.toDouble();
                  QPointF sz = val1.toPointF();
                  if (ii.sv == 0) {
                        if (sz.x() != v)
                              score->undoChangeProperty(e, id, QVariant(QPointF(v, sz.y())));
                        }
                  else {
                        if (sz.y() != v)
                              score->undoChangeProperty(e, id, QVariant(QPointF(sz.x(), v)));
                        }
                  }
            else {
                  if (val1 != val2)
                        score->undoChangeProperty(e, id, val2);
                  }
            }
      score->setLayoutAll(true);    // ?
      score->endCmd();
      mscore->endCmd();
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
      QWidget* w   = ii.w;

      if (qobject_cast<QDoubleSpinBox*>(w))
            static_cast<QDoubleSpinBox*>(w)->setValue(def.toDouble());
      else if (qobject_cast<QSpinBox*>(w))
            static_cast<QSpinBox*>(w)->setValue(def.toInt());
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
      else
            qFatal("not supported widget %s", w->metaObject()->className());
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
