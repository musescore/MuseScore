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

using namespace mu;
using namespace mu::framework;
using namespace mu::async;

Settings::Settings()
{
#ifndef Q_OS_MAC
    QSettings::setDefaultFormat(QSettings::IniFormat);
#endif
    m_settings = new QSettings();
}

Settings::~Settings()
{
    delete m_settings;
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

/**
 * @brief Settings::reload method needed only for compatibility with the old MU preferences
 */
void Settings::reload()
{
    std::map<QString, Item& > items;
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        Item& item = it->second;
        items.insert({ QString::fromStdString(item.key.key), item });
    }

    QStringList keys = m_settings->allKeys();
    for (const QString& key : keys) {
        auto it = items.find(key);
        if (it == items.end()) {
            continue;
        }

        Item& item = it->second;

        QVariant val = m_settings->value(key);

        setValue(item.key, Val::fromQVariant(val));
    }
}

void Settings::load()
{
    std::map<QString, Item& > items;
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        Item& item = it->second;
        items.insert({ QString::fromStdString(item.key.key), item });
    }

    QStringList keys = m_settings->allKeys();
    for (const QString& key : keys) {
        auto it = items.find(key);
        if (it == items.end()) {
            // LOGW() << "not found item with key: " << key;
            continue;
        }

        Item& item = it->second;
        item.val = Val::fromQVariant(m_settings->value(key));
        item.val.setType(item.defaultVal.type());
    }
}

Val Settings::value(const Key& key) const
{
    const Item& item = findItem(key);
    if (item.isNull()) {
        return Val::fromQVariant(m_settings->value(QString::fromStdString(key.key)));
    }

    if (item.val.isNull()) {
        return item.defaultVal;
    }

    return item.val;
}

Val Settings::defaultValue(const Key& key) const
{
    return findItem(key).defaultVal;
}

void Settings::setValue(const Key& key, const Val& val)
{
    m_settings->setValue(QString::fromStdString(key.key), val.toQVariant());

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

Channel<Val> Settings::valueChanged(const Key& key) const
{
    return m_channels[key];
}
