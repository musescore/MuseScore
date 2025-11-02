/*
MIT License

Copyright (c) Igor Korsukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <memory>
#include <string>
#include <cassert>

#include "async.h"
#include "internal/channelimpl.h"

namespace kors::async {
enum class PromiseType {
    AsyncByPromise,
    AsyncByBody
};

template<typename ... T>
class Promise;
template<typename ... T>
class Promise
{
public:

    struct Result;

    struct Resolve
    {
        Resolve() = default;

        Resolve(Promise<T...> _p)
            : p(_p) {}

        [[nodiscard]] Result operator ()(const T& ... val) const
        {
            p.resolve(val ...);
            return {};
        }

    private:
        mutable Promise<T...> p;
    };

    struct Reject
    {
        Reject() = default;

        Reject(Promise<T...> _p)
            : p(_p) {}

        [[nodiscard]] Result operator ()(int code, const std::string& msg) const
        {
            p.reject(code, msg);
            return {};
        }

    private:
        mutable Promise<T...> p;
    };

    // Dummy struct, with the purpose to enforce that the body
    // of a Promise resolves OR rejects exactly once
    struct Result {
        static Result unchecked()
        {
            return {};
        }

    private:
        Result() = default;

        friend struct Resolve;
        friend struct Reject;
    };

    static Result dummy_result() { return Result::unchecked(); }

    using BodyResolveReject = std::function<Result (Resolve, Reject)>;
    using BodyResolve = std::function<Result (Resolve)>;

    Promise(BodyResolveReject body, PromiseType type)
        : m_data(std::make_shared<Data>())
    {
        m_data->make_rejectCh();

        Resolve res(*this);
        Reject rej(*this);

        switch (type) {
        case PromiseType::AsyncByPromise:
            Async::call(nullptr, [res, rej](BodyResolveReject body) mutable {
                body(res, rej);
            }, body);
            break;

        case PromiseType::AsyncByBody:
            body(res, rej);
            break;
        }
    }

    Promise(BodyResolveReject body, const std::thread::id& th = std::this_thread::get_id())
        : m_data(std::make_shared<Data>())
    {
        m_data->make_rejectCh();

        Resolve res(*this);
        Reject rej(*this);

        Async::call(nullptr, [res, rej](BodyResolveReject body) mutable {
            body(res, rej);
        }, body, th);
    }

    Promise(BodyResolve body, PromiseType type)
        : m_data(std::make_shared<Data>())
    {
        Resolve res(*this);

        switch (type) {
        case PromiseType::AsyncByPromise:
            Async::call(nullptr, [res](BodyResolve body) mutable {
                body(res);
            }, body);
            break;

        case PromiseType::AsyncByBody:
            body(res);
            break;
        }
    }

    Promise(BodyResolve body, const std::thread::id& th = std::this_thread::get_id())
        : m_data(std::make_shared<Data>())
    {
        Resolve res(*this);

        Async::call(nullptr, [res](BodyResolve body) mutable {
            body(res);
        }, body, th);
    }

    Promise(const Promise& p)
        : m_data(p.m_data) {}

    Promise& operator=(const Promise& p)
    {
        m_data = p.m_data;
        return *this;
    }

    Promise<T...>& onResolve(const Asyncable* receiver, const std::function<void(const T&...)>& callback)
    {
        m_data->resolveCh.onReceive(receiver, [data { m_data }, callback](const std::shared_ptr<Data>& d, const T&... args) {
            (void)data;
            (void)d;
            assert(data == d);
            callback(args ...);
        }, Asyncable::Mode::SetOnce);
        return *this;
    }

    Promise<T...>& onReject(const Asyncable* receiver, std::function<void(int, const std::string&)> callback)
    {
        bool has_reject = m_data->rejectCh != nullptr;
        assert(has_reject && "This promise has no rejection");
        if (has_reject) {
            m_data->rejectCh->onReceive(receiver, [data { m_data }, callback](const std::shared_ptr<Data>& d, int code, const std::string& msg) {
                (void)data;
                (void)d;
                assert(data == d);
                callback(code, msg);
            }, Asyncable::Mode::SetOnce);
        }
        return *this;
    }

private:
    Promise() = default;

    void resolve(const T& ... args)
    {
        if (m_data->resolveCh.isConnected()) {
            // a promise is often used as a temporary object,
            // so let's store it in the message being sent so it arrives.
            m_data->resolveCh.send(SendMode::Auto, m_data, args ...);
        }
    }

    void reject(int code, const std::string& msg)
    {
        bool has_reject = m_data->rejectCh != nullptr;
        assert(has_reject && "This promise has no rejection");
        if (has_reject) {
            if (m_data->rejectCh->isConnected()) {
                // a promise is often used as a temporary object,
                // so let's store it in the message being sent so it arrives.
                m_data->rejectCh->send(SendMode::Auto, m_data, code, msg);
            }
        }
    }

    struct Data
    {
        ChannelImpl<std::shared_ptr<Data>, T...> resolveCh;
        std::unique_ptr<ChannelImpl<std::shared_ptr<Data>, int, std::string>> rejectCh;

        void make_rejectCh()
        {
            rejectCh = std::make_unique<ChannelImpl<std::shared_ptr<Data>, int, std::string>>();
        }
    };

    std::shared_ptr<Data> m_data;
};

template<typename ... T>
inline Promise<T...> make_promise(typename Promise<T...>::BodyResolveReject f, PromiseType type = PromiseType::AsyncByPromise)
{
    return Promise<T...>(f, type);
}

template<typename ... T>
inline Promise<T...> make_promise(typename Promise<T...>::BodyResolve f, PromiseType type = PromiseType::AsyncByPromise)
{
    return Promise<T...>(f, type);
}
}
