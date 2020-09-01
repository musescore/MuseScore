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
#include <cfloat>

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
      for (PreferenceItem* item : _preferenceItems.values())
            item->update();
      }

void PreferencesListWidget::addPreference(PreferenceItem* item)
      {
      addTopLevelItem(item);
      setItemWidget(item, PREF_VALUE_COLUMN, item->editor());
      _preferenceItems[item->name()] = item;
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
                  item->apply();
                  changedPreferences.push_back(item->name());
                  }
            }

      return changedPreferences;
      }

//---------------------------------------------------------
//   PreferenceItem
//---------------------------------------------------------

PreferenceItem::PreferenceItem(QString name)
      : _name(name)
      {
      setText(0, name);
      setSizeHint(0, QSize(0, 25));
      }

void PreferenceItem::apply(QVariant value)
      {
      preferences.setPreference(name(), value);
      }

//---------------------------------------------------------
//   ColorPreferenceItem
//---------------------------------------------------------

ColorPreferenceItem::ColorPreferenceItem(QString name, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getColor(name)),
        _editorColorLabel(new Awl::ColorLabel)
      {
      _editorColorLabel->setColor(_initialValue);
      _editorColorLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
      connect(_editorColorLabel, &Awl::ColorLabel::colorChanged, this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

ColorPreferenceItem::ColorPreferenceItem(QString name, Awl::ColorLabel* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getColor(name)),
        _editorColorLabel(editor)
      {
      _editorColorLabel->setColor(_initialValue);
      _editorColorLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
      connect(_editorColorLabel, &Awl::ColorLabel::colorChanged, this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

void ColorPreferenceItem::apply()
      {
      if (_applyFunction) {
            _applyFunction();
            _initialValue = preferences.getColor(name());
            }
      else {
            QColor newValue = _editorColorLabel->color();
            _initialValue = newValue;
            PreferenceItem::apply(newValue);
            }
      }

void ColorPreferenceItem::update(bool setup)
      {
      if (_updateFunction) {
            _updateFunction();
            }
      else {
            QColor newValue = preferences.getColor(name());
            _editorColorLabel->setColor(newValue);
            }
      if (setup)
            setInitialValueToEditor();
      }

void ColorPreferenceItem::setDefaultValue()
      {
      _editorColorLabel->setColor(preferences.defaultValue(name()).value<QColor>());
      if (_applyFunction)
            _applyFunction();
      }

QWidget* ColorPreferenceItem::editor() const
      {
      return _editorColorLabel;
      }

bool ColorPreferenceItem::isModified() const
      {
      return _initialValue != _editorColorLabel->color();
      }

void ColorPreferenceItem::setInitialValueToEditor()
      {
      _initialValue = _editorColorLabel->color();
      }


//---------------------------------------------------------
//   IntPreferenceItem
//---------------------------------------------------------

IntPreferenceItem::IntPreferenceItem(QString name, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getInt(name)),
        _editorSpinBox(new QSpinBox)
      {
      _editorSpinBox->setMaximum(INT_MAX);
      _editorSpinBox->setMinimum(INT_MIN);
      _editorSpinBox->setValue(_initialValue);
      connect(_editorSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

IntPreferenceItem::IntPreferenceItem(QString name, QSpinBox* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getInt(name)),
        _editorSpinBox(editor)
      {
      _editorSpinBox->setValue(_initialValue);
      connect(_editorSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

IntPreferenceItem::IntPreferenceItem(QString name, QComboBox* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _editorComboBox(editor)
      {
      int index = _editorComboBox->findData(preferences.getInt(name));
      _editorComboBox->setCurrentIndex(index);
      _initialEditorIndex = index;
      connect(_editorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

void IntPreferenceItem::apply()
      {
      if (_applyFunction) {
            _applyFunction();
            _initialValue = preferences.getInt(name());
            }
      else {
            if (_editorSpinBox) {
                  int newValue = _editorSpinBox->value();
                  _initialValue = newValue;
                  PreferenceItem::apply(newValue);
                  }
            else if (_editorComboBox) {
                  int newValue = _editorComboBox->currentData().toInt();
                  PreferenceItem::apply(newValue);
                  _initialEditorIndex = _editorComboBox->currentIndex();
                  }
            }
      }

void IntPreferenceItem::update(bool setup)
      {
      if (_updateFunction) {
            _updateFunction();
            }
      else {
            if (_editorSpinBox) {
                  int newValue = preferences.getInt(name());
                  _editorSpinBox->setValue(newValue);
                  }
            else if (_editorComboBox) {
                  int index = _editorComboBox->findData(preferences.getInt(name()));
                  if (index == -1)
                        setDefaultValue();
                  else
                        _editorComboBox->setCurrentIndex(index);
                  }
            }
      if (setup)
            setInitialValueToEditor();
      }

void IntPreferenceItem::setDefaultValue()
      {
      if (_editorSpinBox) {
            _editorSpinBox->setValue(preferences.defaultValue(name()).toInt());
            }
      else if (_editorComboBox) {
            int index = _editorComboBox->findData(preferences.defaultValue(name()).toInt());
            qDebug() << "Preference: " << name() << ":" << index << " != " << "-1" << endl;
            _editorComboBox->setCurrentIndex(index);
            }
      if (_applyFunction)
            _applyFunction();
      }

QWidget* IntPreferenceItem::editor() const
      {
      if (_editorSpinBox)
            return _editorSpinBox;
      else if (_editorComboBox)
            return _editorComboBox;
      else
            Q_ASSERT(false);
      return nullptr;
      }


bool IntPreferenceItem::isModified() const
      {
      if (_editorSpinBox)
            return _initialValue != _editorSpinBox->value();
      else if (_editorComboBox)
            return _initialEditorIndex != _editorComboBox->currentIndex();
      else
            Q_ASSERT(false);
      return false;
      }

void IntPreferenceItem::setInitialValueToEditor()
      {
      if (_editorSpinBox)
            _initialValue = _editorSpinBox->value();
      else if (_editorComboBox)
            _initialEditorIndex = _editorComboBox->currentIndex();
      }

//---------------------------------------------------------
//   DoublePreferenceItem
//---------------------------------------------------------

DoublePreferenceItem::DoublePreferenceItem(QString name, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getDouble(name)),
        _editorDoubleSpinBox(new QDoubleSpinBox)
      {
      _editorDoubleSpinBox->setMaximum(DBL_MAX);
      _editorDoubleSpinBox->setMinimum(DBL_MIN);
      _editorDoubleSpinBox->setValue(_initialValue);
      if (qAbs(_initialValue) < 2.0)
            _editorDoubleSpinBox->setSingleStep(0.1);
      connect(_editorDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

DoublePreferenceItem::DoublePreferenceItem(QString name, QDoubleSpinBox* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getDouble(name)),
        _editorDoubleSpinBox(editor)
      {
      _editorDoubleSpinBox->setValue(_initialValue);
      if (qAbs(_initialValue) < 2.0)
            _editorDoubleSpinBox->setSingleStep(0.1);
      connect(_editorDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

DoublePreferenceItem::DoublePreferenceItem(QString name, QComboBox* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _editorComboBox(editor)
      {
      int index = _editorComboBox->findData(preferences.getDouble(name));
      _editorComboBox->setCurrentIndex(index);
      _initialEditorIndex = index;
      connect(_editorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

DoublePreferenceItem::DoublePreferenceItem(QString name, QSpinBox* editor, std::function<void ()> applyFunc, std::function<void ()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getDouble(name)),
        _editorSpinBox(editor)
      {
      _editorSpinBox->setValue(_initialValue);
      if (qAbs(_initialValue) < 2.0)
            _editorDoubleSpinBox->setSingleStep(0.1);
      connect(_editorSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

void DoublePreferenceItem::apply()
      {
      if (_applyFunction) {
            _applyFunction();
            _initialValue = preferences.getDouble(name());
            }
      else {
            if (_editorDoubleSpinBox) {
                  double newValue = _editorDoubleSpinBox->value();
                  _initialValue = newValue;
                  PreferenceItem::apply(newValue);
                  }
            else if (_editorComboBox) {
                  double newValue = _editorComboBox->currentData().toDouble();
                  PreferenceItem::apply(newValue);
                  _initialEditorIndex = _editorComboBox->currentIndex();
                  }
            else if (_editorSpinBox) {
                  double newValue = _editorSpinBox->value();
                  _initialValue = newValue;
                  PreferenceItem::apply(newValue);
                  }
            }
      }

void DoublePreferenceItem::update(bool setup)
      {
      if (_updateFunction) {
            _updateFunction();
            }
      else {
            if (_editorDoubleSpinBox) {
                  double newValue = preferences.getDouble(name());
                  _editorDoubleSpinBox->setValue(newValue);
                  }
            else if (_editorComboBox) {
                  int index = _editorComboBox->findData(preferences.getDouble(name()));
                  if (index == -1)
                        setDefaultValue();
                  else
                        _editorComboBox->setCurrentIndex(index);
                  }
            else if (_editorSpinBox) {
                  double newValue = preferences.getDouble(name());
                  _editorSpinBox->setValue(newValue);
                  }
            }
      if (setup)
            setInitialValueToEditor();
      }

void DoublePreferenceItem::setDefaultValue()
      {
      if (_editorDoubleSpinBox){
            _editorDoubleSpinBox->setValue(preferences.defaultValue(name()).toDouble());
            }
      else if (_editorComboBox) {
            int index = _editorComboBox->findData(preferences.defaultValue(name()).toDouble());
            _editorComboBox->setCurrentIndex(index);
            }
      else if (_editorSpinBox){
            _editorSpinBox->setValue(preferences.defaultValue(name()).toDouble());
            }
      if (_applyFunction)
            _applyFunction();
      }

QWidget* DoublePreferenceItem::editor() const
      {
      if (_editorDoubleSpinBox)
            return _editorDoubleSpinBox;
      else if (_editorComboBox)
            return _editorComboBox;
      else if (_editorSpinBox)
            return _editorSpinBox;
      else
            Q_ASSERT(false);
      return nullptr;
      }

bool DoublePreferenceItem::isModified() const
      {
      if (_editorDoubleSpinBox)
            return _initialValue != _editorDoubleSpinBox->value();
      else if (_editorComboBox)
            return _initialEditorIndex != _editorComboBox->currentIndex();
      else if (_editorSpinBox)
            return _initialValue != _editorSpinBox->value();
      else
            Q_ASSERT(false);
      return false;
      }

void DoublePreferenceItem::setInitialValueToEditor()
      {
      if (_editorDoubleSpinBox)
            _initialValue = _editorDoubleSpinBox->value();
      else if (_editorComboBox)
            _initialEditorIndex = _editorComboBox->currentIndex();
      else if (_editorSpinBox)
            _initialValue = _editorSpinBox->value();
      else
            Q_ASSERT(false);
      }


//---------------------------------------------------------
//   BoolPreferenceItem
//---------------------------------------------------------

BoolPreferenceItem::BoolPreferenceItem(QString name, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editorCheckBox(new QCheckBox)
      {
      _editorCheckBox->setChecked(_initialValue);
      connect(_editorCheckBox, &QCheckBox::toggled, this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

BoolPreferenceItem::BoolPreferenceItem(QString name, QCheckBox* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editorCheckBox(editor)
      {
      _editorCheckBox->setChecked(_initialValue);
      connect(_editorCheckBox, &QCheckBox::toggled, this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

BoolPreferenceItem::BoolPreferenceItem(QString name, QGroupBox* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editorGroupBox(editor)
      {
      _editorGroupBox->setChecked(_initialValue);
      connect(_editorGroupBox, &QGroupBox::toggled, this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

BoolPreferenceItem::BoolPreferenceItem(QString name, QRadioButton* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editorRadioButton(editor)
      {
      _editorRadioButton->setChecked(_initialValue);
      connect(_editorRadioButton, &QRadioButton::toggled, this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

void BoolPreferenceItem::apply()
      {
      if (_applyFunction) {
            _applyFunction();
            _initialValue = preferences.getBool(name());
            }
      else {
            if (_editorCheckBox) {
                  bool newValue = _editorCheckBox->isChecked();
                  _initialValue = newValue;
                  PreferenceItem::apply(newValue);
                  }
            else if (_editorGroupBox) {
                  bool newValue = _editorGroupBox->isChecked();
                  _initialValue = newValue;
                  PreferenceItem::apply(newValue);
                  }
            else if (_editorRadioButton) {
                  bool newValue = _editorRadioButton->isChecked();
                  _initialValue = newValue;
                  PreferenceItem::apply(newValue);
                  }
            }
      }

void BoolPreferenceItem::update(bool setup)
      {
      if (_updateFunction) {
            _updateFunction();
            }
      else {
            if (_editorCheckBox) {
                  bool newValue = preferences.getBool(name());
                  _editorCheckBox->setChecked(newValue);
                  }
            else if (_editorGroupBox) {
                  bool newValue = preferences.getBool(name());
                  _editorGroupBox->setChecked(newValue);
                  }
            else if (_editorRadioButton) {
                  bool newValue = preferences.getBool(name());
                  _editorRadioButton->setChecked(newValue);
                  }
            }
      if (setup)
            setInitialValueToEditor();
      }

void BoolPreferenceItem::setDefaultValue()
      {
      if (_editorCheckBox)
            _editorCheckBox->setChecked(preferences.defaultValue(name()).toBool());
      else if (_editorGroupBox)
            _editorGroupBox->setChecked(preferences.defaultValue(name()).toBool());
      else if (_editorRadioButton)
            _editorRadioButton->setChecked(preferences.defaultValue(name()).toBool());
      if (_applyFunction)
            _applyFunction();
      }

QWidget* BoolPreferenceItem::editor() const
      {
      if (_editorCheckBox)
            return _editorCheckBox;
      else if (_editorGroupBox)
            return _editorGroupBox;
      else if (_editorRadioButton)
            return _editorRadioButton;
      else
            Q_ASSERT(false);
      return nullptr;
      }

bool BoolPreferenceItem::isModified() const
      {
      if (_editorCheckBox)
            return _initialValue != _editorCheckBox->isChecked();
      else if (_editorGroupBox)
            return _initialValue != _editorGroupBox->isChecked();
      else if (_editorRadioButton)
            return _initialValue != _editorRadioButton->isChecked();
      else
            Q_ASSERT(false);
      return false;
      }

void BoolPreferenceItem::setInitialValueToEditor()
      {
      if (_editorCheckBox)
            _initialValue = _editorCheckBox->isChecked();
      else if (_editorGroupBox)
            _initialValue = _editorGroupBox->isChecked();
      else if (_editorRadioButton)
            _initialValue = _editorRadioButton->isChecked();
      else
            Q_ASSERT(false);
      }

//---------------------------------------------------------
//   StringPreferenceItem
//---------------------------------------------------------

StringPreferenceItem::StringPreferenceItem(QString name, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editorLineEdit(new QLineEdit)
      {
      _editorLineEdit->setText(_initialValue);
      connect(_editorLineEdit, &QLineEdit::textChanged, this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

StringPreferenceItem::StringPreferenceItem(QString name, QLineEdit* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editorLineEdit(editor)
      {
      _editorLineEdit->setText(_initialValue);
      connect(_editorLineEdit, &QLineEdit::textChanged, this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

StringPreferenceItem::StringPreferenceItem(QString name, QFontComboBox* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editorFontComboBox(editor)
      {
      _editorFontComboBox->setCurrentFont(QFont(_initialValue));
      connect(_editorFontComboBox, &QFontComboBox::currentFontChanged, this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

StringPreferenceItem::StringPreferenceItem(QString name, QComboBox* editor, std::function<void()> applyFunc, std::function<void()> updateFunc)
      : PreferenceItem(name),
        _editorComboBox(editor)
      {
      int index = _editorComboBox->findData(preferences.getString(name));
      _editorComboBox->setCurrentIndex(index);
      _initialEditorIndex = index;
      connect(_editorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceItem::editorValueModified);
      _applyFunction = applyFunc;
      _updateFunction = updateFunc;
      }

StringPreferenceItem::StringPreferenceItem(QString name, QRadioButton* editor, std::function<void ()> applyFunc, std::function<void ()> updateFunc)
      : PreferenceItem(name),
        _initialValue(""),
        _editorRadioButton(editor)
      {
      connect(_editorRadioButton, &QRadioButton::toggled, this, &PreferenceItem::editorValueModified);
      Q_ASSERT(applyFunc); // if an apply and an update function are not provided this cannot work
      _applyFunction = applyFunc;
      Q_ASSERT(updateFunc);
      _updateFunction = updateFunc;
      update(true);
      }


void StringPreferenceItem::apply()
      {
      if (_applyFunction) {
            _applyFunction();
            _initialValue = preferences.getString(name());
            }
      else {
            if (_editorLineEdit) {
                  QString newValue = _editorLineEdit->text();
                  _initialValue = newValue;
                  PreferenceItem::apply(newValue);
                  }
            else if (_editorFontComboBox) {
                  QString newValue = _editorFontComboBox->currentFont().family();
                  _initialValue = newValue;
                  PreferenceItem::apply(newValue);
                  }
            else if (_editorComboBox) {
                  QString newValue = _editorComboBox->currentText();
                  PreferenceItem::apply(newValue);
                  _initialEditorIndex = _editorComboBox->currentIndex();;
                  }
            else if (_editorRadioButton) {
                  // there must always be a _applyFunction
                  Q_ASSERT(false);
                  }
            }
      }

void StringPreferenceItem::update( bool setup)
      {
      if (_updateFunction) {
            _updateFunction();
            }
      else {
            if (_editorLineEdit) {
                  QString newValue = preferences.getString(name());
                  _editorLineEdit->setText(newValue);
                  }
            else if (_editorFontComboBox) {
                  QString newValue = preferences.getString(name());
                  _editorFontComboBox->setCurrentFont(QFont(newValue));
                  }
            else if (_editorComboBox) {
                  int index = _editorComboBox->findData(preferences.getString(name()));
                  if (index == -1)
                        setDefaultValue();
                  else
                        _editorComboBox->setCurrentIndex(index);
                  }
            else if (_editorRadioButton) {
                  // there must always be a _updateFunction
                  Q_ASSERT(false);
                  }
            }
      if (setup)
            setInitialValueToEditor();
      }

void StringPreferenceItem::setDefaultValue()
      {
      if (_editorLineEdit) {
            _editorLineEdit->setText(preferences.defaultValue(name()).toString());
            }
      else if (_editorFontComboBox) {
            _editorFontComboBox->setCurrentFont(QFont(preferences.defaultValue(name()).toString()));
            }
      else if (_editorComboBox) {
            int index = _editorComboBox->findData(preferences.defaultValue(name()).toString());
            _editorComboBox->setCurrentIndex(index);
            }
      else if (_editorRadioButton) {
            ;
            }
      if (_applyFunction)
            _applyFunction();
      }

QWidget* StringPreferenceItem::editor() const
      {
      if (_editorLineEdit)
            return _editorLineEdit;
      else if (_editorFontComboBox)
            return _editorFontComboBox;
      else if (_editorComboBox)
            return _editorComboBox;
      else if (_editorRadioButton)
            return _editorRadioButton;
      else
            Q_ASSERT(false);
      return nullptr;
      }

bool StringPreferenceItem::isModified() const
      {
      if (_editorLineEdit)
            return _initialValue != _editorLineEdit->text();
      else if (_editorFontComboBox)
            return _initialValue != _editorFontComboBox->currentFont().family();
      else if (_editorComboBox)
            return _initialEditorIndex != _editorComboBox->currentIndex();
      else if (_editorRadioButton)
            return _initialIsChecked != _editorRadioButton->isChecked();
      else
            Q_ASSERT(false);
      return false;
      }

void StringPreferenceItem::setInitialValueToEditor()
      {
      if (_editorLineEdit)
            _initialValue = _editorLineEdit->text();
      else if (_editorFontComboBox)
            _initialValue = _editorFontComboBox->currentFont().family();
      else if (_editorComboBox)
            _initialEditorIndex = _editorComboBox->currentIndex();
      else if (_editorRadioButton)
             _initialIsChecked = _editorRadioButton->isChecked();
      else
            Q_ASSERT(false);
      }

//---------------------------------------------------------
//   CustomPreferenceItem
//---------------------------------------------------------

CustomPreferenceItem::CustomPreferenceItem(QString name, QRadioButton* editor, std::function<void ()> applyFunc, std::function<void ()> updateFunc)
      : PreferenceItem(name),
        _editorRadioButton(editor)
      {
      Q_ASSERT(applyFunc); // if an apply and an update function are not provided this cannot work
      _applyFunction = applyFunc;
      Q_ASSERT(updateFunc);
      _updateFunction = updateFunc;
      update(true); // update on creation is the same as assigning an initival value
      connect(_editorRadioButton, &QRadioButton::toggled, this, &PreferenceItem::editorValueModified);
      }

CustomPreferenceItem::CustomPreferenceItem(QString name, QComboBox* editor, std::function<void ()> applyFunc, std::function<void ()> updateFunc)
      : PreferenceItem(name),
        _editorComboBox(editor)
      {
      Q_ASSERT(applyFunc); // if an apply and an update function are not provided this cannot work
      _applyFunction = applyFunc;
      Q_ASSERT(updateFunc);
      _updateFunction = updateFunc;
      update(true); // update on creation is the same as assigning an initival value
      connect(_editorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreferenceItem::editorValueModified);
      }

void CustomPreferenceItem::apply()
      {
      _applyFunction();
      }

void CustomPreferenceItem::update(bool setup)
      {
      _updateFunction();
      if (setup)
            setInitialValueToEditor();
      }

void CustomPreferenceItem::setDefaultValue()
      {
      Q_ASSERT(false);
      }

QWidget* CustomPreferenceItem::editor() const
      {
      if (_editorRadioButton)
            return _editorRadioButton;
      else if (_editorComboBox)
            return _editorComboBox;
      else
            Q_ASSERT(false);
      return nullptr;
      }

bool CustomPreferenceItem::isModified() const
      {
      if (_editorRadioButton)
            return _initialIsChecked != _editorRadioButton->isChecked();
      else if (_editorComboBox)
            return _initialEditorIndex != _editorComboBox->currentIndex();
      else
            Q_ASSERT(false);
      return false;
      }

void CustomPreferenceItem::setInitialValueToEditor()
      {
      if (_editorRadioButton)
            _initialIsChecked = _editorRadioButton->isChecked();
      else if (_editorComboBox)
            _initialEditorIndex = _editorComboBox->currentIndex();
      else
            Q_ASSERT(false);
      }

} // namespace Ms
