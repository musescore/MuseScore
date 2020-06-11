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

    struct Val
    {
        enum Type {
            Undefined = 0,
            Bool,
            Int,
            Double,
            String,
            Color
        };

        std::string val;    //! NOTE In C++17 can be replaced by std::any or std::variant
        Type type = Undefined;

        Val() = default;

        explicit Val(const char* str);
        explicit Val(const std::string& str);
        explicit Val(const std::string&& str);
        explicit Val(double val);
        explicit Val(bool val);
        explicit Val(int val);
        explicit Val(QColor val);

        bool isNull() const;
        const std::string& toString() const;
        double toDouble() const;
        bool toBool() const;
        int toInt() const;
        QColor toColor() const;
    };

    struct Item
    {
        Key key;
        Val val;
        Val defaultVal;

        Item() = default;
        Item(const Key& k, const Val& defVal)
            : key(k), defaultVal(defVal) {}

        bool isNull() const { return key.isNull(); }
    };

    void addItem(const Item& item);
    void addItem(const Key& key, const Val& val);
    const Item& item(const Key& key) const;
    const std::map<Key, Item>& items() const;

    void load();

    Val value(const Key& key) const;
    Val defaultValue(const Key& key) const;
    void setValue(const Key& key, const Val& val);

private:

    Settings();

    Item& item(const Key& key);

    QSettings* m_settings = nullptr;
    std::map<Key, Item> m_items;
};

inline Settings* settings()
{
    return Settings::instance();
}
}
}

#endif // MU_FRAMEWORK_SETTINGS_H
