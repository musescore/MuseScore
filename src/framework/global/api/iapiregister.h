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
#ifndef MUSE_API_IAPIREGISTER_H
#define MUSE_API_IAPIREGISTER_H

#include "modularity/imoduleinterface.h"
#include "iapiengine.h"
#include "api/apiobject.h"

namespace muse::api {
class IApiRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(api::IApiRegister)
public:
    virtual ~IApiRegister() = default;

    struct ICreator {
        virtual ~ICreator() {}
        virtual ApiObject* create(IApiEngine* e) = 0;
        virtual bool isNeedDelete() const = 0;
    };

    virtual void regApiCreator(const std::string& module, const std::string& api, ICreator* c) = 0;
    virtual void regApiSingltone(const std::string& module, const std::string& api, ApiObject* o) = 0;
    virtual std::pair<ApiObject*, bool /*is need delete*/> createApi(const std::string& api, IApiEngine* e) const = 0;

    // dev
    struct Dump
    {
        enum class MethodType {
            Method = 0,
            Property
        };

        struct Arg {
            QString type;
            QString name;
        };

        struct Sig {
            QString name;
            QString retType;
            QList<Arg> args;
        };

        struct Method {
            MethodType type = MethodType::Method;
            Sig sig;
            QString doc;
        };

        struct Api {
            QString prefix;
            QList<Method> methods;
        };

        QList<Api> apis;
    };

    virtual Dump dump() const = 0;
};

template<class T>
struct ApiCreator : public IApiRegister::ICreator
{
    ApiObject* create(IApiEngine* e) override { return new T(e); }
    bool isNeedDelete() const override { return true; }
};
}

#endif // MUSE_API_IAPIREGISTER_H
