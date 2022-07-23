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
#ifndef MU_FRAMEWORK_TRANSLATABLESTRING_H
#define MU_FRAMEWORK_TRANSLATABLESTRING_H

#include "translation.h"

#include "types/string.h"

#ifndef NO_QT_SUPPORT
#include <QString>
#endif

namespace mu {
//! Note: in order to make the string visible for `lupdate`,
//! you must write TranslatableString(...) explicitly.
struct TranslatableString {
    const char* context = nullptr;
    const char* str = nullptr;
    const char* disambiguation = nullptr;

    TranslatableString() = default;
    TranslatableString(const char* context, const char* str, const char* disambiguation = nullptr)
        : context(context), str(str), disambiguation(disambiguation) {}

    inline bool isValid() const
    {
        // Not null or empty
        return context && context[0] && str && str[0];
    }

    inline String translated(int n = -1) const
    {
        if (!isValid()) {
            return {};
        }

        String res = mtrc(context, str, disambiguation, n);

        for (auto arg : args) {
            arg->apply(res);
        }

        return res;
    }

    template<typename ... Args>
    inline TranslatableString arg(const Args& ...);

    template<typename ... Args>
    inline TranslatableString arg(Args&& ...);

private:
    struct IArg {
        virtual ~IArg() = default;
        virtual void apply(String& res) const = 0;
    };

    template<typename ... Args>
    struct Arg;

    template<typename ... Args>
    Arg(const Args&...)->Arg<std::enable_if_t<std::is_constructible_v<String, Args...>, String> >;

    std::vector<std::shared_ptr<const IArg> > args;
};

template<typename ... Args>
struct TranslatableString::Arg : public TranslatableString::IArg
{
    std::tuple<Args...> args;

    Arg(const Args&... args)
        : args(args ...) {}

    Arg(Args&&... args)
        : args(std::forward<Args>(args)...) {}

    void apply(String& res) const override
    {
        res = std::apply([&](const Args&... args) { return res.arg(args ...); }, args);
    }
};

template<typename Int>
struct TranslatableString::Arg<Int> : public TranslatableString::IArg
{
    const Int arg;

    Arg(Int arg) : arg(arg) {
    }

    void apply(String& res) const override
    {
        res = res.arg(arg);
    }
};

template<>
struct TranslatableString::Arg<String> : public TranslatableString::IArg
{
    const String arg;

    template<typename ... T>
    Arg(const T& ... arg) : arg(arg ...) {
    }

    template<typename ... T>
    Arg(T && ... arg) : arg(std::forward<T>(arg)...) {
    }

    void apply(String& res) const override
    {
        res = res.arg(arg);
    }
};

template<>
struct TranslatableString::Arg<TranslatableString> : public TranslatableString::IArg
{
    const TranslatableString arg;

    Arg(const TranslatableString& arg) : arg(arg) {
    }

    Arg(TranslatableString && arg) : arg(std::forward<TranslatableString>(arg)) {
    }

    void apply(String& res) const override
    {
        res = res.arg(arg.translated());
    }
};

template<typename ... Args>
TranslatableString TranslatableString::arg(const Args& ... args)
{
    TranslatableString res = *this;
    auto* a = new Arg(args ...);
    res.args.push_back(std::shared_ptr<std::decay_t<decltype(*a)> >(a));
    return res;
}

template<typename ... Args>
TranslatableString TranslatableString::arg(Args&& ... args)
{
    TranslatableString res = *this;
    auto* a = new Arg(std::forward<Args>(args) ...);
    res.args.push_back(std::shared_ptr<std::decay_t<decltype(*a)> >(a));
    return res;
}
}

#endif // MU_FRAMEWORK_TRANSLATABLESTRING_H
