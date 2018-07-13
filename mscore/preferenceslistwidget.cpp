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
#include "preferencestreewidget_delegate.h"

namespace Ms {

extern QString mscoreGlobalShare;

PreferencesListWidget::PreferencesListWidget(QWidget* parent)
      : QTreeWidget(parent)
      {
      setObjectName("PreferencesListWidget");
      header()->setSectionResizeMode(0, QHeaderView::Interactive);
      sortByColumn(0, Qt::AscendingOrder);
      loadPreferences();

      setItemDelegate(new Ms::PreferencesTreeWidget_Delegate(this));
      expandAll();
      resizeColumnToContents(0);

      setContextMenuPolicy(Qt::ActionsContextMenu);
      QAction* selectAllPreferences = new QAction(tr("Select all preferences"), this);
      selectAllPreferences->setShortcut(QKeySequence::SelectAll);
      addAction(selectAllPreferences);
      connect(selectAllPreferences, &QAction::triggered, this, &PreferencesListWidget::selectAllVisiblePreferences);
      }

PreferencesListWidget::~PreferencesListWidget()
      {
      }

// Find the first child of parent with text 'name'
QTreeWidgetItem* PreferencesListWidget::findChildByText(const QTreeWidgetItem* parent, const QString& text, const int column) const
      {
      for (int childNum = 0; childNum < parent->childCount(); ++childNum) {
            QTreeWidgetItem* child = parent->child(childNum);
            if (child->text(column) == text)
                  return child;
            }
      return nullptr;
      }

// Gets the list af all items in that have parent as a parent, direct or not.
void PreferencesListWidget::recursiveChildList(QList<QTreeWidgetItem*>& list, QTreeWidgetItem* parent) const
      {
      list << parent;
      for(int i = 0; i < parent->childCount(); ++i)
            recursiveChildList(list, parent->child(i));
      }

// Gets the list af all items in that have parent as a parent, direct or not.
// This is an overloded function.
const QList<QTreeWidgetItem*> PreferencesListWidget::recursiveChildList(QTreeWidgetItem* parent) const
      {
      QList<QTreeWidgetItem*> list;

      // if there's no parent, return an emty list.
      if (!parent) {
            qDebug() << "QList<PreferenceItem*> PreferencesListWidget::recursivePreferenceList(QTreeWidgetItem* parent)"
                        " : invalid parent. Returning an empty list";
            return list;
            }

      recursiveChildList(list, parent);
      return list;
      }

// for if a 'showAll' checkbox is implemented
#if 0
void PreferencesListWidget::showAll(const bool all)
      {
      for (PreferenceItem* item : preferenceItems->values()) {
            if (!(preferences.allPreferences().value(item->name())->showInAdvancedList()))
                  item->setHidden(!all);
            }
      hideEmptyItems();
      }

// This function combines the search filter and the showAll checkBox (which isn't currently implemented)
void PreferencesListWidget::filterVisiblePreferences(const QString& query, const bool all)
      {
      // if there's no double thing (all and a query) just use one of the two.
      if (query.isEmpty()) {
            showAll(all);
            return;
            }
      if (!all) {
            filter(query);
            return;
            }

      QString queryLowered = query.toLower();
      for (PreferenceItem* item : preferenceItems->values()) {
            // If the URL of the item contains the query, and the item needs to be shown (because
            // of paremeter "all" or because the preference is always in the advanced list).
            item->setVisible(((item->name().toLower().contains(queryLowered))
                              && (all || (preferences.allPreferences().value(item->name())->showInAdvancedList()))));
            }

      hideEmptyItems();
      }
#endif

const QList<PreferenceItem*> PreferencesListWidget::recursivePreferenceItemList(QTreeWidgetItem* parent) const
      {
      QList<PreferenceItem*> preferenceItemList;
      // return an empty list if parent doesn't exist.
      if (!parent) {
            qDebug() << "QList<PreferenceItem*> PreferencesListWidget::recursivePreferenceList(QTreeWidgetItem* parent)"
                        " : invalid parent. Returning an empty list";
            return preferenceItemList;
            }

      for (QTreeWidgetItem* child : recursiveChildList(parent)) {
            PreferenceItem* castedChild = dynamic_cast<PreferenceItem*> (child);
            if (castedChild)
                  preferenceItemList << castedChild;
            }

      return preferenceItemList;
      }

void PreferencesListWidget::loadPreferences()
      {
      QTreeWidgetItem* currentParent = invisibleRootItem();
      // iterate over all the preferences.
      for (QString path : preferences.allPreferences().keys()) {
            // see preftranslations.h for details.
            Preference* pref = preferences.allPreferences().value(path);
#ifdef PREFS_NO_SUPPORT_FOR_ENUMS
            // For now, enums are of Type QMetaType::User.
            // See EnumPreference class for more details.
            if (pref->type() == QMetaType::User)
                  continue;

#endif // PREFS_NO_SUPPORT_FOR_ENUMS
            if(!pref->showInAdvancedList())
                  continue;

            // iterate over the directories of the preferences.
            QStringList dirs = path.split("/");
            for (int dirNumber = 0; dirNumber < dirs.count(); ++dirNumber) {
                  QString currentDir = dirs.at(dirNumber);
                  // check if child already exists.
                  QTreeWidgetItem* child = findChildByText(currentParent, currentDir, 0);
                  // if doesn't exist, appendChild. if exist, current parent becomes child.
                  if (!child) {
                        // if it's not a "directory" but it's a "file",
                        // then just change it to the corresponding preferenceItem.
                        if (dirNumber == dirs.count() - 1) {
                              // send the english path as first argument, so the preference keeps its name.
                              pref->accept(path, currentParent, *this);
                              }
                        else
                              currentParent->addChild(new QTreeWidgetItem(currentParent, QStringList() << currentDir));
                        currentParent = currentParent->child(currentParent->childCount() - 1);
                        }
                  else
                        currentParent = child;
                  }
            // once the preference is put, get back to the root item
            // to put the next preference.
            currentParent = invisibleRootItem();
            }
      }

void PreferencesListWidget::updatePreferences()
      {
      for (PreferenceItem* item : preferenceItems.values())
            item->update();
      }

void PreferencesListWidget::addPreference(PreferenceItem* item)
      {
      setItemWidget(item, PREF_VALUE_COLUMN, item->editor());
      preferenceItems[item->name()] = item;
      }

void PreferencesListWidget::visit(const QString& key, QTreeWidgetItem* parent, IntPreference*)
      {
      IntPreferenceItem* item = new IntPreferenceItem(key);
      parent->addChild(item);
      addPreference(item);
      }

void PreferencesListWidget::visit(const QString& key, QTreeWidgetItem* parent, DoublePreference*)
      {
      DoublePreferenceItem* item = new DoublePreferenceItem(key);
      parent->addChild(item);
      addPreference(item);
      }

void PreferencesListWidget::visit(const QString& key, QTreeWidgetItem* parent, BoolPreference*)
      {
      BoolPreferenceItem* item = new BoolPreferenceItem(key);
      parent->addChild(item);
      addPreference(item);
      }

void PreferencesListWidget::visit(const QString& key, QTreeWidgetItem* parent, StringPreference*)
      {
      StringPreferenceItem* item = new StringPreferenceItem(key);
      parent->addChild(item);
      addPreference(item);
      }

void PreferencesListWidget::visit(const QString& key, QTreeWidgetItem* parent, FilePreference*)
      {
      FilePreferenceItem* item = new FilePreferenceItem(key);
      parent->addChild(item);
      addPreference(item);
      }

void PreferencesListWidget::visit(const QString& key, QTreeWidgetItem* parent, DirPreference*)
      {
      DirPreferenceItem* item = new DirPreferenceItem(key);
      parent->addChild(item);
      addPreference(item);
      }

void PreferencesListWidget::visit(const QString& key, QTreeWidgetItem* parent, ColorPreference*)
      {
      ColorPreferenceItem* item = new ColorPreferenceItem(key);
      parent->addChild(item);
      addPreference(item);
      }

void PreferencesListWidget::filter(const QString& query)
      {
      QString s = query.toLower();
      for (PreferenceItem* item : preferenceItems.values())
            item->setVisible(item->name().toLower().contains(s));
      hideEmptyItems();
      }

void PreferencesListWidget::resetSelectedPreferencesToDefault()
      {
      preferences.setReturnDefaultValues(true);
      for (QTreeWidgetItem* item : selectedItems()) {
            PreferenceItem* pref = dynamic_cast<PreferenceItem*> (item);
            if (pref)
                  pref->setDefaultValue();
            }
      preferences.setReturnDefaultValues(false);
      }

// Hide the QTreeWidgetItems which are not parent of any VISIBLE PreferenceItems.
void PreferencesListWidget::hideEmptyItems() const
      {
      // iterate over all items.
      for(QTreeWidgetItem* parent : recursiveChildList(invisibleRootItem())) {
            // If the item is already hidden, nothing to do.
            if (parent->isHidden())
                  continue;

            // Hide the parent if it doesn't contain visible PreferenceItems
            // which aren't hidden.
            bool hide = true;
            for(PreferenceItem* pref : recursivePreferenceItemList(parent)) {
                  if (!pref->isHidden()) {
                        hide = false;
                        break;
                        }
                  }
            parent->setHidden(hide);
            }
      }

void PreferencesListWidget::selectAllVisiblePreferences()
      {
      clearSelection();
      for (PreferenceItem* pref : preferenceItems.values())
            pref->setSelected(!pref->isHidden());
      }

void PreferencesListWidget::save() const
      {
      for (PreferenceItem* item : preferenceItems.values()) {
            if (item->isModified())
                  item->save();
            }
      }

//---------------------------------------------------------
//   PreferenceItem
//---------------------------------------------------------

PreferenceItem::PreferenceItem(const QString& name)
      : _name(name)
      {
      setObjectName("PreferenceItem");
      setText(0, name.split("/").last());
      }

void PreferenceItem::save(QVariant value)
      {
      preferences.setPreference(name(), value);
      }

void PreferenceItem::setVisible(const bool visible)
      {
      if (visible) {
            // show the item and it's parents
            setHidden(false);
            QTreeWidgetItem* item = this->QTreeWidgetItem::parent();
            while(item) {
                  item->setExpanded(true);
                  item->setHidden(false);
                  item = item->QTreeWidgetItem::parent();
                  }
            }
      else
            setHidden(true);
      }

//---------------------------------------------------------
//   ColorPreferenceItem
//---------------------------------------------------------

ColorPreferenceItem::ColorPreferenceItem(const QString& name)
      : PreferenceItem(name),
        _initialValue(preferences.getColor(name)),
        _editor(new Awl::ColorLabel(treeWidget()))
      {
      _editor->setColor(_initialValue);
      _editor->setText(tr("Click to modify"));
      _editor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

      auto setToolTip = [&](const QColor& c)
            {
            _editor->setToolTip(tr("RGBA: (%1, %2, %3, %4)")
                                .arg(c.red())
                                .arg(c.green())
                                .arg(c.blue())
                                .arg(c.alpha()));
            };
      setToolTip(_initialValue);

      connect(_editor, &Awl::ColorLabel::colorChanged, this, setToolTip);
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
      return (_initialValue != _editor->color());
      }


//---------------------------------------------------------
//   IntPreferenceItem
//---------------------------------------------------------

IntPreferenceItem::IntPreferenceItem(const QString& name)
      : PreferenceItem(name),
        _initialValue(preferences.getInt(name)),
        _editor(new QSpinBox(treeWidget()))
      {
      setObjectName("IntPreferenceItem");
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
      return (_initialValue != _editor->value());
      }

//---------------------------------------------------------
//   DoublePreferenceItem
//---------------------------------------------------------

DoublePreferenceItem::DoublePreferenceItem(const QString& name)
      : PreferenceItem(name),
        _initialValue(preferences.getDouble(name)),
        _editor(new QDoubleSpinBox(treeWidget()))
      {
      setObjectName("DoublePreferenceItem");
      _editor->setFocusPolicy(Qt::ClickFocus); // disable accepting wheel events
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
      return (_initialValue != _editor->value());
      }


//---------------------------------------------------------
//   BoolPreferenceItem
//---------------------------------------------------------

BoolPreferenceItem::BoolPreferenceItem(const QString& name)
      : PreferenceItem(name),
        _initialValue(preferences.getBool(name)),
        _editor(new QCheckBox(treeWidget()))
      {
      setObjectName("BoolPreferenceItem");
      _editor->setChecked(_initialValue);
      _editor->setCursor(Qt::PointingHandCursor);
      _editor->setForegroundRole(QPalette::NoRole); // make the text visible, even if selected
      auto setTexts = [&](bool checked)
            {
            _editor->setText(checked ? tr("true") : tr("false"));
            _editor->setToolTip(checked ? tr("true") : tr("false"));
            };
      setTexts(_initialValue);
      connect(_editor, &QCheckBox::toggled, this, setTexts);
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
      return (_initialValue != _editor->isChecked());
      }

//---------------------------------------------------------
//   StringPreferenceItem
//---------------------------------------------------------

StringPreferenceItem::StringPreferenceItem(const QString& name)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editor(new QLineEdit(treeWidget()))
      {
      setObjectName("StringPreferenceItem");
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
      return (_initialValue != _editor->text());
      }

//---------------------------------------------------------
//   FilePreferenceItem
//---------------------------------------------------------

FilePreferenceItem::FilePreferenceItem(const QString& name)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editor(new QPushButton(treeWidget()))
      {
      setObjectName("FilePreferenceItem");
      _editor->setText(_initialValue);
      _editor->setCursor(Qt::PointingHandCursor);
      _editor->setToolTip(tr("Click to choose a new file..."));
      if (_initialValue.isEmpty())
            _editor->setText(tr("No file selected"));
      connect(_editor, &QPushButton::clicked, this, &FilePreferenceItem::getFile);
      }

void FilePreferenceItem::save()
      {
      QString newValue = _editor->text();
      _initialValue = newValue;
      PreferenceItem::save(newValue);
      }

void FilePreferenceItem::update()
      {
      QString newValue = preferences.getString(name());
      _editor->setText(newValue);
      }

void FilePreferenceItem::setDefaultValue()
      {
      _editor->setText(preferences.defaultValue(name()).toString());
      }

bool FilePreferenceItem::isModified() const
      {
      return (_initialValue != _editor->text());
      }

void FilePreferenceItem::getFile() const
      {
      QString fileName = QFileDialog::getOpenFileName (
                               treeWidget(),
                               tr("Choose file"),
                               QFile(_editor->text()).exists()
                               ? _editor->text() : mscoreGlobalShare,
                               static_cast<FilePreference*> (preferences.allPreferences().value(name()))->filter(),
                               0,
                               (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)
                                ? QFileDialog::Options() : QFileDialog::Options() | QFileDialog::DontUseNativeDialog)
                               );
      if (!fileName.isNull() && (fileName != _editor->text())) {
            _editor->setText(fileName);
            if (fileName.isEmpty())
                  _editor->setText(tr("No file selected"));
            }
      }

//---------------------------------------------------------
//   DirPreferenceItem
//---------------------------------------------------------

DirPreferenceItem::DirPreferenceItem(const QString& name)
      : PreferenceItem(name),
        _initialValue(preferences.getString(name)),
        _editor(new QPushButton(treeWidget()))
      {
      setObjectName("DirPreferenceItem");
      _editor->setText(_initialValue);
      _editor->setCursor(Qt::PointingHandCursor);
      _editor->setToolTip(tr("Click to choose a new directory..."));
      if (_initialValue.isEmpty())
            _editor->setText(tr("No directory selected"));
      connect(_editor, &QPushButton::clicked, this, &DirPreferenceItem::getDirectory);
      }

void DirPreferenceItem::save()
      {
      QString newValue = _editor->text();
      _initialValue = newValue;
      PreferenceItem::save(newValue);
      }

void DirPreferenceItem::update()
      {
      QString newValue = preferences.getString(name());
      _editor->setText(newValue);
      }

void DirPreferenceItem::setDefaultValue()
      {
      _editor->setText(preferences.defaultValue(name()).toString());
      }

bool DirPreferenceItem::isModified() const
      {
      return (_initialValue != _editor->text());
      }

void DirPreferenceItem::getDirectory() const
      {
      QString dir = QFileDialog::getExistingDirectory (
                       treeWidget(),
                       tr("Choose directory"),
                       QDir(_editor->text()).exists()
                       ? _editor->text() : mscoreGlobalShare,
                       (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS)
                        ? QFileDialog::Options() : QFileDialog::Options() | QFileDialog::DontUseNativeDialog)
                       );
      if (!dir.isNull() && (dir != _editor->text())) {
            _editor->setText(dir);
            if (dir.isEmpty())
                  _editor->setText(tr("No directory selected"));
            }
      }

} // namespace Ms
