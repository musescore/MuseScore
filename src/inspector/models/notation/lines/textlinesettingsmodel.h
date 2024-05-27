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
#ifndef MU_INSPECTOR_TEXTLINESETTINGSMODEL_H
#define MU_INSPECTOR_TEXTLINESETTINGSMODEL_H

#include "models/inspectormodelwithvoiceandpositionoptions.h"

#include "ui/view/iconcodes.h"

namespace mu::inspector {
class TextLineSettingsModel : public InspectorModelWithVoiceAndPositionOptions
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isLineVisible READ isLineVisible CONSTANT)
    Q_PROPERTY(PropertyItem * allowDiagonal READ allowDiagonal CONSTANT)

    Q_PROPERTY(PropertyItem * lineStyle READ lineStyle CONSTANT)

    Q_PROPERTY(PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(PropertyItem * dashLineLength READ dashLineLength CONSTANT)
    Q_PROPERTY(PropertyItem * dashGapLength READ dashGapLength CONSTANT)

    Q_PROPERTY(PropertyItem * startHookType READ startHookType CONSTANT)
    Q_PROPERTY(PropertyItem * endHookType READ endHookType CONSTANT)
    Q_PROPERTY(PropertyItem * startHookHeight READ startHookHeight CONSTANT)
    Q_PROPERTY(PropertyItem * endHookHeight READ endHookHeight CONSTANT)
    Q_PROPERTY(PropertyItem * gapBetweenTextAndLine READ gapBetweenTextAndLine CONSTANT)

    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

    Q_PROPERTY(PropertyItem * beginningText READ beginningText CONSTANT)
    Q_PROPERTY(PropertyItem * beginningTextOffset READ beginningTextOffset CONSTANT)

    Q_PROPERTY(PropertyItem * continuousText READ continuousText CONSTANT)
    Q_PROPERTY(PropertyItem * continuousTextOffset READ continuousTextOffset CONSTANT)

    Q_PROPERTY(PropertyItem * endText READ endText CONSTANT)
    Q_PROPERTY(PropertyItem * endTextOffset READ endTextOffset CONSTANT)

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
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

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

#endif // MU_INSPECTOR_TEXTLINESETTINGSMODEL_H
