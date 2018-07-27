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

#ifndef __PREFERENCESLISTWIDGET_H__
#define __PREFERENCESLISTWIDGET_H__

#include "awl/colorlabel.h"
#include "preferences.h"

#define PREF_VALUE_COLUMN 1

#include <QtWidgets>

namespace Ms {

//---------------------------------------------------------
//   PreferenceItem
//---------------------------------------------------------

// TODO: Add a Use'type'Preference class (ex: UseStringPreferenceItem),
// which is basically a string that can be enabled or not.
// PREF_UI_CANVAS_BG_USECOLOR and PREF_UI_CANVAS_BG_COLOR would be merged into
// a UseColorPreferenceItem instead of 2 distinct items.
class PreferenceItem : public QTreeWidgetItem, public QObject {

      // the name is actually the Url (or #define value) of the preference
      QString _name;

    protected:
      void save(QVariant value);

    public:
      PreferenceItem(const QString& name = ""); // default value to have a default ctor

      virtual QWidget* editor() const = 0;
      virtual void save() = 0;
      // using a variable other than name means it will go take the information coming
      // from another name. THis is useful, for example, to sync a preference with a
      // temporary copy of itself (probably under the name ("temporary" + name())
      virtual void update(const QString name = this->name()) = 0;
      virtual void setDefaultValue() = 0;
      virtual bool isModified() const = 0;
      void setVisible(const bool visible);
      const QString& name() const { return _name; }
      };

//---------------------------------------------------------
//   BoolPreferenceItem
//---------------------------------------------------------
class BoolPreferenceItem : public PreferenceItem {
   private:
      bool _initialValue;
      QCheckBox* _editor;

   public:
      BoolPreferenceItem(const QString& name);

      QWidget* editor() const override { return _editor; }
      inline virtual void save() override;
      inline virtual void update(const QString name = name()) override;
      inline virtual void setDefaultValue() override;
      inline virtual bool isModified() const override;
      };

//---------------------------------------------------------
//   IntPreferenceItem
//---------------------------------------------------------
class IntPreferenceItem : public PreferenceItem {
      int _initialValue;
      QSpinBox* _editor;

   public:
      IntPreferenceItem(const QString& name);

      QWidget* editor() const override { return _editor; }
      inline virtual void save() override;
      inline virtual void update(const QString name = name()) override;
      inline virtual void setDefaultValue() override;
      inline virtual bool isModified() const override;
      };

//---------------------------------------------------------
//   DoublePreferenceItem
//---------------------------------------------------------
class DoublePreferenceItem : public PreferenceItem {
      double _initialValue;
      QDoubleSpinBox* _editor;

   public:
      DoublePreferenceItem(const QString& name);

      QWidget* editor() const override { return _editor; }
      inline virtual void save() override;
      inline virtual void update(const QString name = name()) override;
      inline virtual void setDefaultValue() override;
      inline virtual bool isModified() const override;
      };

//---------------------------------------------------------
//   StringPreferenceItem
//---------------------------------------------------------
class StringPreferenceItem : public PreferenceItem {
      QString _initialValue;
      QLineEdit* _editor;

   public:
      StringPreferenceItem(const QString& name);

      QWidget* editor() const override { return _editor; }
      inline virtual void save() override;
      inline virtual void update(const QString name = name()) override;
      inline virtual void setDefaultValue() override;
      inline virtual bool isModified() const override;
      };

//---------------------------------------------------------
//   FilePreferenceItem
//---------------------------------------------------------
class FilePreferenceItem : public PreferenceItem {
      QString _initialValue;
      QPushButton* _editor;

   public:
      FilePreferenceItem(const QString& name);

      QWidget* editor() const override { return _editor; }
      inline virtual void save() override;
      inline virtual void update(const QString name = name()) override;
      inline virtual void setDefaultValue() override;
      inline virtual bool isModified() const override;

   private slots:
      void getFile() const;
      };

//---------------------------------------------------------
//   DirPreferenceItem
//---------------------------------------------------------
class DirPreferenceItem : public PreferenceItem {
      QString _initialValue;
      QPushButton* _editor;

   public:
      DirPreferenceItem(const QString& name);

      QWidget* editor() const override { return _editor; }
      inline virtual void save() override;
      inline virtual void update(const QString name = name()) override;

      inline virtual void setDefaultValue() override;
      inline virtual bool isModified() const override;

   private slots:
      void getDirectory() const;
      };

//---------------------------------------------------------
//   ColorPreferenceItem
//---------------------------------------------------------
class ColorPreferenceItem : public PreferenceItem {
      QColor _initialValue;
      Awl::ColorLabel* _editor;

   public:
      ColorPreferenceItem(const QString& name);

      QWidget* editor() const override { return _editor; }
      inline virtual void save() override;
      inline virtual void update(const QString name = name()) override;

      inline virtual void setDefaultValue() override;
      inline virtual bool isModified() const override;
      };


//---------------------------------------------------------
//   PreferencesListWidget
//---------------------------------------------------------

class PreferencesListWidget : public QTreeWidget, public PreferenceVisitor {

      QHash<const QString, PreferenceItem*> preferenceItems;

      void addPreference(PreferenceItem* item);
      QTreeWidgetItem* findChildByText(const QTreeWidgetItem* parent, const QString& text, const int column) const;
      void recursiveChildList(QList<QTreeWidgetItem*>& list, QTreeWidgetItem* item) const;
      const QList<QTreeWidgetItem*> recursiveChildList(QTreeWidgetItem* parent) const;
      const QList<PreferenceItem*> recursivePreferenceItemList(QTreeWidgetItem* parent) const;

   private slots:
      void hideEmptyItems() const;
      void selectAllVisiblePreferences();

      virtual void showEvent(QShowEvent* event) override;
      virtual void keyPressEvent(QKeyEvent* event) override;

   public:
      explicit PreferencesListWidget(QWidget* parent = nullptr);
      ~PreferencesListWidget();

      void loadPreferences();
      void updatePreferences();
      // not used for now, since there is no syncing with the other tabs of the prefs dialog.
      void updateToTemporaryPreferences();

      void save() const;

      virtual void visit(const QString& key, QTreeWidgetItem* parent, IntPreference*)    override;
      virtual void visit(const QString& key, QTreeWidgetItem* parent, DoublePreference*) override;
      virtual void visit(const QString& key, QTreeWidgetItem* parent, BoolPreference*)   override;
      virtual void visit(const QString& key, QTreeWidgetItem* parent, StringPreference*) override;
      virtual void visit(const QString& key, QTreeWidgetItem* parent, ColorPreference*)  override;
      // for now, there is no file and directory preferences in the advanced tab:
      // they are all managed in the other tabs.
      virtual void visit(const QString& key, QTreeWidgetItem* parent, FilePreference*)   override;
      virtual void visit(const QString& key, QTreeWidgetItem* parent, DirPreference*)    override;

   public slots:
      void filter(const QString& query);
      void resetSelectedPreferencesToDefault();
      // If a "showAll" check box is put, this function combines the filter and the showAll checkBox
      /// void filterVisiblePreferences(const QString& query, const bool all);
      // And this one does a simple show all.
      /// void showAll(const bool all = true);
};

} // namespace Ms

#endif // __PREFERENCESLISTWIDGET_H__
