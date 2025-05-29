/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
#pragma once

#include <QObject>

#include "async/asyncable.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "engraving/dom/engravingitem.h"

namespace mu::notation {
class AbstractElementPopupModel : public QObject, public muse::Injectable, public muse::async::Asyncable, public muse::actions::Actionable
{
    Q_OBJECT

    Q_PROPERTY(PopupModelType modelType READ modelType CONSTANT)
    Q_PROPERTY(QRect itemRect READ itemRect NOTIFY itemRectChanged)

public:
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    enum class PopupModelType {
        TYPE_UNDEFINED = -1,
        TYPE_HARP_DIAGRAM,
        TYPE_CAPO,
        TYPE_STRING_TUNINGS,
        TYPE_SOUND_FLAG,
        TYPE_DYNAMIC,
        TYPE_TEXT,
        TYPE_PARTIAL_TIE,
        TYPE_SHADOW_NOTE
    };
    Q_ENUM(PopupModelType)

    AbstractElementPopupModel(PopupModelType modelType, QObject* parent = nullptr);

    PopupModelType modelType() const;
    QRect itemRect() const;

    static bool hasElementEditPopup(const engraving::EngravingItem* element);
    static bool hasTextStylePopup(const engraving::EngravingItem* element);
    static PopupModelType modelTypeFromElement(const engraving::ElementType& elementType);

    virtual void init();

signals:
    void dataChanged();
    void itemRectChanged(QRect rect);

protected:
    virtual void updateItemRect();

    muse::PointF fromLogical(muse::PointF point) const;
    muse::RectF fromLogical(muse::RectF rect) const;

    notation::INotationUndoStackPtr undoStack() const;
    void beginCommand(const muse::TranslatableString& actionName);
    void beginMultiCommands(const muse::TranslatableString& actionName);
    void endCommand();
    void endMultiCommands();
    void updateNotation();
    notation::INotationPtr currentNotation() const;
    INotationInteractionPtr interaction() const;

    void changeItemProperty(engraving::Pid id, const PropertyValue& value);
    void changeItemProperty(engraving::Pid id, const PropertyValue& value, engraving::PropertyFlags flags);

    EngravingItem* m_item = nullptr;
    QRect m_itemRect;

private:
    INotationSelectionPtr selection() const;

    engraving::ElementType elementType() const;
    const engraving::ElementTypeSet& dependentElementTypes() const;

    PopupModelType m_modelType = PopupModelType::TYPE_UNDEFINED;
};

using PopupModelType = AbstractElementPopupModel::PopupModelType;
#ifndef NO_QT_SUPPORT
inline size_t qHash(mu::notation::PopupModelType key)
{
    return ::qHash(int(key));
}

#endif
} //namespace mu::notation

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::notation::PopupModelType)
#endif
