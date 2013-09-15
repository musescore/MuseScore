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

#include "globals.h"
#include "shortcut.h"
// #include "musescore.h"
#include "icons.h"
#include "libmscore/xml.h"

namespace Ms {

bool Shortcut::dirty = false;
QMap<QString, Shortcut*> Shortcut::_shortcuts;
extern QString dataPath;

//---------------------------------------------------------
//   Shortcut
//---------------------------------------------------------

Shortcut::Shortcut()
      {
      _key         = 0;
      _descr       = 0;
      _text        = 0;
      _help        = 0;
      _state       = 0;
      _flags       = 0;
      _standardKey = QKeySequence::UnknownKey;
      _context     = Qt::WindowShortcut;
      _icon        = -1;
      _action      = 0;
      }

Shortcut::Shortcut(int s, int f, const char* name, Qt::ShortcutContext cont,
   const char* txt, const char* d, const char* h, int i)
      {
      _key         = name;
      _text        = txt;
      _descr       = d ? d : _text;
      _help        = h ? h : _descr;
      _state       = s;
      _flags       = f;
      _standardKey = QKeySequence::UnknownKey;
      _context     = cont;
      _icon        = i;
      _action      = 0;
      }

Shortcut::Shortcut(int s, int f, const char* name,
   const char* txt, const char* d, const char* h, int i)
      {
      _key         = name;
      _text        = txt;
      _descr       = d ? d : _text;
      _help        = h ? h : _descr;
      _state       = s;
      _flags       = f;
      _standardKey = QKeySequence::UnknownKey;
      _context     = Qt::WindowShortcut;
      _icon        = i;
      _action      = 0;
      }

Shortcut::Shortcut(int s, int f, const char* name, const char* txt,
   const char* d, int i)
      {
      _key         = name;
      _text        = txt;
      _descr       = d;
      _help        = _descr;
      _state       = s;
      _flags       = f;
      _standardKey = QKeySequence::UnknownKey;
      _context     = Qt::WindowShortcut;
      _icon        = i;
      _action      = 0;
      }

Shortcut::Shortcut(int s, int f, const char* name, Qt::ShortcutContext cont,
   const char* txt, const char* d, int i)
      {
      _key         = name;
      _text        = txt;
      _descr       = d;
      _help        = _descr;
      _state       = s;
      _flags       = f;
      _standardKey = QKeySequence::UnknownKey;
      _context     = cont;
      _icon        = i;
      _action      = 0;
      }

Shortcut::Shortcut(int s, int f, const char* name, const char* txt, int i)
      {
      _key         = name;
      _text        = txt;
      _descr       = _text;
      _help        = _descr;
      _state       = s;
      _flags       = f;
      _standardKey = QKeySequence::UnknownKey;
      _context     = Qt::WindowShortcut;
      _icon        = i;
      _action      = 0;
      }

Shortcut::Shortcut(int s, int f, const char* name, Qt::ShortcutContext cont,
   const char* txt, int i)
      {
      _key         = name;
      _text        = txt;
      _descr       = _text;
      _help        = _descr;
      _state       = s;
      _flags       = f;
      _standardKey = QKeySequence::UnknownKey;
      _context     = cont;
      _context     = Qt::WindowShortcut;
      _icon        = i;
      _action      = 0;
      }

Shortcut::Shortcut(const Shortcut& sc)
      {
      _key         = sc._key;
      _descr       = sc._descr;
      _text        = sc._text;
      _help        = sc._help;
      _state       = sc._state;
      _flags       = sc._flags;
      _standardKey = sc._standardKey;
      _keys        = sc._keys;
      _context     = sc._context;
      _icon        = sc._icon;
      _action      = 0;
      }

//Shortcut::~Shortcut()
//      {
//      delete _action;
//    _action->deleteLater();
//      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Shortcut::clear()
      {
      _standardKey = QKeySequence::UnknownKey;
      _keys.clear();
      if (_action)
            _action->setShortcuts(_keys);
      }

//---------------------------------------------------------
//   setKeys
//---------------------------------------------------------

void Shortcut::setKeys(const QList<QKeySequence>& ks)
      {
      _standardKey = QKeySequence::UnknownKey;
      _keys = ks;
      if (_action)
            _action->setShortcuts(_keys);
      }

//---------------------------------------------------------
//   descr
//---------------------------------------------------------

QString Shortcut::descr() const
      {
      return qApp->translate("action", _descr);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Shortcut::text() const
      {
      return qApp->translate("action", _text);
      }

//---------------------------------------------------------
//   help
//---------------------------------------------------------

QString Shortcut::help() const
      {
      return qApp->translate("action", _help);
      }

//---------------------------------------------------------
//   getShortcut
//---------------------------------------------------------

Shortcut* Shortcut::getShortcut(const char* id)
      {
      Shortcut* s = _shortcuts.value(id);
      if (s == 0) {
            qDebug("internal error: shortcut <%s> not found\n", id);
            return 0;
            }
      return s;
      }

//---------------------------------------------------------
//   getAction
//    returns action for shortcut
//---------------------------------------------------------

QAction* getAction(const char* id)
      {
      Shortcut* s = Shortcut::getShortcut(id);
      return s->action();
      }

//---------------------------------------------------------
//   aAction
//---------------------------------------------------------

QAction* Shortcut::action() const
      {
      if (_action)
            return _action;

      if (_state == STATE_NEVER)
            return 0;

      _action = new QAction(_text, 0);
      _action->setData(_key);
      _action->setIconVisibleInMenu (false);

      if (_keys.isEmpty())
            _action->setShortcuts(_standardKey);
      else
            _action->setShortcuts(_keys);

      _action->setShortcutContext(_context);
      if (_help) {
            _action->setToolTip(help());
            _action->setWhatsThis(help());
            }
      else {
            _action->setToolTip(descr());
            _action->setWhatsThis(descr());
            }
      QList<QKeySequence> kl = _action->shortcuts();
      if (!kl.isEmpty()) {
            QString s(_action->toolTip());
            s += " (";
            for (int i = 0; i < kl.size(); ++i) {
                  if (i)
                        s += ",";
//                  s += kl[i].toString(QKeySequence::NativeText);
                  s += Shortcut::keySeqToString(kl[i], QKeySequence::NativeText);
                  }
            s += ")";
            _action->setToolTip(s);
            }
      if (_icon != -1)
            _action->setIcon(*icons[_icon]);
      return _action;
      }

//---------------------------------------------------------
//   addShortcut
//---------------------------------------------------------

void Shortcut::addShortcut(const QKeySequence& ks)
      {
      _keys.append(ks);
      if (_action)
            _action->setShortcuts(_keys);
      dirty = true;
      }

//---------------------------------------------------------
//   keysToString
//---------------------------------------------------------

QString Shortcut::keysToString() const
      {
      QString s;
      for (int i = 0; i < _keys.size(); ++i) {
            if (i)
                  s += "; ";
            s += Shortcut::keySeqToString(_keys[i], QKeySequence::NativeText);
            }
      return s;
      }

//---------------------------------------------------------
//   compareKeys
//    return true if keys are equal
//---------------------------------------------------------

bool Shortcut::compareKeys(const Shortcut& sc) const
      {
      if (sc._keys.size() != _keys.size())
            return false;
      for (int i = 0; i < _keys.size(); ++i) {
            if (sc._keys[i] != _keys[i])
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   initShortcuts
//---------------------------------------------------------

void Shortcut::init()
      {
      //
      // initialize shortcut hash table
      //
      _shortcuts.clear();
      for (unsigned i = 0;; ++i) {
            if (sc[i]._key == 0)
                  break;
            _shortcuts[sc[i]._key] = &sc[i];
            }
      load();
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Shortcut::save()
      {
      QFile f(dataPath + "/shortcuts.xml");
      if (!f.open(QIODevice::WriteOnly)) {
            qDebug("cannot save shortcuts");
            return;
            }
      Xml xml(&f);
      xml.header();
      xml.stag("Shortcuts");
      for (unsigned i = 0;; ++i) {
            Shortcut* s = &sc[i];
            if (s->_key == 0)
                  break;
            s->write(xml);
            }
      xml.etag();
      f.close();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Shortcut::write(Xml& xml) const
      {
      xml.stag("SC");
      xml.tag("key", _key);
      if (_standardKey != QKeySequence::UnknownKey)
            xml.tag("std", QString("%1").arg(_standardKey));
      foreach(QKeySequence ks, _keys)
//            xml.tag("seq", ks.toString(QKeySequence::PortableText));
            xml.tag("seq", Shortcut::keySeqToString(ks, QKeySequence::PortableText));
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Shortcut::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "key")
                  _key = strdup(e.readElementText().toLatin1().data());      // memory leak!
            else if (tag == "std")
                  _standardKey = QKeySequence::StandardKey(e.readInt());
            else if (tag == "seq")
//                  _keys.append(QKeySequence::fromString(e.readElementText(), QKeySequence::PortableText));
                  _keys.append(Shortcut::keySeqFromString(e.readElementText(), QKeySequence::PortableText));
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Shortcut::load()
      {
      QFile f(dataPath + "/shortcuts.xml");
      if (!f.exists())
            f.setFileName(":/data/shortcuts.xml");
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("cannot open shortcuts");
            return;
            }
      else
            qDebug("read shortcuts from <%s>", qPrintable(f.fileName()));

      XmlReader e(&f);

      while (e.readNextStartElement()) {
            if (e.name() == "Shortcuts") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "SC") {
                              Shortcut* sc = 0;
                              while (e.readNextStartElement()) {
                                    const QStringRef& tag(e.name());
                                    if (tag == "key") {
                                          QString val(e.readElementText());
                                          sc = getShortcut(val.toLatin1().data());
                                          if (!sc)
                                                qDebug("cannot find shortcut <%s>", qPrintable(val));
                                          else
                                                sc->clear();
                                          }
                                    else if (tag == "std") {
                                          int i = e.readInt();
                                          if(sc)
                                                sc->_standardKey = QKeySequence::StandardKey(i);
                                          }
                                    else if (tag == "seq") {
                                          QString s = e.readElementText();
                                          if(sc)
                                                sc->_keys.append(Shortcut::keySeqFromString(s, QKeySequence::PortableText));
//                                          sc->_keys.append(QKeySequence::fromString(e.readElementText(), QKeySequence::PortableText));
                                          }
                                    else
                                          e.unknown();
                                    }
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      dirty = false;
      }

//---------------------------------------------------------
//   Shortcut1
//---------------------------------------------------------

struct Shortcut1 {
      char* key;
      QList<QKeySequence> keys;
      QKeySequence::StandardKey standardKey;

      Shortcut1()  { key = 0; standardKey = QKeySequence::UnknownKey; }
      ~Shortcut1() { if (key) free(key); }
      };

//---------------------------------------------------------
//   read
//---------------------------------------------------------

static QList<Shortcut1*> loadDefaultShortcuts()
      {
      QList<Shortcut1*> list;
      QFile f(":/data/shortcuts.xml");
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("cannot open shortcuts");
            return list;
            }
      XmlReader e(&f);
      while (e.readNextStartElement()) {
            if (e.name() == "Shortcuts") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "SC") {
                              Shortcut1* sc = new Shortcut1;
                              sc->key = 0;
                              while (e.readNextStartElement()) {
                                    const QStringRef& tag(e.name());
                                    if (tag == "key")
                                          sc->key = strdup(e.readElementText().toLatin1().data());
                                    else if (tag == "std")
                                          sc->standardKey = QKeySequence::StandardKey(e.readInt());
                                    else if (tag == "seq")
//                                          sc->keys.append(QKeySequence::fromString(e.readElementText(), QKeySequence::PortableText));
                                          sc->keys.append(Shortcut::keySeqFromString(e.readElementText(), QKeySequence::PortableText));
                                    else
                                          e.unknown();
                                    }
                              list.append(sc);
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      return list;
      }

//---------------------------------------------------------
//   resetToBuildin
//    reset all shortcuts to builtin values
//---------------------------------------------------------

void Shortcut::resetToDefault()
      {
      QList<Shortcut1*> sl = loadDefaultShortcuts();
      foreach(Shortcut1* sc, sl) {
            Shortcut* s = getShortcut(sc->key);
            if (s) {
                  s->setKeys(sc->keys);
                  s->setStandardKey(sc->standardKey);
                  }
            }
      qDeleteAll(sl);
      dirty = true;
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Shortcut::reset()
      {
      _standardKey = QKeySequence::UnknownKey;
      _keys.clear();
      QList<Shortcut1*> sl = loadDefaultShortcuts();
      foreach(Shortcut1* sc, sl) {
            if (strcmp(sc->key, _key) == 0) {
                  setKeys(sc->keys);
                  setStandardKey(sc->standardKey);
                  break;
                  }
            }
      qDeleteAll(sl);
      dirty = true;
      }

//---------------------------------------------------------
//   keySeqToString / keySeqFromString
//---------------------------------------------------------

static const QString numPadPrefix("NumPad+");
static const int NUMPADPREFIX_SIZE = 7;         // the length in chars of the above string

QString Shortcut::keySeqToString(const QKeySequence& keySeq, QKeySequence::SequenceFormat fmt)
      {
      QString s;
      for (int i = 0; i < KEYSEQ_SIZE; ++i) {
            int code;
            if ( (code = keySeq[i]) == 0)
                  break;
            if (i)
                  s += ",";
            if (code & Qt::KeypadModifier) {
                  s += numPadPrefix;
                  code &= ~Qt::KeypadModifier;
                  }
            QKeySequence kSeq(code);
            s += kSeq.toString(fmt);
            }
      return s;
      }

QKeySequence Shortcut::keySeqFromString(const QString& str, QKeySequence::SequenceFormat fmt)
      {
      int code[KEYSEQ_SIZE], i;
      for (i = 0; i < KEYSEQ_SIZE; ++i)
            code[i] = 0;

      QStringList strList = str.split(",", QString::SkipEmptyParts, Qt::CaseSensitive);

      i = 0;
      for (const QString& s : strList) {
            QString keyStr = s.trimmed();
            if( keyStr.startsWith(numPadPrefix, Qt::CaseInsensitive) ) {
                  code[i] += Qt::KeypadModifier;
                  keyStr.remove(0, NUMPADPREFIX_SIZE);
                  }
            QKeySequence seq = QKeySequence::fromString(keyStr, fmt);
            code[i] += seq[0];
            if (++i >= KEYSEQ_SIZE)
                  break;
            }
      QKeySequence keySeq(code[0], code[1], code[2], code[3]);
      return keySeq;
      }
}

