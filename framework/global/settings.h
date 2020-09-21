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
#include <map>
#include <functional>
#include <QColor>

#include "val.h"
#include "async/channel.h"

//! NOTE We are gradually abandoning Qt in non-GUI classes.
//! This settings interface is almost independent of Qt,
//! in the future, the `QColor` can be replaced with its own structure.
//! QSettings are used only in the implementation for compatibility with current settings.
//! Perhaps in the future this will be changed.

class QSettings;

namespace mu {
namespace framework {
class Settings
{
public:

    static Settings* instance()
    {
        static Settings s;
        return &s;
    }

    struct Key
    {
        std::string module;
        std::string key;
        Key() = default;
        Key(const std::string& m, const std::string& k)
            : module(m), key(k)
        {}

        bool isNull() const { return module.empty() || key.empty(); }

        bool operator ==(const Key& k) const { return module == k.module && key == k.key; }

        bool operator <(const Key& k) const
        {
            if (module != k.module) {
                return module < k.module;
            }
            return key < k.key;
        }
    };

    struct Item
    {
        Key key;
        Val val;
        Val defaultVal;

        Item() = default;
        Item(const Key& k, const Val& defVal)
            : key(k), val(defVal), defaultVal(defVal) {}

        bool isNull() const { return key.isNull(); }
    };

    void addItem(const Item& findItem);
    void addItem(const Key& key, const Val& val);
    const Item& findItem(const Key& key) const;
    const std::map<Key, Item>& items() const;

    void reload();
    void load();

    Val value(const Key& key) const;
    Val defaultValue(const Key& key) const;
    void setValue(const Key& key, const Val& val);
    async::Channel<Val> valueChanged(const Key& key) const;

private:

    Settings();
    ~Settings();

    Item& findItem(const Key& key);

    QSettings* m_settings = nullptr;
    std::map<Key, Item> m_items;
    mutable std::map<Key, async::Channel<Val> > m_channels;
};

inline Settings* settings()
{
    return Settings::instance();
}
}
}

#endif // MU_FRAMEWORK_SETTINGS_H
