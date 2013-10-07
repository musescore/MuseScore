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

#ifndef __SHORTCUT_H__
#define __SHORTCUT_H__

/*---------------------------------------------------------
NOTE ON ARCHITECTURE

The Shortcut class describes the basic configurable shortcut element.
'Real' data are contained in 2 static member variables:

1) sc[], an array of Shortcut: contains the default, built-in data for each shortcut
      except the key sequences; it is initialized at startup (code at the begining of
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

namespace Ms {

class Xml;
class XmlReader;

enum ShortcutFlags {
      A_SCORE = 0x1, A_CMD = 0x2
      };

static const int KEYSEQ_SIZE = 4;

//---------------------------------------------------------
//   Shortcut
//    hold the basic values for configurable shortcuts
//---------------------------------------------------------

class Shortcut {
      const char* _key;     //! xml tag name for configuration file
      const char* _descr;   //! descriptor, shown in editor
      const char* _text;    //! text as shown on buttons or menus
      const char* _help;    //! ballon help
      int _state;           //! shortcut is valid in this Mscore state
                            //! (or'd list of states)
      int _flags;

      QList<QKeySequence> _keys;     //! shortcut list
      QKeySequence::StandardKey _standardKey;
      Qt::ShortcutContext _context;
      int _icon;
      mutable QAction* _action;             //! cached action

      static Shortcut sc[];
      static QMap<QString, Shortcut*> _shortcuts;

   public:
      Shortcut();
      Shortcut(int state, int flags,
         const char* name,
         Qt::ShortcutContext cont,
         const char* d,
         const char* txt = 0,
         const char* h = 0,
         int i = -1);
      Shortcut(int state, int flags,
         const char* name,
         const char* d,
         const char* txt = 0,
         const char* h = 0,
         int i = -1);
      Shortcut(int state, int flags,
         const char* name,
         const char* d,
         int i);
      Shortcut(int state, int flags,
         const char* name,
         const char* d,
         const char* txt,
         int i);
      Shortcut(int state, int flags,
         const char* name,
         Qt::ShortcutContext cont,
         const char* d,
         int i);
      Shortcut(int state, int flags,
         const char* name,
         Qt::ShortcutContext cont,
         const char* d,
         const char* txt,
         int i);

      Shortcut(const Shortcut& c);
      ~Shortcut() {}

      QAction* action() const;

      const char* key() const { return _key; }
      QString descr() const;
      QString text() const;
      QString help() const;
      void clear();           //! remove shortcuts
      void reset();           //! reset to buildin
      void addShortcut(const QKeySequence&);
      int state() const                        { return _state; }
      int flags() const                        { return _flags; }
      int icon() const                         { return _icon;  }
      const QList<QKeySequence>& keys() const  { return _keys;  }
      QKeySequence::StandardKey standardKey() const { return _standardKey; }
      void setKeys(const QList<QKeySequence>& ks);
      void setStandardKey(QKeySequence::StandardKey k) {  _standardKey = k; }

      bool compareKeys(const Shortcut&) const;
      QString keysToString() const;
      void write(Ms::Xml&) const;
      void read(Ms::XmlReader&);

      static void init();
      static void load();
      static void save();
      static void resetToDefault();
      static bool dirty;
      static Shortcut* getShortcut(const char* key);
      static const QMap<QString, Shortcut*>& shortcuts() { return _shortcuts; }

      static QString keySeqToString(const QKeySequence& keySeq, QKeySequence::SequenceFormat fmt);
      static QKeySequence keySeqFromString(const QString& str, QKeySequence::SequenceFormat fmt);
      };

} // namespace Ms
#endif

