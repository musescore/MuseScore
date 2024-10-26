/*
  This file is part of KDBindings.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <kdbindings/node.h>
#include <kdbindings/node_operators.h>
#include <kdbindings/node_functions.h>
#include <kdbindings/make_node.h>
#include <kdbindings/binding_evaluator.h>
#include <kdbindings/property_updater.h>

namespace KDBindings {

/**
 * @brief A combination of a root Node with an evaluator.
 *
 * A root Node is formed whenever multiple properties are combined inside
 * a expression and an evaluator is responsible for re-evaluating such
 * an expression whenever any of the constituent properties change.
 *
 * @tparam T The type of the value that the Binding expression evaluates to.
 * @tparam EvaluatorT The type of the evaluator that is used to evaluate the Binding.
 */
template<typename T, typename EvaluatorT = BindingEvaluator>
class Binding : public PropertyUpdater<T>, public Private::Dirtyable
{
    static_assert(
            std::is_base_of<BindingEvaluator, EvaluatorT>::value,
            "The EvaluatorT type must inherit from BindingEvaluator.");

public:
    /**
     * @brief Construct a new Binding with a specific evaluator.
     *
     * @param rootNode Represents that expression contained in the Binding.
     * @param evaluator Used to evaluate the expression contained in the Binding.
     */
    explicit Binding(Private::Node<T> &&rootNode, EvaluatorT const &evaluator)
        : m_rootNode{ std::move(rootNode) }
        , m_evaluator{ evaluator }
    {
        m_bindingId = m_evaluator.insert(this);
        m_rootNode.setParent(this);
    }

    /** Destructs the Binding by deregistering it from its evaluator. */
    ~Binding() override
    {
        m_evaluator.remove(m_bindingId);
    }

    /** A Binding is not default constructible. */
    Binding() = delete;

    /** A Binding cannot be copy constructed. */
    Binding(Binding const &other) = delete;
    /** A Binding cannot be copy assigned. */
    Binding &operator=(Binding const &other) = delete;

    // Move construction would invalidate the this pointer passed to the evaluator
    // in the constructor
    /** A Binding can not be move constructed. */
    Binding(Binding &&other) = delete;
    /** A Binding can not be move assigned. */
    Binding &operator=(Binding &&other) = delete;

    /** Set the function that should be used to notify
     * associated properties when the Binding re-evaluates.
     */
    void setUpdateFunction(std::function<void(T &&)> const &updateFunction) override
    {
        m_propertyUpdateFunction = updateFunction;
    }

    /** Returns the current value of the Binding. */
    T get() const override { return m_rootNode.evaluate(); }

    /** Re-evaluates the value of the Binding and notifies all dependants of the change. */
    void evaluate()
    {
        T value = m_rootNode.evaluate();

        // Use this to update any associated property via the PropertyUpdater's update function
        m_propertyUpdateFunction(std::move(value));
    }

protected:
    Private::Dirtyable **parentVariable() override { return nullptr; }
    const bool *dirtyVariable() const override { return nullptr; }

    /** The root Node of the Binding represents the expression contained by the Binding. */
    Private::Node<T> m_rootNode;
    /** The evaluator responsible for evaluating this Binding. */
    EvaluatorT m_evaluator;
    /** The function used to notify associated properties when the Binding re-evaluates */
    std::function<void(T &&)> m_propertyUpdateFunction = [](T &&) {};
    /** The id of the Binding, used for keeping track of the Binding in its evaluator. */
    int m_bindingId = -1;
};

/**
 * @brief Helper function to create a Binding from a Property.
 *
 * @tparam T The type of the value that the Binding expression evaluates to.
 * @tparam EvaluatorT The type of the evaluator that is used to evaluate the Binding.
 * @param evaluator The evaluator that is used to evaluate the Binding.
 * @param property The Property to create a Binding from.
 * @return std::unique_ptr<Binding<T, EvaluatorT>> A new Binding that is powered by the evaluator.
 *
 * *Note: For the difference between makeBinding and makeBoundProperty, see the
 * ["Reassigning a Binding"](../../getting-started/data-binding/#reassigning-a-binding) section in the Getting Started guide.*
 */
template<typename T, typename EvaluatorT>
inline std::unique_ptr<Binding<T, EvaluatorT>> makeBinding(EvaluatorT &evaluator, Property<T> &property)
{
    return std::make_unique<Binding<T, EvaluatorT>>(Private::makeNode(property), evaluator);
}

/**
 * @brief Helper function to create a Binding from a root Node.
 *
 * @tparam T The type of the value that the Binding expression evaluates to.
 * @tparam EvaluatorT The type of the evaluator that is used to evaluate the Binding.
 * @param evaluator The evaluator that is used to evaluate the Binding.
 * @param rootNode Represents the expression that will be evaluated by the Binding.
 * @return std::unique_ptr<Binding<T, EvaluatorT>> A new Binding that combines the rootNode with the evaluator.
 *
 * *Note: For the difference between makeBinding and makeBoundProperty, see the
 * ["Reassigning a Binding"](../../getting-started/data-binding/#reassigning-a-binding) section in the Getting Started guide.*
 */
template<typename T, typename EvaluatorT>
inline std::unique_ptr<Binding<T, EvaluatorT>> makeBinding(EvaluatorT &evaluator, Private::Node<T> &&rootNode)
{
    return std::make_unique<Binding<T, EvaluatorT>>(std::move(rootNode), evaluator);
}

/**
 * @brief Helper function to create a Binding from a function and its arguments.
 *
 * @tparam EvaluatorT The type of the evaluator that is used to evaluate the Binding.
 * @param evaluator The evaluator that is used to evaluate the Binding.
 * @tparam Func The type of the function - may be any type that implements operator().
 * @param func The function object.
 * @tparam Args The function argument types
 * @param args The function arguments - Possible values include: Properties, Constants and Nodes
 *              They will be automatically unwrapped, i.e. a Property<T> will pass a value of type T to func.
 * @return std::unique_ptr<Binding<ReturnType, EvaluatorT>> where ReturnType is the type that results from evaluationg func with the given arguments.
 *          The Binding will be powered by the new evaluator.
 *
 * *Note: For the difference between makeBinding and makeBoundProperty, see the
 * ["Reassigning a Binding"](../../getting-started/data-binding/#reassigning-a-binding) section in the Getting Started guide.*
 */
template<typename EvaluatorT, typename Func, typename... Args, typename = std::enable_if_t<sizeof...(Args) != 0>, typename ResultType = Private::operator_node_result_t<Func, Args...>>
inline std::unique_ptr<Binding<ResultType, EvaluatorT>> makeBinding(EvaluatorT &evaluator, Func &&func, Args &&...args)
{
    return std::make_unique<Binding<ResultType, EvaluatorT>>(Private::makeNode(std::forward<Func>(func), std::forward<Args>(args)...), evaluator);
}

/**
 * @brief Provides a convenience for old-school, immediate mode Bindings.
 *
 * This works in conjunction with a do-nothing ImmediateBindingEvaluator class to update the
 * result of the Binding immediately upon any of the dependent bindables (i.e. Property instances)
 * notifying that they have changed. This can lead to a Property Binding being evaluated many
 * times before the result is ever used in a typical GUI application.
 *
 * @tparam T The type of the value that the Binding expression evaluates to.
 */
template<typename T>
class Binding<T, ImmediateBindingEvaluator> : public Binding<T, BindingEvaluator>
{
public:
    /**
     * @brief Construct a new Binding with an immediate mode evaluator.
     *
     * @param rootNode Represents that expression contained in the Binding.
     */
    explicit Binding(Private::Node<T> &&rootNode)
        : Binding<T, BindingEvaluator>(std::move(rootNode), ImmediateBindingEvaluator::instance())
    {
    }

    /** A Binding is not default constructible. */
    Binding() = delete;

    virtual ~Binding() = default;

    /** A Binding cannot be copy constructed. */
    Binding(Binding const &other) = delete;
    /** A Binding cannot be copy assigned. */
    Binding &operator=(Binding const &other) = delete;

    /** A Binding can not be move constructed. */
    Binding(Binding &&other) = delete;
    /** A Binding can not be move assigned. */
    Binding &operator=(Binding &&other) = delete;

    void markDirty() override
    {
        Binding::evaluate();
    }
};

/**
 * @brief Helper function to create an immediate mode Binding from a Property.
 *
 * @tparam T The type of the value that the Binding expression evaluates to.
 * @param property The Property to create a Binding from.
 * @return std::unique_ptr<Binding<T, ImmediateBindingEvaluator>>
 *        An new Binding bound to an existing Property with immediate evaluation.
 *
 * *Note: For the difference between makeBinding and makeBoundProperty, see the
 * ["Reassigning a Binding"](../../getting-started/data-binding/#reassigning-a-binding) section in the Getting Started guide.*
 */
template<typename T>
inline std::unique_ptr<Binding<T, ImmediateBindingEvaluator>> makeBinding(Property<T> &property)
{
    return std::make_unique<Binding<T, ImmediateBindingEvaluator>>(Private::makeNode(property));
}

/**
 * @brief Helper function to create an immediate mode Binding from a root Node.
 *
 * @tparam T The type of the value that the Binding expression evaluates to.
 * @param rootNode Represents the expression that will be evaluated by the Binding.
 *                  Typically constructed from a unary/binary operator on a Property.
 * @return std::unique_ptr<Binding<<T, ImmediateBindingEvaluator>> An new Binding bound to a root Node with immediate evaluation.
 *
 * *Note: For the difference between makeBinding and makeBoundProperty, see the
 * ["Reassigning a Binding"](../../getting-started/data-binding/#reassigning-a-binding) section in the Getting Started guide.*
 */
template<typename T>
inline std::unique_ptr<Binding<T, ImmediateBindingEvaluator>> makeBinding(Private::Node<T> &&rootNode)
{
    return std::make_unique<Binding<T, ImmediateBindingEvaluator>>(std::move(rootNode));
}

/**
 * @brief Helper function to create an immediate mode Binding from a function and its arguments.
 *
 * @tparam Func The type of the function - may be any type that implements operator().
 * @param func The function object.
 * @tparam Args The function argument types
 * @param args The function arguments - Possible values include: Properties, Constants and Nodes
 *              They will be automatically unwrapped, i.e. a Property<T> will pass a value of type T to func.
 * @return std::unique_ptr<Binding<ReturnType, ImmediateBindingEvaluator>> where ReturnType is the type that results from evaluationg func with the given arguments.
 *          The Binding will feature immediate evaluation.
 *
 * *Note: For the difference between makeBinding and makeBoundProperty, see the
 * ["Reassigning a Binding"](../../getting-started/data-binding/#reassigning-a-binding) section in the Getting Started guide.*
 */
template<typename Func, typename... Args, typename = std::enable_if_t<sizeof...(Args) != 0>, typename ResultType = Private::operator_node_result_t<Func, Args...>>
inline std::unique_ptr<Binding<ResultType, ImmediateBindingEvaluator>> makeBinding(Func &&func, Args &&...args)
{
    return std::make_unique<Binding<ResultType, ImmediateBindingEvaluator>>(Private::makeNode(std::forward<Func>(func), std::forward<Args>(args)...));
}

/**
 * @brief Helper function to create a Property with a Binding.
 *
 * This function can take:
 * - Another Property.
 * - A Node, typically created by combining Property instances using operators.
 * - A function with arguments (Nodes, Constants or Properties)
 * By default this will construct a Property with an immediate binding evaluation.
 *
 * Alternatively a BindingEvaluator can be passed as the first argument to this function to control
 * when evaluation takes place.
 *
 * See the documentation for the various overloads of the free @ref makeBinding function for a
 * detailed description of which arguments can be used in which order.
 *
 * Examples:
 * - @ref 05-property-bindings/main.cpp
 * - @ref 06-lazy-property-bindings/main.cpp
 *
 * @return Property A new Property that is bound to the inputs
 *
 * *Note: For the difference between makeBinding and makeBoundProperty, see the
 * ["Reassigning a Binding"](../../getting-started/data-binding/#reassigning-a-binding) section in the Getting Started guide.*
 */
template<typename... T>
inline auto makeBoundProperty(T &&...args)
{
    auto binding = makeBinding(std::forward<T>(args)...);
    return Property<decltype(binding->get())>(std::move(binding));
}

} // namespace KDBindings
