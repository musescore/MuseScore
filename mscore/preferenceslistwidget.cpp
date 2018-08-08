//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2017 Werner Schweer and others
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

#include "preferenceslistwidget.h"

namespace Ms {


PreferencesListWidget::PreferencesListWidget(QWidget* parent)
      : QTreeWidget(parent)
      {
      setRootIsDecorated(false);
      setHeaderLabels(QStringList() << tr("Preference") << tr("Value"));
      header()->setStretchLastSection(false);
      header()->setSectionResizeMode(0, QHeaderView::Stretch);
      setAccessibleName(tr("Advanced preferences"));
      setAccessibleDescription(tr("Access to more advanced preferences"));
      setAlternatingRowColors(true);
      setSortingEnabled(true);
      sortByColumn(0, Qt::AscendingOrder);
      setAllColumnsShowFocus(true);
      }

void PreferencesListWidget::loadPreferences()
      {
      for (QString key : preferences.allPreferences().keys()) {
            Preference* pref = preferences.allPreferences().value(key);

            if (pref->showInAdvancedList()) {
                  // multiple dispatch using Visitor pattern, see overloaded visit() methods
                  pref->accept(key, *this);
                  }
            }
      }

void PreferencesListWidget::updatePreferences()
      {
      for (PreferenceItem* item : preferenceItems.values())
            item->update();
      }

void PreferencesListWidget::addPreference(PreferenceItem* item)
      {
      addTopLevelItem(item);
      setItemWidget(item, PREF_VALUE_COLUMN, item->editor());
      preferenceItems[item->name()] = item;
      }

void PreferencesListWidget::visit(QString key, IntPreference*)
      {
      IntPreferenceItem* item = new IntPreferenceItem(key);
      addPreference(item);
      }

void PreferencesListWidget::visit(QString key, DoublePreference*)
      {
      DoublePreferenceItem* item = new DoublePreferenceItem(key);
      addPreference(item);
      }

void PreferencesListWidget::visit(QString key, BoolPreference*)
      {
      BoolPreferenceItem* item = new BoolPreferenceItem(key);
      addPreference(item);
      }

void PreferencesListWidget::visit(QString key, StringPreference*)
      {
      StringPreferenceItem* item = new StringPreferenceItem(key);
      addPreference(item);
      }

void PreferencesListWidget::visit(QString key, ColorPreference*)
      {
      ColorPreferenceItem* item = new ColorPreferenceItem(key);
      addPreference(item);
      }

std::vector<QString> PreferencesListWidget::save()
      {
      std::vector<QString> changedPreferences;
      for (int i = 0; i < topLevelItemCount(); ++i) {
            PreferenceItem* item = static_cast<PreferenceItem*>(topLevelItem(i));
            if (item->isModified()) {
                  item->save();
                  changedPreferences.push_back(item->name());
                  }
            }

      return changedPreferences;
      }

//---------------------------------------------------------
//   PreferenceItem
//---------------------------------------------------------

PreferenceItem::PreferenceItem()
{
}

PreferenceItem::PreferenceItem(QString name)
      : _name(name)
      {
      setText(0, name);
      setSizeHint(0, QSize(0, 25));
      }

void PreferenceItem::save(QVariant value)
      {
      preferences.setPreference(name(), value);
      }

//---------------------------------------------------------
//   ColorPreferenceItem
//---------------------------------------------------------

ColorPreferenceItem::ColorPreferenceItem(QString name)
      : PreferenceItem(name),
        _initialValue(preferences.getColor(name)),
        _editor(new Awl::ColorLabel)
      {
      _editor->setColor(_initialValue);
      _editor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
      }

void ColorPreferenceItem::save()
      {
      QColor newValue = _editor->color();
      _initialValue = newValue;
      PreferenceItem::save(newValue);
      }

void ColorPreferenceItem::update()
      {
      QColor newValue = preferences.getColor(name());
      _editor->setColor(newValue);
      }

void ColorPreferenceItem::setDefaultValue()
      {
      _editor->setColor(preferences.defaultValue(name()).value<QColor>());
      }

bool ColorPreferenceItem::isModified() const
      {
      return _initialValue != _editor->color();
      }


//---------------------------------------------------------
//   IntPreferenceItem
//---------------------------------------------------------

IntPreferenceItem::IntPreferenceItem(QString name)
      : PreferenceItem(name),
        _initialValue(preferences.getInt(name))
{
      _editor = new QSpinBox;
      _editor->setMaximum(INT_MAX);
      _editor->setMinimum(INT_MIN);
      _editor->setValue(_initialValue);
}

void IntPreferenceItem::save()
      {
      int newValue = _editor->value();
      _initialValue = newValue;
      PreferenceItem::save(newValue);
      }

void IntPreferenceItem::update()
      {
      int newValue = preferences.getInt(name());
      _editor->setValue(newValue);
      }

void IntPreferenceItem::setDefaultValue()
      {
      _editor->setValue(preferences.defaultValue(name()).toInt());
      }


bool IntPreferenceItem::isModified() const
      {
      return _initialValue != _editor->value();
      }

//---------------------------------------------------------
//   DoublePreferenceItem
//---------------------------------------------------------

DoublePreferenceItem::DoublePreferenceItem(QString name)
      : PreferenceItem(name),
        _initialValue(preferences.getDouble(name)),
        _editor(new QDoubleSpinBox)
      {
      _editor->setMaximum(DBL_MAX);
      _editor->setMinimum(DBL_MIN);
      _editor->setValue(_initialValue);
      }

void DoublePreferenceItem::save()
      {
      double newValue = _editor->value();
      _initialValue = newValue;
      PreferenceItem::save(newValue);
      }

void DoublePreferenceItem::update()
      {
      double newValue = preferences.getDouble(name());
      _editor->setValue(newValue);
      }

void DoublePreferenceItem::setDefaultValue()
      {
      _editor->setValue(preferences.defaultValue(name()).toDouble());
      }

bool DoublePreferenceItem::isModified() const
      {
      return _initialValue != _editor->value();
      }


//---------------------------------------------------------
//   BoolPreferenceItem
//---------------------------------------------------------

BoolPreferenceItem::BoolPreferenceItem(QString name)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editor(new QCheckBox)
      {
      _editor->setChecked(_initialValue);
      }

void BoolPreferenceItem::save()
      {
      bool newValue = _editor->isChecked();
      _initialValue = newValue;
      PreferenceItem::save(newValue);
      }

void BoolPreferenceItem::update()
      {
      bool newValue = preferences.getBool(name());
      _editor->setChecked(newValue);
      }

void BoolPreferenceItem::setDefaultValue()
      {
      _editor->setChecked(preferences.defaultValue(name()).toBool());
      }

bool BoolPreferenceItem::isModified() const
      {
      return _initialValue != _editor->isChecked();
      }

//---------------------------------------------------------
//   StringPreferenceItem
//---------------------------------------------------------

StringPreferenceItem::StringPreferenceItem(QString name)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editor(new QLineEdit)
      {
      _editor->setText(_initialValue);
      }

void StringPreferenceItem::save()
      {
      QString newValue = _editor->text();
      _initialValue = newValue;
      PreferenceItem::save(newValue);
      }

void StringPreferenceItem::update()
      {
      QString newValue = preferences.getString(name());
      _editor->setText(newValue);
      }

void StringPreferenceItem::setDefaultValue()
      {
      _editor->setText(preferences.defaultValue(name()).toString());
      }

bool StringPreferenceItem::isModified() const
      {
      return _initialValue != _editor->text();
      }



} // namespace Ms
