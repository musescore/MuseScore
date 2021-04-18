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
#ifndef MU_FRAMEWORK_SETTINGS_H
#define MU_FRAMEWORK_SETTINGS_H

#include <string>
#include <vector>

#include "val.h"
#include "async/channel.h"

//! NOTE We are gradually abandoning Qt in non-GUI classes.
//! This settings interface is almost independent of Qt,
//! QSettings are used only in the implementation for compatibility with current settings.
//! Perhaps in the future this will be changed.

class QSettings;

namespace mu::framework {
class Settings
{
public:
    static Settings* instance();

    struct Key
    {
        std::string moduleName;
        std::string key;

        Key() = default;
        Key(std::string moduleName, std::string key);

        bool isNull() const;
        bool operator==(const Key& k) const;
        bool operator<(const Key& k) const;
    };

    struct Item
    {
        Key key;
        Val value;
        Val defaultValue;
        bool canBeMannualyEdited = false;

        bool isNull() const { return key.isNull(); }
    };

    using Items = std::map<Key, Item>;

    const Items& items() const;

    void reload();
    void load();

    void reset(bool keepDefaultSettings = false);

    Val value(const Key& key) const;
    Val defaultValue(const Key& key) const;

    void setValue(const Key& key, const Val& value);
    void setDefaultValue(const Key& key, const Val& value);
    void setCanBeMannualyEdited(const Settings::Key& key, bool canBeMannualyEdited);

    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();

    async::Channel<Val> valueChanged(const Key& key) const;

private:
    Settings();
    ~Settings();

    Item& findItem(const Key& key) const;
    async::Channel<Val>& findChannel(const Key& key) const;

    void insertNewItem(const Key& key, const Val& value);

    Items readItems() const;
    void writeValue(const Key& key, const Val& value);

    QString dataPath() const;

    QSettings* m_settings = nullptr;
    mutable Items m_items;
    mutable Items m_localSettings;
    mutable bool m_isTransactionStarted = false;
    mutable std::map<Key, async::Channel<Val> > m_channels;
};

inline Settings* settings()
{
    return Settings::instance();
}
}

#endif // MU_FRAMEWORK_SETTINGS_H
