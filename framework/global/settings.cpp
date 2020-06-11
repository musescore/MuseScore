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

static Settings::Val valFromVariant(const QVariant& var)
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

static QVariant valToVariant(const Settings::Val& val)
{
    switch (val.type) {
    case Settings::Val::Undefined: return QVariant();
    case Settings::Val::Bool: return QVariant(val.toBool());
    case Settings::Val::Int: return QVariant(val.toInt());
    case Settings::Val::Double: return QVariant(val.toDouble());
    case Settings::Val::String: return QVariant(QString::fromStdString(val.toString()));
    case Settings::Val::Color: return QVariant::fromValue(val.toColor());
    }
    return QVariant();
}

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

const Settings::Item& Settings::item(const Key& key) const
{
    auto it = m_items.find(key);
    if (it != m_items.end()) {
        return it->second;
    }

    static Item null;
    return null;
}

Settings::Item& Settings::item(const Key& key)
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
            LOGE() << "not found item with key: " << key;
            continue;
        }

        Item& item = it->second;
        item.val = valFromVariant(m_settings->value(key));
    }
}

Settings::Val Settings::value(const Key& key) const
{
    const Item& it = item(key);
    if (it.isNull()) {
        return valFromVariant(m_settings->value(QString::fromStdString(key.key)));
    }

    if (it.val.isNull()) {
        return it.defaultVal;
    }

    return it.val;
}

Settings::Val Settings::defaultValue(const Key& key) const
{
    return item(key).defaultVal;
}

void Settings::setValue(const Key& key, const Val& val)
{
    m_settings->setValue(QString::fromStdString(key.key), valToVariant(val));
    Item& it = item(key);
    if (!it.isNull()) {
        it.val = val;
    }
}

// Val
Settings::Val::Val(const char* str)
    : val(str) {}

Settings::Val::Val(const std::string& str)
    : val(str) {}

Settings::Val::Val(const std::string&& str)
    : val(std::move(str)) {}

Settings::Val::Val(double val)
    : val(std::to_string(val)) { }

Settings::Val::Val(bool val)
    : val(std::to_string(val ? 1 : 0)) {}

Settings::Val::Val(int val)
    : val(std::to_string(val)) {}

Settings::Val::Val(QColor val)
    : val(val.name().toStdString()) {}

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
    return std::stoi(val);
}

int Settings::Val::toInt() const
{
    return std::stoi(val);
}

QColor Settings::Val::toColor() const
{
    return QColor(val.c_str());
}
