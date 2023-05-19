/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#ifndef MU_NOTATION_ABSTRACTELEMENTPOPUPMODEL_H
#define MU_NOTATION_ABSTRACTELEMENTPOPUPMODEL_H

#include "async/asyncable.h"
#include "actions/actionable.h"
#include "actions/actiontypes.h"

#include "context/iglobalcontext.h"
#include "libmscore/engravingitem.h"
#include "modularity/ioc.h"
#include <QObject>

namespace mu::notation {
class AbstractElementPopupModel : public QObject, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(actions::IActionsDispatcher, dispatcher)
    INJECT(context::IGlobalContext, globalContext)

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(PopupModelType modelType READ modelType CONSTANT)

public:
    enum class PopupModelType {
        TYPE_UNDEFINED = -1,
        TYPE_HARP_DIAGRAM
    };
    Q_ENUM(PopupModelType)

    explicit AbstractElementPopupModel(QObject* parent = nullptr);
    QString title() const;
    PopupModelType modelType() const;
    EngravingItem* getElement();

    static PopupModelType modelTypeFromElement(const mu::engraving::ElementType& elementType);

    virtual void init();

public slots:
    void setTitle(QString title);
    void setModelType(mu::notation::AbstractElementPopupModel::PopupModelType modelType);

signals:
    void titleChanged();
    void dataChanged();

protected:
    PointF fromLogical(PointF point) const;
    RectF fromLogical(RectF rect) const;

    notation::INotationUndoStackPtr undoStack() const;
    void beginCommand();
    void endCommand();
    void updateNotation();
    notation::INotationPtr currentNotation() const;

private:
    INotationInteractionPtr interaction() const;
    INotationSelectionPtr selection() const;
    const INotationInteraction::HitElementContext& hitElementContext() const;

    engraving::ElementType elementType() const;

    QString m_title;
    PopupModelType m_modelType = PopupModelType::TYPE_UNDEFINED;
};

using PopupModelType = AbstractElementPopupModel::PopupModelType;
} //namespace mu::notation

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::notation::PopupModelType)
#endif

#endif // MU_NOTATION_ABSTRACTELEMENTPOPUPMODEL_H
