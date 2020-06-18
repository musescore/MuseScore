//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "settings.h"
#include <QSettings>

#include "log.h"

using namespace mu::framework;
using namespace mu::async;

Settings::Settings()
{
#ifndef Q_OS_MAC
    QSettings::setDefaultFormat(QSettings::IniFormat);
#endif
}

void Settings::addItem(const Item& item)
{
    m_items.insert({ item.key, item });
}

void Settings::addItem(const Key& key, const Val& val)
{
    m_items.insert({ key, Item(key, val) });
}

const Settings::Item& Settings::findItem(const Key& key) const
{
    auto it = m_items.find(key);
    if (it != m_items.end()) {
        return it->second;
    }

    static Item null;
    return null;
}

Settings::Item& Settings::findItem(const Key& key)
{
    auto it = m_items.find(key);
    if (it != m_items.end()) {
        return it->second;
    }

    static Item null;
    return null;
}

const std::map<Settings::Key, Settings::Item>& Settings::items() const
{
    return m_items;
}

void Settings::load()
{
    if (m_settings) {
        delete m_settings;
    }
    m_settings = new QSettings();

    std::map<QString, Item& > items;
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        Item& item = it->second;
        items.insert({ QString::fromStdString(item.key.key), item });
    }

    QStringList keys = m_settings->allKeys();
    for (const QString& key : keys) {
        auto it = items.find(key);
        if (it == items.end()) {
            LOGW() << "not found item with key: " << key;
            continue;
        }

        Item& item = it->second;
        item.val = Val::fromVariant(m_settings->value(key));
        item.val.type = item.defaultVal.type;
    }
}

Settings::Val Settings::value(const Key& key) const
{
    const Item& item = findItem(key);
    if (item.isNull()) {
        return Val::fromVariant(m_settings->value(QString::fromStdString(key.key)));
    }

    if (item.val.isNull()) {
        return item.defaultVal;
    }

    return item.val;
}

Settings::Val Settings::defaultValue(const Key& key) const
{
    return findItem(key).defaultVal;
}

void Settings::setValue(const Key& key, const Val& val)
{
    m_settings->setValue(QString::fromStdString(key.key), val.toVariant());

    Item& item = findItem(key);
    if (!item.isNull()) {
        item.val = val;
    }

    auto it = m_channels.find(key);
    if (it != m_channels.end()) {
        Channel<Val> ch = it->second;
        ch.send(val);
    }
}

Channel<Settings::Val> Settings::valueChanged(const Key& key) const
{
    return m_channels[key];
}

// Val
Settings::Val::Val(const char* str)
    : val(str), type(Type::String) {}

Settings::Val::Val(const std::string& str)
    : val(str), type(Type::String) {}

Settings::Val::Val(const std::string&& str)
    : val(std::move(str)), type(Type::String) {}

Settings::Val::Val(double val)
    : val(std::to_string(val)), type(Type::Double) {}

Settings::Val::Val(bool val)
    : val(std::to_string(val ? 1 : 0)), type(Type::Bool) {}

Settings::Val::Val(int val)
    : val(std::to_string(val)), type(Type::Int) {}

Settings::Val::Val(QColor val)
    : val(val.name().toStdString()), type(Type::Color) {}

bool Settings::Val::isNull() const
{
    return val.empty();
}

const std::string& Settings::Val::toString()const
{
    return val;
}

double Settings::Val::toDouble() const
{
    return std::stof(val);
}

bool Settings::Val::toBool() const
{
    if (val == "true") {
        return true;
    }

    if (val == "false") {
        return false;
    }

    return std::stoi(val);
}

int Settings::Val::toInt() const
{
    return std::stoi(val);
}

QColor Settings::Val::toQColor() const
{
    return QColor(val.c_str());
}

QVariant Settings::Val::toVariant() const
{
    switch (type) {
    case Settings::Val::Type::Undefined: return QVariant();
    case Settings::Val::Type::Bool: return QVariant(toBool());
    case Settings::Val::Type::Int: return QVariant(toInt());
    case Settings::Val::Type::Double: return QVariant(toDouble());
    case Settings::Val::Type::String: return QVariant(QString::fromStdString(toString()));
    case Settings::Val::Type::Color: return QVariant::fromValue(toQColor());
    }
    return QVariant();
}

Settings::Val Settings::Val::fromVariant(const QVariant& var)
{
    switch (var.type()) {
    case QVariant::Bool: return Settings::Val(var.toBool());
    case QVariant::Int: return Settings::Val(var.toInt());
    case QVariant::Double: return Settings::Val(var.toDouble());
    case QVariant::String: return Settings::Val(var.toString().toStdString());
    case QVariant::Color: return Settings::Val(var.value<QColor>());
    default:
        LOGE() << "not supported type: " << var.typeName() << ", val: " << var.toString();
        break;
    }
    return Settings::Val();
}
