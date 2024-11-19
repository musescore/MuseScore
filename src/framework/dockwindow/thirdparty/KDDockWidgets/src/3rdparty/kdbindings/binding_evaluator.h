/*
  This file is part of KDBindings.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <functional>
#include <map>
#include <memory>

namespace KDBindings {

/**
 * @brief A BindingEvaluator provides a mechanism to control the exact time
 * when a KDBindings::Binding is reevaluated.
 *
 * A BindingEvaluator represents a collection of Binding instances that can be
 * selectively reevaluated.
 *
 * If a Binding is created using KDBindings::makeBoundProperty with a BindingEvaluator,
 * the Binding will only be evaluated if BindingEvaluator::evaluateAll is called
 * on the given evaluator.
 *
 * Note that instances of BindingEvaluator internally wrap their collection of
 * Bindings in such a way that copying a BindingEvaluator does not actually
 * copy the collection of Bindings. Therefore adding a Binding to a copy of a
 * BindingEvaluator will also add it to the original.
 * This is done for ease of use, so evaluators can be passed around easily throughout
 * the codebase.
 *
 * Examples:
 * - @ref 06-lazy-property-bindings/main.cpp
 */
class BindingEvaluator
{
    // We use pimpl here so that we can pass evaluators around by value (copies)
    // yet each copy refers to the same set of data
    struct Private {
        // TODO: Use std::vector here?
        std::map<int, std::function<void()>> m_bindingEvalFunctions;
        int m_currentId;
    };

public:
    /** A BindingEvaluator can be default constructed */
    BindingEvaluator() = default;

    /**
     * A BindingEvaluator can be copy constructed.
     *
     * Note that copying the evaluator will NOT create a new collection of
     * Binding instances, but the new evaluator will refer to the same collection,
     * so creating a new Binding with this evaluator will also modify the previous one.
     */
    BindingEvaluator(const BindingEvaluator &) noexcept = default;

    /**
     * A BindingEvaluator can be copy assigned.
     *
     * Note that copying the evaluator will NOT create a new collection of
     * Binding instances, but the new evaluator will refer to the same collection,
     * so creating a new Binding with this evaluator will also modify the previous one.
     */
    BindingEvaluator &operator=(const BindingEvaluator &) noexcept = default;

    /**
     * A BindingEvaluator can not be move constructed.
     */
    BindingEvaluator(BindingEvaluator &&other) noexcept = delete;

    /**
     * A BindingEvaluator can not be move assigned.
     */
    BindingEvaluator &operator=(BindingEvaluator &&other) noexcept = delete;

    /**
     * This function evaluates all Binding instances that were constructed with this
     * evaluator, in the order they were inserted.
     *
     * It will therefore update the associated Property instances as well.
     */
    void evaluateAll() const
    {
        // a std::map's ordering is deterministic, so the bindings are evaluated
        // in the order they were inserted, ensuring correct transitive dependency
        // evaluation.
        for (auto &[id, func] : m_d->m_bindingEvalFunctions)
            func();
    }

private:
    template<typename BindingType>
    int insert(BindingType *binding)
    {
        m_d->m_bindingEvalFunctions.insert({ ++(m_d->m_currentId),
                                             [=]() { binding->evaluate(); } });
        return m_d->m_currentId;
    }

    void remove(int id)
    {
        m_d->m_bindingEvalFunctions.erase(id);
    }

    std::shared_ptr<Private> m_d{ std::make_shared<Private>() };

    template<typename T, typename UpdaterT>
    friend class Binding;
};

/**
 * This subclass of BindingEvaluator doesn't do anything special on its own.
 * It is used together with a template specialization of Binding to provide
 * old-school, immediate mode Bindings.
 *
 * Any Binding that is constructed with an ImmediateBindingEvaluator will not wait
 * for the evaluator to call evaluateAll, but rather evaluate the Binding immediately
 * when any of its bindables (i.e. Property instances) change.
 * This can lead to a Property Binding being evaluated many
 * times before the result is ever used in a typical GUI application.
 */
class ImmediateBindingEvaluator final : public BindingEvaluator
{
public:
    static inline ImmediateBindingEvaluator instance()
    {
        static ImmediateBindingEvaluator evaluator;
        return evaluator;
    }
};

} // namespace KDBindings

/**
 * @example 06-lazy-property-bindings/main.cpp
 *
 * An example of how to use KDBindings::BindingEvaluator together
 * with a KDBindings::Property to create a Property binding that is
 * only reevaluated on demand.
 *
 * The output of this example is:
 * ```
 * The initial size of the image = 1920000 bytes
 * The new size of the image = 8294400 bytes
 * ```
 *
 * Note the difference to @ref 05-property-bindings/main.cpp, where the
 * new size of the image is calculated twice.
 *
 * This feature is especially useful to reduce the performance impact of
 * bindings and to create bindings that only update in specific intervals.
 * <br/><!-- This <br/> is a workaround for a bug in doxybook2 that causes
 * the rendering of the example code to break because it is missing a
 * newline-->
 */
