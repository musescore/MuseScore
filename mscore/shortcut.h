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

class Xml;

enum ShortcutFlags {
      A_SCORE = 0x1, A_CMD = 0x2
      };

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
      Shortcut(const Shortcut& c);
      ~Shortcut();

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
      void setKeys(const QList<QKeySequence>& ks);
      void setStandardKey(QKeySequence::StandardKey k) {  _standardKey = k; }

      bool compareKeys(const Shortcut&) const;
      QString keysToString();
      void write(Xml&);

      static void init();
      static void load();
      static void save();
      static void resetToDefault();
      static bool dirty;
      static Shortcut* getShortcut(const char* key);
      static const QMap<QString, Shortcut*>& shortcuts() { return _shortcuts; }
      };

#endif

