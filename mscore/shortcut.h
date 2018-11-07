//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __SHORTCUT_H__
#define __SHORTCUT_H__

/*---------------------------------------------------------
NOTE ON ARCHITECTURE

The Shortcut class describes the basic configurable shortcut element.
'Real' data are contained in 2 static member variables:

1) sc[], an array of Shortcut: contains the default, built-in data for each shortcut
      except the key sequences; it is initialized at startup (code at the beginning of
      mscore/actions.cpp)
2) _shortcuts, a QMap using the shortcut xml tag name as hash value: is initialized from
      data in sc via a call to Shortcut::init() in program main() (mscore/musescore.cpp).
      This also load actual key sequences either from an external, hard-coded, file with
      user customizations or from a resource (<= mscore/data/shortcuts.xml), if there are
      no customizations.
      Later during startup, QAction's are derived from each of its elements and pooled
      in a single QActionGroup during MuseScore::MuseScore() costructor (mscore/musescore.cpp)

ShortcutFlags:
      To be documented

State flags:

Defined in mscore/global.h (ScoreState enum): each shortcut is ignored if its _flags mask
does not include the current score state. This is different from (and additional to)
QAction processing performed by the Qt framework and happens only after the action has
been forwarded to the application (the action must be enabled).

The STATE_NEVER requires an explanation. It has been introduced to mark shortcuts
which need to be recorded (and possibly customized) but are never used directly.
Currently, this applies to a number of shortcuts which:
- have been split between a common and a TAB-specific variant AND
- are linked to tool bar buttons or menu items
If QAction's are created for both, Qt blocks either as duplicate; in addition, the button
or menu item may become disabled on state change. The currently implemented solution is
to create a QAction only for one of them (the common one) and swap the key sequences when
entering or leaving the relevant state.
Swapping is implemented in MuseScore::changeState() (mscore/musescore.cpp).
QAction creation for the 'other' shortcut is blocked in Shortcut::action() (mscore/shortcut.cpp).

This means that Shortcut::action() may return 0. When scanning the whole
shortcuts[] array, this has to be taken into account; currently it happens in two
cases:
- in MuseScore::MuseScore() constructor (mscore/musescore.cpp)
- in MuseScore::changeState() method (mscore/musescore.cpp)

Shortcuts marked with the STATE_NEVER state should NEVER used directly as shortcuts!
---------------------------------------------------------*/

#include "icons.h"
#include "globals.h"

namespace Ms {

class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   ShortcutFlags
//---------------------------------------------------------

enum class ShortcutFlags : char {
      NONE        = 0,
      A_SCORE     = 1,
      A_CMD       = 1 << 1,
      A_CHECKABLE = 1 << 2,
      A_CHECKED   = 1 << 3
      };

constexpr ShortcutFlags operator| (ShortcutFlags t1, ShortcutFlags t2) {
      return static_cast<ShortcutFlags>(static_cast<int>(t1) | static_cast<int>(t2));
      }

constexpr bool operator& (ShortcutFlags t1, ShortcutFlags t2) {
      return static_cast<int>(t1) & static_cast<int>(t2);
      }

static const int KEYSEQ_SIZE = 4;

//---------------------------------------------------------
//   Shortcut
//    hold the basic values for configurable shortcuts
//---------------------------------------------------------

class Shortcut {
      MsWidget _assignedWidget;   //! the widget where the action will be assigned
      int _state { 0 };           //! shortcut is valid in this Mscore state
      QByteArray _key;            //! xml tag name for configuration file
      QByteArray _descr;          //! descriptor, shown in editor
      QByteArray _text;           //! text as shown on buttons or menus
      QByteArray _help;           //! ballon help
                                  //! (or'd list of states)

      Icons _icon                            { Icons::Invalid_ICON };
      Qt::ShortcutContext _context           { Qt::WindowShortcut };
      ShortcutFlags _flags                   { ShortcutFlags::NONE };

      QList<QKeySequence> _keys;     //! shortcut list

      QKeySequence::StandardKey _standardKey { QKeySequence::UnknownKey };
      mutable QAction* _action               { 0 };             //! cached action

      static QString source;

      static Shortcut _sc[];
      static QHash<QByteArray, Shortcut*> _shortcuts;
      void translateAction(QAction* action) const;

   public:

      static constexpr const char* defaultFileName = ":/data/shortcuts.xml";

      Shortcut() {}
      Shortcut(
         Ms::MsWidget assignedWidget,
         int state,
         const char* key,
         const char* d    = 0,
         const char* txt  = 0,
         const char* h    = 0,
         Icons i          = Icons::Invalid_ICON,
         Qt::ShortcutContext cont = Qt::WindowShortcut,
         ShortcutFlags f = ShortcutFlags::NONE
         );

      QAction* action() const;
      const QByteArray& key() const { return _key; }
      QString descr() const;
      QString text() const;
      QString help() const;
      MsWidget assignedWidget() const { return _assignedWidget; }
      void clear();           //! remove shortcuts
      void reset();           //! reset to buildin
      void addShortcut(const QKeySequence&);
      int state() const                        { return _state; }
      void setState(int v)                      { _state = v;     }
      bool needsScore() const                  { return _flags & ShortcutFlags::A_SCORE; }
      bool isCmd() const                       { return _flags & ShortcutFlags::A_CMD; }
      bool isCheckable() const                 { return _flags & ShortcutFlags::A_CHECKABLE; }
      bool isChecked() const                   { return _flags & ShortcutFlags::A_CHECKED; }
      Icons icon() const                       { return _icon;  }
      const QList<QKeySequence>& keys() const  { return _keys;  }
      QKeySequence::StandardKey standardKey() const { return _standardKey; }
      void setStandardKey(QKeySequence::StandardKey k);
      void setKeys(const QList<QKeySequence>& ks);
      void setKeys(const Shortcut&);

      bool compareKeys(const Shortcut&) const;
      QString keysToString() const;
      static QString getMenuShortcutString(const QMenu* menu);

      void write(Ms::XmlWriter&) const;
      void read(Ms::XmlReader&);

      static void init();
      static void retranslate();
      static void refreshIcons();
      static void load();
      static void loadFromNewFile(QString fileLocation);
      static void save();
      static void saveToNewFile(QString fileLocation);
      static void resetToDefault();
      static bool dirty;
      static bool customSource() { return source != defaultFileName; }
      static Shortcut* getShortcut(const char* key);
      static const QHash<QByteArray, Shortcut*>& shortcuts() { return _shortcuts; }
      static QActionGroup* getActionGroupForWidget(MsWidget w);
      static QActionGroup* getActionGroupForWidget(MsWidget w, Qt::ShortcutContext newShortcutContext);

      static QString keySeqToString(const QKeySequence& keySeq, QKeySequence::SequenceFormat fmt);
      static QKeySequence keySeqFromString(const QString& str, QKeySequence::SequenceFormat fmt);
      };

} // namespace Ms
#endif

