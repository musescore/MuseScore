/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "settings.h"
#include "config.h"
#include "log.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>

#include "multiinstances/resourcelockguard.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::async;

static const std::string MULTI_INSTANCES_LOCK_NAME("settings");

Settings* Settings::instance()
{
    static Settings s;
    return &s;
}

Settings::Settings()
{
#if defined(WIN_PORTABLE)
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, dataPath());
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, dataPath());
#endif

#ifndef Q_OS_MAC
    QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

    m_settings = new QSettings();
}

Settings::~Settings()
{
    delete m_settings;
}

io::path Settings::filePath() const
{
    return m_settings->fileName();
}

const Settings::Items& Settings::items() const
{
    return m_isTransactionStarted ? m_localSettings : m_items;
}

/**
 * @brief Settings::reload method needed only for compatibility with the old MU preferences
 */
void Settings::reload()
{
    Items items = readItems();

    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        setSharedValue(it->first, it->second.value);
    }
}

void Settings::load()
{
    m_items = readItems();
}

void Settings::reset(bool keepDefaultSettings)
{
    m_settings->clear();

    m_isTransactionStarted = false;
    m_localSettings.clear();

    if (!keepDefaultSettings) {
        QDir(dataPath()).removeRecursively();
        QDir().mkpath(dataPath());
    }

    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        it->second.value = it->second.defaultValue;

        Channel<Val>& channel = findChannel(it->first);
        channel.send(it->second.value);
    }
}

Settings::Items Settings::readItems() const
{
    Items result;

    mi::ResourceLockGuard resource_lock(multiInstancesProvider(), MULTI_INSTANCES_LOCK_NAME);

    for (const QString& key : m_settings->allKeys()) {
        Item item;
        item.key = Key(std::string(), key.toStdString());
        item.value = Val::fromQVariant(m_settings->value(key));

        result[item.key] = item;
    }

    return result;
}

Val Settings::value(const Key& key) const
{
    return findItem(key).value;
}

Val Settings::defaultValue(const Key& key) const
{
    return findItem(key).defaultValue;
}

void Settings::setSharedValue(const Key& key, const Val& value)
{
    setLocalValue(key, value);

    if (multiInstancesProvider()) {
        multiInstancesProvider()->settingsSetValue(key.key, value);
    }
}

void Settings::setLocalValue(const Key& key, const Val& value)
{
    Item& item = findItem(key);

    if (!item.isNull() && item.value == value) {
        return;
    }

    if (!m_isTransactionStarted) {
        writeValue(key, value);
    }

    if (item.isNull()) {
        insertNewItem(key, value);
    } else {
        item.value = value;
    }

    auto it = m_channels.find(key);
    if (it != m_channels.end()) {
        async::Channel<Val> channel = it->second;
        channel.send(value);
    }
}

void Settings::writeValue(const Key& key, const Val& value)
{
    mi::ResourceLockGuard resource_lock(multiInstancesProvider(), MULTI_INSTANCES_LOCK_NAME);

    // TODO: implement writing/reading first part of key (module name)
    m_settings->setValue(QString::fromStdString(key.key), value.toQVariant());
}

QString Settings::dataPath() const
{
#if defined(WIN_PORTABLE)
    return QDir::cleanPath(QString("%1/../../../Data/settings")
                           .arg(QCoreApplication::applicationDirPath())
                           .arg(QCoreApplication::applicationName()));
#else
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
}

void Settings::setDefaultValue(const Key& key, const Val& value)
{
    Item& item = findItem(key);

    if (item.isNull()) {
        m_items[key] = Item{ key, value, value };
    } else {
        item.defaultValue = value;
        item.value.setType(value.type());
    }
}

void Settings::setCanBeMannualyEdited(const Settings::Key& key, bool canBeMannualyEdited)
{
    Item& item = findItem(key);

    if (item.isNull()) {
        m_items[key] = Item{ key, Val(), Val(), canBeMannualyEdited };
    } else {
        item.canBeMannualyEdited = canBeMannualyEdited;
    }
}

void Settings::insertNewItem(const Settings::Key& key, const Val& value)
{
    Item item = Item{ key, value, value };
    if (m_isTransactionStarted) {
        m_localSettings[key] = item;
    } else {
        m_items[key] = item;
    }
}

void Settings::beginTransaction(bool notifyToOtherInstances)
{
    if (m_isTransactionStarted) {
        LOGW() << "Transaction is already started";
        return;
    }

    m_localSettings = m_items;
    m_isTransactionStarted = true;

    if (notifyToOtherInstances && multiInstancesProvider()) {
        multiInstancesProvider()->settingsBeginTransaction();
    }
}

void Settings::commitTransaction(bool notifyToOtherInstances)
{
    m_isTransactionStarted = false;

    for (auto it = m_localSettings.begin(); it != m_localSettings.end(); ++it) {
        Item& item = findItem(it->first);
        if (item.value == it->second.value) {
            continue;
        }

        if (item.isNull()) {
            insertNewItem(it->first, it->second.value);
        } else {
            item.value = it->second.value;
        }

        writeValue(it->first, it->second.value);
    }

    m_localSettings.clear();

    if (notifyToOtherInstances && multiInstancesProvider()) {
        multiInstancesProvider()->settingsCommitTransaction();
    }
}

void Settings::rollbackTransaction(bool notifyToOtherInstances)
{
    m_isTransactionStarted = false;

    for (auto it = m_localSettings.begin(); it != m_localSettings.end(); ++it) {
        Item item = findItem(it->first);
        if (item.value == it->second.value) {
            continue;
        }

        Channel<Val>& channel = findChannel(it->first);
        channel.send(item.value);
    }

    m_localSettings.clear();

    if (notifyToOtherInstances && multiInstancesProvider()) {
        multiInstancesProvider()->settingsRollbackTransaction();
    }
}

Settings::Item& Settings::findItem(const Key& key) const
{
    Items& items = m_isTransactionStarted ? m_localSettings : m_items;

    auto it = items.find(key);

    if (it == items.end()) {
        static Item null;
        return null;
    }

    return it->second;
}

async::Channel<Val>& Settings::findChannel(const Settings::Key& key) const
{
    auto it = m_channels.find(key);

    if (it == m_channels.end()) {
        static async::Channel<Val> null;
        return null;
    }

    return it->second;
}

async::Channel<Val> Settings::valueChanged(const Key& key) const
{
    return m_channels[key];
}

Settings::Key::Key(std::string moduleName, std::string key)
    : moduleName(std::move(moduleName)), key(std::move(key))
{
}

bool Settings::Key::operator==(const Key& k) const
{
    return key == k.key;
}

bool Settings::Key::operator<(const Key& k) const
{
    return key < k.key;
}

bool Settings::Key::isNull() const
{
    return key.empty();
}
