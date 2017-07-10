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
#include "libmscore/undo.h"
#include "musescore.h"
#include "inspectorBase.h"
#include "inspector.h"
#include "icons.h"
#include "preferences.h"
#include "offsetSelect.h"
#include "scaleSelect.h"
#include "sizeSelect.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorBase
//---------------------------------------------------------

InspectorBase::InspectorBase(QWidget* parent)
   : QWidget(parent)
      {
      setAccessibleName(tr("Inspector"));
      resetMapper  = new QSignalMapper(this);
      valueMapper  = new QSignalMapper(this);
      styleMapper  = new QSignalMapper(this);

      inspector = static_cast<Inspector*>(parent);
      _layout    = new QVBoxLayout(this);
      _layout->setSpacing(0);
      _layout->setContentsMargins(0, 10, 0, 0);
      _layout->addStretch(100);

      connect(resetMapper, SIGNAL(mapped(int)), SLOT(resetClicked(int)));
      connect(valueMapper, SIGNAL(mapped(int)), SLOT(valueChanged(int)));
      connect(styleMapper, SIGNAL(mapped(int)), SLOT(setStyleClicked(int)));
      }

//---------------------------------------------------------
//   getValue
//    get value from gui element
//---------------------------------------------------------

QVariant InspectorBase::getValue(const InspectorItem& ii) const
      {
      QWidget* w = ii.w;

      QVariant v;
      if (qobject_cast<QDoubleSpinBox*>(w) || qobject_cast<QSpinBox*>(w))
            v = w->property("value");
      else if (qobject_cast<QFontComboBox*>(w))
            v = static_cast<QFontComboBox*>(w)->currentFont().family();
      else if (qobject_cast<QComboBox*>(w)) {
            QComboBox* cb = qobject_cast<QComboBox*>(w);
            int val = cb->currentIndex();
            if (cb->itemData(val).isValid())
                  val = cb->itemData(val).toInt();
            v = val;
            }
      else if (qobject_cast<QCheckBox*>(w) || qobject_cast<QPushButton*>(w) || qobject_cast<QToolButton*>(w))
            v = w->property("checked");
      else if (qobject_cast<QLineEdit*>(w))
            v =  w->property("text");
      else if (qobject_cast<Awl::ColorLabel*>(w))
            v = static_cast<Awl::ColorLabel*>(w)->color();
      else if (qobject_cast<Ms::AlignSelect*>(w))
            v = int(static_cast<Ms::AlignSelect*>(w)->align());
      else if (qobject_cast<Ms::OffsetSelect*>(w))
            v = static_cast<Ms::OffsetSelect*>(w)->offset();
      else if (qobject_cast<Ms::ScaleSelect*>(w))
            v = static_cast<Ms::ScaleSelect*>(w)->scale();
      else if (qobject_cast<Ms::SizeSelect*>(w))
            v = static_cast<Ms::SizeSelect*>(w)->value();
      else
            qFatal("not supported widget %s", w->metaObject()->className());

      switch (propertyType(ii.t)) {
            case P_TYPE::POINT_SP:
                  v = v.toPointF() * inspector->element()->score()->spatium();
                  break;
            case P_TYPE::SP_REAL:
                  v = v.toDouble() * inspector->element()->score()->spatium();
                  break;
            case P_TYPE::TEMPO:
                  v = v.toDouble() / 60.0;
                  break;
            case P_TYPE::ZERO_INT:
                  v = v.toInt() - 1;
                  break;
            case P_TYPE::POINT_MM:
                  v = v.toPointF() * DPMM;
                  break;
            case P_TYPE::SIZE_MM:
                  v = v.toSizeF() * DPMM;
                  break;
            case P_TYPE::BARLINE_TYPE:
                  v = QVariant::fromValue(BarLineType(v.toInt()));
                  break;
            case P_TYPE::DIRECTION:
                  v = QVariant::fromValue(Direction(v.toInt()));
                  break;
            case P_TYPE::INT_LIST: {
                  QStringList sl = v.toString().split(",", QString::SkipEmptyParts);
                  QList<int> il;
                  for (const QString& l : sl) {
                        int i = l.simplified().toInt();
                        il.append(i);
                        }
                  v = QVariant::fromValue(il);
                  }
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

      switch (propertyType(id)) {
            case P_TYPE::POINT_SP:
                  val = val.toPointF() / inspector->element()->score()->spatium();
                  break;
            case P_TYPE::SP_REAL:
                  val = val.toDouble() / inspector->element()->score()->spatium();
                  break;
            case P_TYPE::TEMPO:
                  val = val.toDouble() * 60.0;
                  break;
            case P_TYPE::ZERO_INT:
                  val = val.toInt() + 1;
                  break;
            case P_TYPE::POINT_MM:
                  val = val.toPointF() / DPMM;
                  break;
            case P_TYPE::SIZE_MM:
                  val = val.toSizeF() / DPMM;
                  break;
            case P_TYPE::DIRECTION:
                  val = int(val.value<Direction>());
                  break;
            case P_TYPE::INT_LIST: {
                  QString s;
                  QList<int> il = val.value<QList<int>>();
                  for (int i : il) {
                        if (!s.isEmpty())
                              s += ", ";
                        s += QString("%1").arg(i);
                        }
                  val = s;
                  }
                  break;

            default:
                  break;
            }
      if (qobject_cast<QDoubleSpinBox*>(w))
            static_cast<QDoubleSpinBox*>(w)->setValue(val.toDouble());
      else if (qobject_cast<QSpinBox*>(w))
            static_cast<QSpinBox*>(w)->setValue(val.toInt());
      else if (qobject_cast<QFontComboBox*>(w))
            static_cast<QFontComboBox*>(w)->setCurrentFont(QFont(val.toString()));
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
      else if (qobject_cast<QPushButton*>(w))
            static_cast<QPushButton*>(w)->setChecked(val.toBool());
      else if (qobject_cast<QToolButton*>(w))
            static_cast<QToolButton*>(w)->setChecked(val.toBool());
      else if (qobject_cast<QLineEdit*>(w))
            static_cast<QLineEdit*>(w)->setText(val.toString());
      else if (qobject_cast<Awl::ColorLabel*>(w))
            static_cast<Awl::ColorLabel*>(w)->setColor(val.value<QColor>());
      else if (qobject_cast<Ms::AlignSelect*>(w))
            static_cast<Ms::AlignSelect*>(w)->setAlign(Align(val.toInt()));
      else if (qobject_cast<Ms::OffsetSelect*>(w))
            static_cast<Ms::OffsetSelect*>(w)->setOffset(val.toPointF());
      else if (qobject_cast<Ms::ScaleSelect*>(w))
            static_cast<Ms::ScaleSelect*>(w)->setScale(val.toSizeF());
      else if (qobject_cast<Ms::SizeSelect*>(w))
            static_cast<Ms::SizeSelect*>(w)->setValue(val);
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
      QVariant val = e->getProperty(id);
      QVariant def = e->propertyDefault(id);
      return val == def;
      }

//---------------------------------------------------------
//   compareValues
//---------------------------------------------------------

bool InspectorBase::compareValues(const InspectorItem& ii, QVariant a, QVariant b)
      {
      P_ID id  = ii.t;
      P_TYPE t = propertyType(id);
      if (t == P_TYPE::SIZE) {
            QSizeF s1 = a.toSizeF();
            QSizeF s2 = b.toSizeF();
            bool a = qFuzzyCompare(s1.width(), s2.width()) && qFuzzyCompare(s1.height(), s2.height());
            printf("%d %f %f -- %f %f\n", a, s1.width(), s2.width(), s1.height(), s2.height());
            return a;
            }
      return b == a;
      }

//---------------------------------------------------------
//   dirty
//    return true if a property has changed
//---------------------------------------------------------

bool InspectorBase::dirty() const
      {
      for (const InspectorItem& ii : iList) {
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
      for (const InspectorItem& ii : iList) {
            P_ID id    = ii.t;
            Element* e = inspector->element();
            for (int k = 0; k < ii.parent; ++k)
                  e = e->parent();
            QVariant val = e->getProperty(id);
            if (ii.w) {
                  ii.w->blockSignals(true);
                  setValue(ii, val);
                  ii.w->blockSignals(false);
                  }
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
      QColor c(preferences.isThemeDark() ? Qt::yellow : Qt::darkCyan);

      if (inspector->el()->size() > 1) {
            P_ID id      = ii.t;
//            P_TYPE pt    = propertyType(id);
            QVariant val = getValue(ii);

            for (Element* e : *inspector->el()) {
                  for (int k = 0; k < ii.parent; ++k)
                        e = e->parent();

                  valuesAreDifferent = !compareValues(ii, e->getProperty(id), val);
                  if (valuesAreDifferent)
                        break;
                  }
            ii.w->setStyleSheet(valuesAreDifferent ? QString("* { color: %1 }").arg(c.name()) : "");
            }

      //deal with reset if only one element, or if values are the same
      bool enableReset = true;
      if (!valuesAreDifferent) {
            PropertyFlags styledValue = inspector->el()->front()->propertyFlags(ii.t);
            switch (styledValue) {
                  case PropertyFlags::STYLED:
                        ii.w->setStyleSheet(QString("* { color: %1 }").arg(c.name()));
                        enableReset = false;
                        break;
                  case PropertyFlags::UNSTYLED:
                        ii.w->setStyleSheet("");
                        enableReset = true;
                        break;
                  case PropertyFlags::NOSTYLE:
                        enableReset = !isDefault(ii);
                        ii.w->setStyleSheet("");
                        break;
                  }
            }
      if (ii.r)
            ii.r->setEnabled(enableReset);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorBase::valueChanged(int idx, bool reset)
      {
      const InspectorItem& ii = iList[idx];
      P_ID id       = ii.t;
      QVariant val2 = getValue(ii);

      Score* score  = inspector->element()->score();

      score->startCmd();
      for (Element* e : *inspector->el()) {
            for (int i = 0; i < ii.parent; ++i)
                  e = e->parent();

            // reset sets property style UNSTYLED to STYLED

            PropertyFlags ps = e->propertyFlags(id);
            if (reset && ps == PropertyFlags::UNSTYLED)
                  ps = PropertyFlags::STYLED;
            else if (ps == PropertyFlags::STYLED)
                  ps = PropertyFlags::UNSTYLED;
            QVariant val1 = e->getProperty(id);
            if (val1 != val2 || (reset && ps != PropertyFlags::NOSTYLE))
                  e->undoChangeProperty(id, val2, ps);
            }
      inspector->setInspectorEdit(true);
      checkDifferentValues(ii);
      score->endCmd();
      inspector->setInspectorEdit(false);
      postInit();

      // a subStyle change may change several other values:
      if (id == P_ID::SUB_STYLE)
            setElement();
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

      Score* s = e->score();
      s->startCmd();
      e->undoResetProperty(id);
      inspector->setInspectorEdit(true);
      s->endCmd();      // this may remove element
      inspector->setInspectorEdit(false);
      inspector->update(s);
      }

//---------------------------------------------------------
//   setStyleClicked
//---------------------------------------------------------

void InspectorBase::setStyleClicked(int i)
      {
      Element* e   = inspector->element();
      const InspectorItem& ii = iList[i];

      StyleIdx sidx = e->getPropertyStyle(ii.t);
      if (sidx == StyleIdx::NOSTYLE)
            return;
      e->score()->startCmd();
      QVariant val = getValue(ii);
      e->undoChangeProperty(ii.t, val, PropertyFlags::STYLED);
      P_ID id      = ii.t;
      P_TYPE t     = propertyType(id);
      if (t == P_TYPE::SP_REAL)
            val = val.toDouble() / e->score()->spatium();
      e->score()->undo(new ChangeStyleVal(e->score(), sidx, val));
      checkDifferentValues(ii);
      e->score()->endCmd();
      }

//---------------------------------------------------------
//   mapSignals
//    initialize inspector panel
//---------------------------------------------------------

void InspectorBase::mapSignals(const std::vector<InspectorItem>& il, const std::vector<InspectorPanel>& pl)
      {
      for (auto& p : pl)
            pList.push_back(p);
      for (auto& p : pList) {
            QToolButton* title = p.title;
            QWidget* panel = p.panel;
            if (title) {
                  title->setCheckable(true);
                  title->setFocusPolicy(Qt::NoFocus);
                  connect(title, &QToolButton::clicked, this, [title, panel] (bool visible) {
                        if (panel)
                              panel->setVisible(visible);
                        if (title) {
                              title->setChecked(visible);
                              title->setArrowType(visible ? Qt::DownArrow : Qt::RightArrow);
                              QString key = title->parent()->objectName();
                              QSettings s;
                              s.setValue(QString("inspector/%1_visible").arg(key), visible);
                              }});
                  title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                  title->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
                  QSettings s;
                  QString key = title->parent()->objectName();
                  bool visible = s.value(QString("inspector/%1_visible").arg(key), true).toBool();
                  title->setArrowType(visible ? Qt::DownArrow : Qt::RightArrow);
                  title->setChecked(visible);
                  if (panel)
                        panel->setVisible(visible);
                  }
            }
      for (auto& i : il)
            iList.push_back(i);
      int i = 0;
      for (const InspectorItem& ii : iList) {
            QToolButton* resetButton = ii.r;
            if (resetButton) {
                  resetButton->setIcon(*icons[int(Icons::reset_ICON)]);
                  connect(resetButton, SIGNAL(clicked()), resetMapper, SLOT(map()));

                  resetMapper->setMapping(resetButton, i);
                  StyleIdx sidx = inspector->element()->getPropertyStyle(ii.t);
                  if (sidx != StyleIdx::NOSTYLE) {
                        QMenu* menu = new QMenu(this);
                        resetButton->setMenu(menu);
                        resetButton->setPopupMode(QToolButton::MenuButtonPopup);
                        QAction* a = menu->addAction(tr("Set as style"));
                        styleMapper->setMapping(a, i);
                        connect(a, SIGNAL(triggered()), styleMapper, SLOT(map()));
                        }
                  }
            QWidget* w = ii.w;
            if (!w)
                  continue;
            valueMapper->setMapping(w, i);
            if (qobject_cast<QDoubleSpinBox*>(w))
                  connect(w, SIGNAL(valueChanged(double)), valueMapper, SLOT(map()));
            else if (qobject_cast<QSpinBox*>(w))
                  connect(w, SIGNAL(valueChanged(int)), valueMapper, SLOT(map()));
            else if (qobject_cast<QFontComboBox*>(w))
                  connect(w, SIGNAL(currentFontChanged(const QFont&)), valueMapper, SLOT(map()));
            else if (qobject_cast<QComboBox*>(w))
                  connect(w, SIGNAL(currentIndexChanged(int)), valueMapper, SLOT(map()));
            else if (qobject_cast<QCheckBox*>(w) || qobject_cast<QPushButton*>(w) || qobject_cast<QToolButton*>(w))
                  connect(w, SIGNAL(toggled(bool)), valueMapper, SLOT(map()));
            else if (qobject_cast<QLineEdit*>(w))
                  connect(w, SIGNAL(textChanged(const QString&)), valueMapper, SLOT(map()));
            else if (qobject_cast<Awl::ColorLabel*>(w))
                  connect(w, SIGNAL(colorChanged(QColor)), valueMapper, SLOT(map()));
            else if (qobject_cast<Ms::AlignSelect*>(w))
                  connect(w, SIGNAL(alignChanged(Align)), valueMapper, SLOT(map()));
            else if (qobject_cast<Ms::OffsetSelect*>(w))
                  connect(w, SIGNAL(offsetChanged(const QPointF&)), valueMapper, SLOT(map()));
            else if (qobject_cast<Ms::ScaleSelect*>(w))
                  connect(w, SIGNAL(scaleChanged(const QSizeF&)), valueMapper, SLOT(map()));
            else if (qobject_cast<Ms::SizeSelect*>(w))
                  connect(w, SIGNAL(valueChanged(const QVariant&)), valueMapper, SLOT(map()));
            else
                  qFatal("not supported widget %s", w->metaObject()->className());
            ++i;
            }
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorBase::valueChanged(int idx)
      {
      valueChanged(idx, false);
      }

//---------------------------------------------------------
//   addWidget
//---------------------------------------------------------

QWidget* InspectorBase::addWidget()
      {
      QWidget* w = new QWidget;
      _layout->insertWidget(_layout->count()-1, w);
      _layout->insertSpacing(_layout->count()-1, 5);
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
      cb->setItemData(5, int(Qt::CustomDashLine));
      }

//---------------------------------------------------------
//   resetToStyle
//---------------------------------------------------------

void InspectorBase::resetToStyle()
      {
      Score* score = inspector->element()->score();
      score->startCmd();
      for (Element* e : *inspector->el()) {     // TODO: ??
            Text* text = toText(e);
            // Preserve <sym> tags
            text->undoChangeProperty(P_ID::TEXT, text->plainText().toHtmlEscaped().replace("&lt;sym&gt;","<sym>").replace("&lt;/sym&gt;","</sym>"));
            }
      score->endCmd();
      }

}

