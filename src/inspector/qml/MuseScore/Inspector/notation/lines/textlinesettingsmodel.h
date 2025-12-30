/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <qqmlintegration.h>

#include "abstractinspectormodel.h"

#include "ui/view/iconcodes.h"

namespace mu::inspector {
class TextLineSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::inspector::PropertyItem * isLineVisible READ isLineVisible CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * allowDiagonal READ allowDiagonal CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * lineStyle READ lineStyle CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * dashLineLength READ dashLineLength CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * dashGapLength READ dashGapLength CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * startHookType READ startHookType CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * endHookType READ endHookType CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * startHookHeight READ startHookHeight CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * endHookHeight READ endHookHeight CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * gapBetweenTextAndLine READ gapBetweenTextAndLine CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * placement READ placement CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * beginningText READ beginningText CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * beginningTextOffset READ beginningTextOffset CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * continuousText READ continuousText CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * continuousTextOffset READ continuousTextOffset CONSTANT)

    Q_PROPERTY(mu::inspector::PropertyItem * endText READ endText CONSTANT)
    Q_PROPERTY(mu::inspector::PropertyItem * endTextOffset READ endTextOffset CONSTANT)

public:
    explicit TextLineSettingsModel(QObject* parent, IElementRepositoryService* repository,
                                   mu::engraving::ElementType elementType = mu::engraving::ElementType::TEXTLINE);

    PropertyItem* isLineVisible() const;
    PropertyItem* allowDiagonal() const;

    PropertyItem* lineStyle() const;

    PropertyItem* thickness() const;
    PropertyItem* dashLineLength() const;
    PropertyItem* dashGapLength() const;

    PropertyItem* startHookType() const;
    PropertyItem* endHookType() const;
    PropertyItem* startHookHeight() const;
    PropertyItem* endHookHeight() const;
    PropertyItem* gapBetweenTextAndLine() const;

    PropertyItem* placement() const;

    PropertyItem* beginningText() const;
    PropertyItem* beginningTextOffset() const;

    PropertyItem* continuousText() const;
    PropertyItem* continuousTextOffset() const;

    PropertyItem* endText() const;
    PropertyItem* endTextOffset() const;

    Q_INVOKABLE QVariantList possibleStartHookTypes() const;
    Q_INVOKABLE QVariantList possibleEndHookTypes() const;

protected:
    enum TextType {
        BeginningText,
        ContinuousText,
        EndText
    };

    struct HookTypeInfo {
        int type = 0;
        muse::ui::IconCode::Code icon = muse::ui::IconCode::Code::NONE;
        QString title;

        HookTypeInfo(mu::engraving::HookType type, muse::ui::IconCode::Code icon, const QString& title)
            : type(static_cast<int>(type)), icon(icon), title(title)
        {
        }

        HookTypeInfo(int type, muse::ui::IconCode::Code icon, const QString& title)
            : type(type), icon(icon), title(title)
        {
        }
    };

    void createProperties() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&) override;

    virtual void onUpdateLinePropertiesAvailability();

    void setPossibleStartHookTypes(const QList<HookTypeInfo>& types);
    void setPossibleEndHookTypes(const QList<HookTypeInfo>& types);

private:
    QVariantList hookTypesToObjList(const QList<HookTypeInfo>& types) const;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    PropertyItem* m_lineStyle = nullptr;
    PropertyItem* m_placement = nullptr;

    PropertyItem* m_thickness = nullptr;
    PropertyItem* m_dashLineLength = nullptr;
    PropertyItem* m_dashGapLength = nullptr;

    PropertyItem* m_isLineVisible = nullptr;
    PropertyItem* m_allowDiagonal = nullptr;

    PropertyItem* m_startHookType = nullptr;
    PropertyItem* m_endHookType = nullptr;
    PropertyItem* m_startHookHeight = nullptr;
    PropertyItem* m_endHookHeight = nullptr;
    PropertyItem* m_gapBetweenTextAndLine = nullptr;

    PropertyItem* m_beginningText = nullptr;
    PointFPropertyItem* m_beginningTextOffset = nullptr;

    PropertyItem* m_continuousText = nullptr;
    PointFPropertyItem* m_continuousTextOffset = nullptr;

    PropertyItem* m_endText = nullptr;
    PointFPropertyItem* m_endTextOffset = nullptr;

    QVariantList m_possibleStartHookTypes;
    QVariantList m_possibleEndHookTypes;
};
}
