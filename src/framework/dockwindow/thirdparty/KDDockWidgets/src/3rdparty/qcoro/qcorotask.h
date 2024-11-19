// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: MIT

// Only used by a KDDW Flutter build and only by the tests

#pragma once

#include "concepts_p.h"
#include "coroutine.h"

#include <atomic>
#include <cassert>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

namespace QCoro {

template<typename T = void>
class Task;

/*! \cond internal */

namespace detail {

template<typename T>
struct awaiter_type;

template<typename T>
using awaiter_type_t = typename awaiter_type<T>::type;

//! Continuation that resumes a coroutine co_awaiting on currently finished coroutine.
class TaskFinalSuspend {
public:
    //! Constructs the awaitable, passing it a handle to the co_awaiting coroutine
    /*!
     * \param[in] awaitingCoroutine handle of the coroutine that is co_awaiting the current
     * coroutine (continuation).
     */
    explicit TaskFinalSuspend(const std::vector<std::coroutine_handle<>> &awaitingCoroutines)
        : mAwaitingCoroutines(awaitingCoroutines) {}

    //! Returns whether the just finishing coroutine should do final suspend or not
    /*!
     * If the coroutine is not being co_awaited by another coroutine, then don't
     * suspend and let the code reach the end of the coroutine, which will take
     * care of cleaning everything up. Otherwise we suspend and let the awaiting
     * coroutine to clean up for us.
     */
    bool await_ready() const noexcept {
        return false;
    }

    //! Called by the compiler when the just-finished coroutine is suspended. for the very last time.
    /*!
     * It is given handle of the coroutine that is being co_awaited (that is the current
     * coroutine). If there is a co_awaiting coroutine and it has not been resumed yet,
     * it resumes it.
     *
     * Finally, it destroys the just finished coroutine and frees all allocated resources.
     *
     * \param[in] finishedCoroutine handle of the just finished coroutine
     */
    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> finishedCoroutine) noexcept {
        auto &promise = finishedCoroutine.promise();

        for (auto &awaiter : mAwaitingCoroutines) {
            awaiter.resume();
        }
        mAwaitingCoroutines.clear();

        // The handle will be destroyed here only if the associated Task has already been destroyed
        if (promise.setDestroyHandle()) {
            finishedCoroutine.destroy();
        }
    }

    //! Called by the compiler when the just-finished coroutine should be resumed.
    /*!
     * In reality this should never be called, as our coroutine is done, so it won't be resumed.
     * In any case, this method does nothing.
     * */
    constexpr void await_resume() const noexcept {}

private:
    std::vector<std::coroutine_handle<>> mAwaitingCoroutines;
};

//! Base class for the \c Task<T> promise_type.
/*!
 * This is a promise_type for a Task<T> returned from a coroutine. When a coroutine
 * is constructed, it looks at its return type, which will be \c Task<T>, and constructs
 * new object of type \c Task<T>::promise_type (which will be \c TaskPromise<T>). Using
 * \c TaskPromise<T>::get_return_object() it obtains a new object of Task<T>. Then the
 * coroutine user code is executed and runs until the it reaches a suspend point - either
 * a co_await keyword, co_return or until it reaches the end of user code.
 *
 * You can think about promise as an interface that is callee-facing, while Task<T> is
 * an caller-facing interface (in respect to the current coroutine).
 *
 * Promise interface must provide several methods:
 *  * get_return_object() - it is called by the compiler at the very beginning of a coroutine
 *    and is used to obtain the object that will be returned from the coroutine whenever it is
 *    suspended.
 *  * initial_suspend() - it is co_awaited by the code generated immediately before user
 *    code. Depending on the Awaitable that it returns, the coroutine will either suspend
 *    and the user code will only be executed once it is co_awaited by some other coroutine,
 *    or it will begin executing the user code immediately. In case of QCoro, the promise always
 *    returns std::suspend_never, which is a standard library awaitable, which prevents the
 *    coroutine from being suspended at the beginning.
 *  * final_suspend() - it is co_awaited when the coroutine co_returns, or when it reaches
 *    the end of user code. Same as with initial_suspend(), depending on the type of Awaitable
 *    it returns, it either suspends the coroutine (and then it must be destroyed explicitly
 *    by the Awaiter), or resumes and takes care of destroying the frame pointer. In case of
 *    QCoro, the promise returns a custom Awaitable called TaskFinalSuspend, which, when
 *    co_awaited by the compiler-generated code will make sure that if there is a coroutine
 *    co_awaiting on the current corutine, that the co_awaiting coroutine is resumed.
 *  * unhandled_exception() - called by the compiler if the coroutine throws an unhandled
 *    exception. The promise will store the exception and it will be rethrown when the
 *    co_awaiting coroutine tries to retrieve a result of the coroutine that has thrown.
 *  * return_value() - called by the compiler to store co_returned result of the function.
 *    It must only be present if the coroutine is not void.
 *  * return_void() - called by the compiler when the coroutine co_returns or flows of the
 *    end of user code. It must only be present if the coroutine return type is void.
 *  * await_transform() - this one is optional and is used by co_awaits inside the coroutine.
 *    It allows the promise to transform the co_awaited type to an Awaitable.
 */
class TaskPromiseBase {
public:
    //! Called when the coroutine is started to decide whether it should be suspended or not.
    /*!
     * We want coroutines that return QCoro::Task<T> to start automatically, because it will
     * likely be executed from Qt's event loop, which will not co_await it, but rather call
     * it as a regular function, therefore it returns `std::suspend_never` awaitable, which
     * indicates that the coroutine should not be suspended.
     * */
    std::suspend_never initial_suspend() const noexcept {
        return {};
    }

    //! Called when the coroutine co_returns or reaches the end of user code.
    /*!
     * This decides what should happen when the coroutine is finished.
     */
    auto final_suspend() const noexcept {
        return TaskFinalSuspend{mAwaitingCoroutines};
    }

    //! Called by co_await to obtain an Awaitable for type \c T.
    /*!
     * When co_awaiting on a value of type \c T, the type \c T must an Awaitable. To allow
     * to co_await even on types that are not Awaitable (e.g. 3rd party types like QNetworkReply),
     * C++ allows promise_type to provide \c await_transform() function that can transform
     * the type \c T into an Awaitable. This is a very powerful mechanism in C++ coroutines.
     *
     * For types \c T for which there is no valid await_transform() overload, the C++ attempts
     * to use those types directly as Awaitables. This is a perfectly valid scenario in cases
     * when co_awaiting a type that implements the neccessary Awaitable interface.
     *
     * In our implementation, the await_transform() is overloaded only for Qt types for which
     * a specialiation of the \c QCoro::detail::awaiter_type template class exists. The
     * specialization returns type of the Awaiter for the given type \c T.
     */
    template<typename T, typename Awaiter = QCoro::detail::awaiter_type_t<std::remove_cvref_t<T>>>
    auto await_transform(T &&value) {
        return Awaiter{value};
    }

    //! Specialized overload of await_transform() for Task<T>.
    /*!
     *
     * When co_awaiting on a value of type \c Task<T>, the co_await will try to obtain
     * an Awaitable object for the \c Task<T>. Since \c Task<T> already implements the
     * Awaitable interface it can be directly used as an Awaitable, thus this specialization
     * only acts as an identity operation.
     *
     * The reason we need to specialize it is that co_await will always call promise's await_transform()
     * overload if it exists. Since our generic await_transform() overload exists only for Qt types
     * that specialize the \c QCoro::detail::awaiter_type template, the compilation would fail
     * due to no suitable overload for \c QCoro::Task<T> being found.
     *
     * The reason the compiler doesn't check for an existence of await_transform() overload for type T
     * and falling back to using the T as an Awaitable, but instead only checks for existence of
     * any overload of await_transform() and failing compilation if non of the overloads is suitable
     * for type T is, that it allows us to delete await_transform() overload for a particular type,
     * and thus disallow that type to be co_awaited, even if the type itself provides the Awaitable
     * interface. This would not be possible if the compiler would always fall back to using the T
     * as an Awaitable type.
     */
    template<typename T>
    auto await_transform(QCoro::Task<T> &&task) {
        return std::forward<QCoro::Task<T>>(task);
    }

    //! \copydoc template<typename T> QCoro::TaskPromiseBase::await_transform(QCoro::Task<T> &&)
    template<typename T>
    auto &await_transform(QCoro::Task<T> &task) {
        return task;
    }

    //! If the type T is already an awaitable, then just forward it as it is.
    template<Awaitable T>
    auto &&await_transform(T &&awaitable) {
        return std::forward<T>(awaitable);
    }

    //! \copydoc template<Awaitable T> QCoro::TaskPromiseBase::await_transform(T &&)
    template<Awaitable T>
    auto &await_transform(T &awaitable) {
        return awaitable;
    }

    //! Called by \c TaskAwaiter when co_awaited.
    /*!
     * This function is called by a TaskAwaiter, e.g. an object obtain by co_await
     * when a value of Task<T> is co_awaited (in other words, when a coroutine co_awaits on
     * another coroutine returning Task<T>).
     *
     * \param awaitingCoroutine handle for the coroutine that is co_awaiting on a coroutine that
     *                          represented by this promise. When our coroutine finishes, it's
     *                          our job to resume the awaiting coroutine.
     */
    void addAwaitingCoroutine(std::coroutine_handle<> awaitingCoroutine) {
        mAwaitingCoroutines.push_back(awaitingCoroutine);
    }

    bool hasAwaitingCoroutine() const {
        return !mAwaitingCoroutines.empty();
    }

    bool setDestroyHandle() noexcept {
        return mDestroyHandle.exchange(true, std::memory_order_acq_rel);
    }

private:
    friend class TaskFinalSuspend;

    //! Handle of the coroutine that is currently co_awaiting this Awaitable
    std::vector<std::coroutine_handle<>> mAwaitingCoroutines;

    //! Indicates whether we can destroy the coroutine handle
    std::atomic<bool> mDestroyHandle{false};
};

//! The promise_type for Task<T>
/*!
 * See \ref TaskPromiseBase documentation for explanation about promise_type.
 */
template<typename T>
class TaskPromise final : public TaskPromiseBase {
public:
    explicit TaskPromise() = default;
    ~TaskPromise() = default;

    //! Constructs a Task<T> for this promise.
    Task<T> get_return_object() noexcept;

    //! Called by the compiler when user code throws an unhandled exception.
    /*!
     * When user code throws but doesn't catch, it is ultimately caught by the code generated by
     * the compiler (effectively the entire user code is wrapped in try...catch ) and this method
     * is called. This method stores the exception. The exception is re-thrown when the calling
     * coroutine is resumed and tries to retrieve result of the finished coroutine that has thrown.
     */
    void unhandled_exception() {
        mValue = std::current_exception();
    }

    //! Called form co_return statement to store result of the coroutine.
    /*!
     * \param[in] value the value returned by the coroutine. It is stored in the
     *            promise, later can be retrieved by the calling coroutine.
     */
    void return_value(T &&value) noexcept {
        mValue.template emplace<T>(std::forward<T>(value));
    }

    //! \copydoc template<typename T> TaskPromise::return_value(T &&value) noexcept
    void return_value(const T &value) noexcept {
        mValue = value;
    }

    template<typename U>
        requires QCoro::concepts::constructible_from<T, U>
    void return_value(U &&value) noexcept {
        mValue = std::move(value);
    }

    template<typename U>
        requires QCoro::concepts::constructible_from<T, U>
    void return_value(const U &value) noexcept {
        mValue = value;
    }

    //! Retrieves the result of the coroutine.
    /*!
     *  \return the value co_returned by the finished coroutine. If the coroutine has
     *  thrown an exception, this method will instead rethrow the exception.
     */
    T &result() & {
        if (std::holds_alternative<std::exception_ptr>(mValue)) {
            assert(std::get<std::exception_ptr>(mValue) != nullptr);
            std::rethrow_exception(std::get<std::exception_ptr>(mValue));
        }

        return std::get<T>(mValue);
    }

    //! \copydoc T &QCoro::TaskPromise<T>::result() &
    T &&result() && {
        if (std::holds_alternative<std::exception_ptr>(mValue)) {
            assert(std::get<std::exception_ptr>(mValue) != nullptr);
            std::rethrow_exception(std::get<std::exception_ptr>(mValue));
        }

        return std::move(std::get<T>(mValue));
    }

private:
    //! Holds either the return value of the coroutine or exception thrown by the coroutine.
    std::variant<std::monostate, T, std::exception_ptr> mValue;
};

//! Specialization of TaskPromise for coroutines returning \c void.
template<>
class TaskPromise<void> final : public TaskPromiseBase {
public:
    // Constructor.
    explicit TaskPromise() = default;

    //! Destructor.
    ~TaskPromise() = default;

    //! \copydoc TaskPromise<T>::get_return_object()
    Task<void> get_return_object() noexcept;

    //! \copydoc TaskPromise<T>::unhandled_exception()
    void unhandled_exception() {
        mException = std::current_exception();
    }

    //! Promise type must have this function when the coroutine return type is void.
    void return_void() noexcept {};

    //! Provides access to the result of the coroutine.
    /*!
     * Since this is a promise type for a void coroutine, the only result that
     * this can return is re-throwing an exception thrown by the coroutine, if
     * there's any.
     */
    void result() {
        if (mException) {
            std::rethrow_exception(mException);
        }
    }

private:
    //! Exception thrown by the coroutine.
    std::exception_ptr mException;
};

//! Base-class for Awaiter objects returned by the \c Task<T> operator co_await().
template<typename Promise>
class TaskAwaiterBase {
public:
    //! Returns whether to co_await
    bool await_ready() const noexcept {
        return !mAwaitedCoroutine || mAwaitedCoroutine.done();
    }

    //! Called by co_await in a coroutine that co_awaits our awaited coroutine managed by the current task.
    /*!
     * In other words, let's have a case like this:
     * \code{.cpp}
     * Task<> doSomething() {
     *    ...
     *    co_return result;
     * };
     *
     * Task<> getSomething() {
     *    ...
     *    const auto something = co_await doSomething();
     *    ...
     * }
     * \endcode
     *
     * If this Awaiter object is an awaiter of the doSomething() coroutine (e.g. has been constructed
     * by the co_await), then \c mAwaitedCoroutine is the handle of the doSomething() coroutine,
     * and \c awaitingCoroutine is a handle of the getSomething() coroutine which is awaiting the
     * completion of the doSomething() coroutine.
     *
     * This is implemented by passing the awaiting coroutine handle to the promise of the
     * awaited coroutine. When the awaited coroutine finishes, the promise will take care of
     * resuming the awaiting coroutine. At the same time this function resumes the awaited
     * coroutine.
     *
     * \param[in] awaitingCoroutine handle of the coroutine that is currently co_awaiting the
     * coroutine represented by this Tak.
     * \return returns whether the awaiting coroutine should be suspended, or whether the
     * co_awaited coroutine has finished synchronously and the co_awaiting coroutine doesn't
     * have to suspend.
     */
    void await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept {
        mAwaitedCoroutine.promise().addAwaitingCoroutine(awaitingCoroutine);
    }

protected:
    //! Constucts a new Awaiter object.
    /*!
     * \param[in] coroutine hande for the coroutine that is being co_awaited.
     */
    TaskAwaiterBase(std::coroutine_handle<Promise> awaitedCoroutine)
        : mAwaitedCoroutine(awaitedCoroutine) {}

    //! Handle of the coroutine that is being co_awaited by this awaiter
    std::coroutine_handle<Promise> mAwaitedCoroutine = {};
};

template<typename T>
struct isTask : std::false_type {
    using return_type = T;
};

template<typename T>
struct isTask<QCoro::Task<T>> : std::true_type {
    using return_type = typename QCoro::Task<T>::value_type;
};

template<typename T>
constexpr bool isTask_v = isTask<T>::value;

} // namespace detail

/*! \endcond */

//! An asynchronously executed task.
/*!
 * When a coroutine is called which has  return type Task<T>, the coroutine will
 * construct a new instance of Task<T> which will be returned to the caller when
 * the coroutine is suspended - that is either when it co_awaits another coroutine
 * of finishes executing user code.
 *
 * In the sense of the interface that the task implements, it is an Awaitable.
 *
 * Task<T> is constructed by a code generated at the beginning of the coroutine by
 * the compiler (i.e. before the user code). The code first creates a new frame
 * pointer, which internally holds the promise. The promise is of type \c R::promise_type,
 * where \c R is the return type of the function (so \c Task<T>, in our case.
 *
 * One can think about it as Task being the caller-facing interface and promise being
 * the callee-facing interface.
 */
template<typename T>
class Task {
public:
    //! Promise type of the coroutine. This is required by the C++ standard.
    using promise_type = detail::TaskPromise<T>;
    //! The type of the coroutine return value.
    using value_type = T;

    //! Constructs a new empty task.
    explicit Task() noexcept = default;

    //! Constructs a task bound to a coroutine.
    /*!
     * \param[in] coroutine handle of the coroutine that has constructed the task.
     */
    explicit Task(std::coroutine_handle<promise_type> coroutine) : mCoroutine(coroutine) {}

    //! Task cannot be copy-constructed.
    Task(const Task &) = delete;
    //! Task cannot be copy-assigned.
    Task &operator=(const Task &) = delete;

    //! The task can be move-constructed.
    Task(Task &&other) noexcept : mCoroutine(other.mCoroutine) {
        other.mCoroutine = nullptr;
    }

    //! The task can be move-assigned.
    Task &operator=(Task &&other) noexcept {
        if (std::addressof(other) != this) {
            if (mCoroutine) {
                // The coroutine handle will be destroyed only after TaskFinalSuspend
                if (mCoroutine.promise().setDestroyHandle()) {
                    mCoroutine.destroy();
                }
            }

            mCoroutine = other.mCoroutine;
            other.mCoroutine = nullptr;
        }
        return *this;
    }

    //! Destructor.
    ~Task() {
        if (!mCoroutine)
            return;

        // The coroutine handle will be destroyed only after TaskFinalSuspend
        if (mCoroutine.promise().setDestroyHandle()) {
            mCoroutine.destroy();
        }
    };

    //! Returns whether the task has finished.
    /*!
     * A task that is ready (represents a finished coroutine) must not attempt
     * to suspend the coroutine again.
     */
    bool isReady() const {
        return !mCoroutine || mCoroutine.done();
    }

    //! Provides an Awaiter for the coroutine machinery.
    /*!
     * The coroutine machinery looks for a suitable operator co_await overload
     * for the current Awaitable (this Task). It calls it to obtain an Awaiter
     * object, that is an object that the co_await keyword uses to suspend and
     * resume the coroutine.
     */
    auto operator co_await() const noexcept {
        //! Specialization of the TaskAwaiterBase that returns the promise result by value
        class TaskAwaiter : public detail::TaskAwaiterBase<promise_type> {
        public:
            TaskAwaiter(std::coroutine_handle<promise_type> awaitedCoroutine)
                : detail::TaskAwaiterBase<promise_type>{awaitedCoroutine} {}

            //! Called when the co_awaited coroutine is resumed.
            /*
             * \return the result from the coroutine's promise, factically the
             * value co_returned by the coroutine. */
            auto await_resume() {
                assert(this->mAwaitedCoroutine != nullptr);
                if constexpr (!std::is_void_v<T>) {
                    return std::move(this->mAwaitedCoroutine.promise().result());
                } else {
                    // Wil re-throw exception, if any is stored
                    this->mAwaitedCoroutine.promise().result();
                }
            }
        };

        return TaskAwaiter{mCoroutine};
    }

    //! A callback to be invoked when the asynchronous task finishes.
    /*!
     * In some scenarios it is not possible to co_await a coroutine (for example from
     * a third-party code that cannot be changed to be a coroutine). In that case,
     * chaining a then() callback is a possible solution how to handle a result
     * of a coroutine without co_awaiting it.
     *
     * @param callback A function or a function object that can be invoked without arguments
     * (discarding the result of the coroutine) or with a single argument of type T that is
     * type matching the return type of the coroutine or is implicitly constructible from
     * the return type returned by the coroutine.
     *
     * @return Returns Task<R> where R is the return type of the callback, so that the
     * result of the then() action can be co_awaited, if desired. If the callback
     * returns an awaitable (Task<R>) then the result of then is the awaitable.
     */
    template<typename ThenCallback>
        requires(std::is_invocable_v<ThenCallback> ||
                 (!std::is_void_v<T> && std::is_invocable_v<ThenCallback, T>))
    auto then(ThenCallback &&callback) {
        // Provide a custom error handler that simply re-throws the current exception
        return thenImpl(std::forward<ThenCallback>(callback), [](const auto &) { throw; });
    }

    template<typename ThenCallback, typename ErrorCallback>
        requires((std::is_invocable_v<ThenCallback> ||
                  (!std::is_void_v<T> && std::is_invocable_v<ThenCallback, T>)) &&
                 std::is_invocable_v<ErrorCallback, const std::exception &>)
    auto then(ThenCallback &&callback, ErrorCallback &&errorCallback) {
        return thenImpl(std::forward<ThenCallback>(callback),
                        std::forward<ErrorCallback>(errorCallback));
    }

private:
    template<typename ThenCallback, typename... Args>
    auto invokeCb(ThenCallback &&callback, [[maybe_unused]] Args &&...args) {
        if constexpr (std::is_invocable_v<ThenCallback, Args...>) {
            return callback(std::forward<Args>(args)...);
        } else {
            return callback();
        }
    }

    template<typename ThenCallback, typename Arg>
    struct cb_invoke_result
        : std::conditional_t<std::is_invocable_v<ThenCallback>, std::invoke_result<ThenCallback>,
                             std::invoke_result<ThenCallback, Arg>> {};

    template<typename ThenCallback>
    struct cb_invoke_result<ThenCallback, void> : std::invoke_result<ThenCallback> {};

    template<typename ThenCallback, typename Arg>
    using cb_invoke_result_t = typename cb_invoke_result<ThenCallback, Arg>::type;

    template<typename R, typename ErrorCallback,
             typename U = typename detail::isTask<R>::return_type>
    auto handleException(ErrorCallback &errCb, const std::exception &exception) -> U {
        errCb(exception);
        if constexpr (!std::is_void_v<U>) {
            return U{};
        }
    }

    template<typename ThenCallback, typename ErrorCallback,
             typename R = cb_invoke_result_t<ThenCallback, T>>
    auto thenImpl(ThenCallback &&thenCallback, ErrorCallback &&errorCallback)
        -> std::conditional_t<detail::isTask_v<R>, R, Task<R>> {
        auto thenCb = std::forward<ThenCallback>(thenCallback);
        auto errCb = std::forward<ErrorCallback>(errorCallback);
        if constexpr (std::is_void_v<value_type>) {
            try {
                co_await *this;
            } catch (const std::exception &e) {
                co_return handleException<R>(errCb, e);
            }
            if constexpr (detail::isTask_v<R>) {
                co_return co_await invokeCb(thenCb);
            } else {
                co_return invokeCb(thenCb);
            }
        } else {
            std::optional<T> value;
            try {
                value = co_await *this;
            } catch (const std::exception &e) {
                co_return handleException<R>(errCb, e);
            }
            if constexpr (detail::isTask_v<R>) {
                co_return co_await invokeCb(thenCb, std::move(*value));
            } else {
                co_return invokeCb(thenCb, std::move(*value));
            }
        }
    }

private:
    //! The coroutine represented by this task
    /*!
     * In other words, this is a handle to the coroutine that has constructed and
     * returned this Task<T>.
     * */
    std::coroutine_handle<promise_type> mCoroutine = {};
};

namespace detail {

template<typename T>
inline Task<T> TaskPromise<T>::get_return_object() noexcept {
    return Task<T>{std::coroutine_handle<TaskPromise>::from_promise(*this)};
}

Task<void> inline TaskPromise<void>::get_return_object() noexcept {
    return Task<void>{std::coroutine_handle<TaskPromise>::from_promise(*this)};
}

template<typename T>
concept TaskConvertible = requires(T v, TaskPromiseBase t) {
                              { t.await_transform(v) };
                          };

template<typename T>
struct awaitable_return_type {
    using type = std::decay_t<decltype(std::declval<T>().await_resume())>;
};

template<QCoro::detail::has_member_operator_coawait T>
struct awaitable_return_type<T> {
    using type = std::decay_t<
        typename awaitable_return_type<decltype(std::declval<T>().operator co_await())>::type>;
};

template<QCoro::detail::has_nonmember_operator_coawait T>
struct awaitable_return_type<T> {
    using type = std::decay_t<
        typename awaitable_return_type<decltype(operator co_await(std::declval<T>()))>::type>;
};

template<Awaitable Awaitable>
using awaitable_return_type_t = typename detail::awaitable_return_type<Awaitable>::type;

template<typename Awaitable>
    requires TaskConvertible<Awaitable>
using convertible_awaitable_return_type_t =
    typename detail::awaitable_return_type<decltype(std::declval<TaskPromiseBase>().await_transform(
        Awaitable()))>::type;

template<typename Awaitable>
    requires TaskConvertible<Awaitable>
auto toTask(Awaitable &&future)
    -> QCoro::Task<detail::convertible_awaitable_return_type_t<Awaitable>> {
    co_return co_await future;
}

struct WaitContext {
    // QEventLoop loop;
    bool coroutineFinished = false;
    std::exception_ptr exception;
};

} // namespace detail

template<typename T, typename QObjectSubclass, typename Callback>
    requires detail::TaskConvertible<T> &&
             (std::is_invocable_v<Callback> ||
              std::is_invocable_v<Callback, detail::convertible_awaitable_return_type_t<T>> ||
              std::is_invocable_v<Callback, QObjectSubclass *> ||
              std::is_invocable_v<Callback, QObjectSubclass *,
                                  detail::convertible_awaitable_return_type_t<T>>) &&
             (!detail::isTask_v<T>)
void connect(T &&future, QObjectSubclass *context, Callback func) {
    auto task = detail::toTask(std::move(future));
    connect(std::move(task), context, func);
}

} // namespace QCoro
