//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: scbytearray.cpp 1840 2009-05-20 11:57:51Z wschweer $
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "scbytearray.h"

//
// demo class from qt
//
Q_DECLARE_METATYPE(QByteArray*)
Q_DECLARE_METATYPE(ByteArrayClass*)

class ByteArrayClassPropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ByteArrayClassPropertyIterator(const QScriptValue &object);
      ~ByteArrayClassPropertyIterator();
      bool hasNext() const;
      void next();
      bool hasPrevious() const;
      void previous();
      void toFront();
      void toBack();
      QScriptString name() const;
      uint id() const;
      };

static qint32 toArrayIndex(const QString &str)
      {
      QByteArray bytes = str.toUtf8();
      char *eptr;
      quint32 pos = strtoul(bytes.constData(), &eptr, 10);
      if ((eptr == bytes.constData() + bytes.size()) && (QByteArray::number(pos) == bytes)) {
            return pos;
            }
      return -1;
      }

ByteArrayClass::ByteArrayClass(QScriptEngine *engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<QByteArray>(engine, toScriptValue, fromScriptValue);

      length = engine->toStringHandle(QLatin1String("length"));

      proto = engine->newQObject(new ByteArrayPrototype(this),
                                      QScriptEngine::QtOwnership,
                                      QScriptEngine::SkipMethodsInEnumeration
                                      | QScriptEngine::ExcludeSuperClassMethods
                                      | QScriptEngine::ExcludeSuperClassProperties);
      QScriptValue global = engine->globalObject();
      proto.setPrototype(global.property("Object").property("prototype"));

      ctor = engine->newFunction(construct);
      ctor.setData(qScriptValueFromValue(engine, this));
      }

ByteArrayClass::~ByteArrayClass()
      {
      }

QScriptClass::QueryFlags ByteArrayClass::queryProperty(const QScriptValue &object,
   const QScriptString &name, QueryFlags flags, uint *id)
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object.data());
      if (!ba)
            return 0;
      if (name == length) {
            return flags;
            }
      else {
            qint32 pos = toArrayIndex(name);
            if (pos == -1)
                  return 0;
            *id = pos;
            if ((flags & HandlesReadAccess) && (pos >= ba->size()))
                  flags &= ~HandlesReadAccess;
            return flags;
            }
      }

QScriptValue ByteArrayClass::property(const QScriptValue &object,
   const QScriptString &name, uint id)
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object.data());
      if (!ba)
            return QScriptValue();
      if (name == length) {
            return QScriptValue(engine(), ba->length());
            }
      else {
            qint32 pos = id;
            if ((pos < 0) || (pos >= ba->size()))
                  return QScriptValue();
            return QScriptValue(engine(), uint(ba->at(pos)) & 255);
            }
      return QScriptValue();
      }

void ByteArrayClass::setProperty(QScriptValue &object,
   const QScriptString &name, uint id, const QScriptValue &value)
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object.data());
      if (!ba)
            return;
      if (name == length) {
            ba->resize(value.toInt32());
            }
      else {
            qint32 pos = id;
            if (pos < 0)
                  return;
            if (ba->size() <= pos)
                  ba->resize(pos + 1);
            (*ba)[pos] = char(value.toInt32());
            }
      }

QScriptValue::PropertyFlags ByteArrayClass::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString &name, uint /*id*/)
      {
      if (name == length) {
            return QScriptValue::Undeletable | QScriptValue::SkipInEnumeration;
            }
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator *ByteArrayClass::newIterator(const QScriptValue &object)
      {
      return new ByteArrayClassPropertyIterator(object);
      }

QString ByteArrayClass::name() const
      {
      return QLatin1String("ByteArray");
      }

QScriptValue ByteArrayClass::prototype() const
      {
      return proto;
      }

QScriptValue ByteArrayClass::constructor()
      {
      return ctor;
      }

QScriptValue ByteArrayClass::newInstance(int size)
      {
      return newInstance(QByteArray(size, /*ch=*/0));
      }

QScriptValue ByteArrayClass::newInstance(const QByteArray &ba)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(ba));
      return engine()->newObject(this, data);
      }

QScriptValue ByteArrayClass::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ByteArrayClass *cls = qscriptvalue_cast<ByteArrayClass*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      int size = ctx->argument(0).toInt32();
      return cls->newInstance(size);
      }

QScriptValue ByteArrayClass::toScriptValue(QScriptEngine *eng, const QByteArray &ba)
      {
      QScriptValue ctor = eng->globalObject().property("ByteArray");
      ByteArrayClass *cls = qscriptvalue_cast<ByteArrayClass*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ByteArrayClass::fromScriptValue(const QScriptValue &obj, QByteArray &ba)
      {
      ba = qscriptvalue_cast<QByteArray>(obj.data());
      }

ByteArrayClassPropertyIterator::ByteArrayClassPropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

ByteArrayClassPropertyIterator::~ByteArrayClassPropertyIterator()
      {
      }

bool ByteArrayClassPropertyIterator::hasNext() const
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object().data());
      return m_index < ba->size();
      }

void ByteArrayClassPropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ByteArrayClassPropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ByteArrayClassPropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ByteArrayClassPropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ByteArrayClassPropertyIterator::toBack()
      {
      QByteArray *ba = qscriptvalue_cast<QByteArray*>(object().data());
      m_index = ba->size();
      m_last = -1;
      }

QScriptString ByteArrayClassPropertyIterator::name() const
      {
      return QScriptString();
      }

uint ByteArrayClassPropertyIterator::id() const
      {
      return m_last;
      }

QByteArray *ByteArrayPrototype::thisByteArray() const
      {
      return qscriptvalue_cast<QByteArray*>(thisObject().data());
      }

void ByteArrayPrototype::chop(int n)
      {
      thisByteArray()->chop(n);
      }

bool ByteArrayPrototype::equals(const QByteArray &other)
      {
      return *thisByteArray() == other;
      }

QByteArray ByteArrayPrototype::left(int len) const
      {
      return thisByteArray()->left(len);
      }

QByteArray ByteArrayPrototype::mid(int pos, int len) const
      {
      return thisByteArray()->mid(pos, len);
      }

QScriptValue ByteArrayPrototype::remove(int pos, int len)
      {
      thisByteArray()->remove(pos, len);
      return thisObject();
      }

QByteArray ByteArrayPrototype::right(int len) const
      {
      return thisByteArray()->right(len);
      }

QByteArray ByteArrayPrototype::simplified() const
      {
      return thisByteArray()->simplified();
      }

QByteArray ByteArrayPrototype::toBase64() const
      {
      return thisByteArray()->toBase64();
      }

QByteArray ByteArrayPrototype::toLower() const
      {
      return thisByteArray()->toLower();
      }

QByteArray ByteArrayPrototype::toUpper() const
      {
      return thisByteArray()->toUpper();
      }

QByteArray ByteArrayPrototype::trimmed() const
      {
      return thisByteArray()->trimmed();
      }

void ByteArrayPrototype::truncate(int pos)
      {
      thisByteArray()->truncate(pos);
      }

QString ByteArrayPrototype::toLatin1String() const
      {
      return QString::fromLatin1(*thisByteArray());
      }

QScriptValue ByteArrayPrototype::valueOf() const
      {
      return thisObject().data();
      }


