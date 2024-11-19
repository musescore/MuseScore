/*
  This file is part of KDBindings.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <kdbindings/property.h>
#include <kdbindings/signal.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace KDBindings {

/**
 * @brief A PropertyDestroyedError is thrown whenever a binding is evaluated
 * that references a property that no longer exists.
 */
class PropertyDestroyedError : public std::runtime_error
{
public:
    PropertyDestroyedError() = delete;

    using std::runtime_error::runtime_error;
};

namespace Private {

class Dirtyable
{
public:
    virtual ~Dirtyable() = default;

    Dirtyable() = default;

    void setParent(Dirtyable *newParent)
    {
        auto **parentVar = parentVariable();
        if (parentVar) {
            *parentVar = newParent;
        }
    }

    // overridden by Binding
    virtual void markDirty()
    {
        auto *dirtyVar = dirtyVariable();
        if (dirtyVar) {
            if (*dirtyVar) {
                return;
                // We are already dirty, don't bother marking the whole tree again.
            }

            // we only want to have one override for dirtyVariable,
            // which is const, so we have to const cast here.
            *const_cast<bool *>(dirtyVar) = true;
        }

        auto **parentVar = parentVariable();
        if (parentVar && *parentVar) {
            (*parentVar)->markDirty();
        }
    }

    bool isDirty() const
    {
        auto *dirtyVar = dirtyVariable();
        return dirtyVar && *dirtyVar;
    }

protected:
    virtual Dirtyable **parentVariable() = 0;
    virtual const bool *dirtyVariable() const = 0;
};

template<typename ResultType>
class NodeInterface : public Dirtyable
{
public:
    // Returns a reference, because we cache each evaluated value.
    // const, because it shouldn't modify the return value of the AST.
    // Requires mutable caches
    virtual const ResultType &evaluate() const = 0;

protected:
    NodeInterface() = default;
};

template<typename ResultType>
class Node
{
public:
    Node(std::unique_ptr<NodeInterface<ResultType>> &&nodeInterface)
        : m_interface(std::move(nodeInterface))
    {
    }

    const ResultType &evaluate() const
    {
        return m_interface->evaluate();
    }

    void setParent(Dirtyable *newParent)
    {
        m_interface->setParent(newParent);
    }

    bool isDirty() const
    {
        return m_interface->isDirty();
    }

private:
    std::unique_ptr<NodeInterface<ResultType>> m_interface;
};

template<typename T>
class ConstantNode : public NodeInterface<T>
{
public:
    explicit ConstantNode(const T &value)
        : m_value{ value }
    {
    }

    const T &evaluate() const override
    {
        return m_value;
    }

protected:
    // A constant can never be dirty, so it doesn't need to
    // know its parent, as it doesn't have to notify it.
    Dirtyable **parentVariable() override { return nullptr; }
    const bool *dirtyVariable() const override { return nullptr; }

private:
    T m_value;
};

template<typename PropertyType>
class PropertyNode : public NodeInterface<PropertyType>
{
public:
    explicit PropertyNode(Property<PropertyType> &property)
        : m_parent(nullptr), m_dirty(false)
    {
        setProperty(property);
    }

    // PropertyNodes cannot be moved
    PropertyNode(PropertyNode<PropertyType> &&) = delete;

    PropertyNode(const PropertyNode<PropertyType> &other)
        : Dirtyable(other.isDirty())
    {
        setProperty(*other.m_property);
    }

    virtual ~PropertyNode()
    {
        m_valueChangedHandle.disconnect();
        m_movedHandle.disconnect();
        m_destroyedHandle.disconnect();
    }

    const PropertyType &evaluate() const override
    {
        if (!m_property) {
            throw PropertyDestroyedError("The Property this node refers to no longer exists!");
        }

        m_dirty = false;
        return m_property->get();
    }

    // This must currently take a const reference, as the "moved" signal emits a const&
    void propertyMoved(Property<PropertyType> &property)
    {
        if (&property != m_property) {
            m_property = &property;
        } else {
            // Another property was moved into the property this node refers to.
            // Therefore it will no longer update this Node.
            m_property = nullptr;
        }
    }

    void propertyDestroyed()
    {
        m_property = nullptr;
    }

protected:
    Dirtyable **parentVariable() override { return &m_parent; }
    const bool *dirtyVariable() const override { return &m_dirty; }

private:
    void setProperty(Property<PropertyType> &property)
    {
        m_property = &property;
        m_valueChangedHandle = m_property->valueChanged().connect(&PropertyNode<PropertyType>::markDirty, this);
        m_movedHandle = m_property->m_moved.connect(&PropertyNode<PropertyType>::propertyMoved, this);
        m_destroyedHandle = m_property->destroyed().connect(&PropertyNode<PropertyType>::propertyDestroyed, this);
    }

    Property<PropertyType> *m_property;
    ConnectionHandle m_movedHandle;
    ConnectionHandle m_valueChangedHandle;
    ConnectionHandle m_destroyedHandle;

    Dirtyable *m_parent;
    mutable bool m_dirty;
};

template<typename ResultType, typename Operator, typename... Ts>
class OperatorNode : public NodeInterface<ResultType>
{
public:
    // add another typename template for the Operator type, so
    // it can be a universal reference.
    template<typename Op>
    explicit OperatorNode(Op &&op, Node<Ts> &&...arguments)
        : m_parent{ nullptr }, m_dirty{ true /*dirty until reevaluated*/ }, m_op{ std::move(op) }, m_values{ std::move(arguments)... }, m_result(reevaluate())
    {
        static_assert(
                std::is_convertible_v<decltype(m_op(std::declval<Ts>()...)), ResultType>,
                "The result of the Operator must be convertible to the ReturnType of the Node");

        setParents<0>();
    }

    template<std::size_t I>
    auto setParents() -> std::enable_if_t<I == sizeof...(Ts)>
    {
    }

    // The enable_if_t confuses clang-format into thinking the
    // first "<" is a comparison, and not the second.
    // clang-format off
    template<std::size_t I>
    auto setParents() -> std::enable_if_t<I < sizeof...(Ts)>
    // clang-format on
    {
        std::get<I>(m_values).setParent(this);
        setParents<I + 1>();
    }

    virtual ~OperatorNode() = default;

    const ResultType &evaluate() const override
    {
        if (Dirtyable::isDirty()) {
            m_result = reevaluate();
        }

        return m_result;
    }

protected:
    Dirtyable **parentVariable() override { return &m_parent; }
    const bool *dirtyVariable() const override { return &m_dirty; }

private:
    template<std::size_t... Is>
    ResultType reevaluate_helper(std::index_sequence<Is...>) const
    {
        return m_op(std::get<Is>(m_values).evaluate()...);
    }

    ResultType reevaluate() const
    {
        m_dirty = false;

        return reevaluate_helper(std::make_index_sequence<sizeof...(Ts)>());
    }

    Dirtyable *m_parent;
    mutable bool m_dirty;

    Operator m_op;
    std::tuple<Node<Ts>...> m_values;

    // Note: it is important that m_result is evaluated last!
    // Otherwise the call to reevaluate in the constructor will fail.
    mutable ResultType m_result;
};

template<typename T>
struct is_node_helper : std::false_type {
};

template<typename T>
struct is_node_helper<Node<T>> : std::true_type {
};

template<typename T>
struct is_node : is_node_helper<T> {
};

} // namespace Private

} // namespace KDBindings
