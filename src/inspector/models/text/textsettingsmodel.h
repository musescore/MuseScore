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
#ifndef MU_INSPECTOR_TEXTSETTINGSMODEL_H
#define MU_INSPECTOR_TEXTSETTINGSMODEL_H

#include "async/asyncable.h"
#include "models/abstractinspectormodel.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"

namespace mu::inspector {
class TextSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    INJECT(muse::actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(PropertyItem * fontFamily READ fontFamily CONSTANT)
    Q_PROPERTY(PropertyItem * fontStyle READ fontStyle CONSTANT)
    Q_PROPERTY(PropertyItem * fontSize READ fontSize CONSTANT)
    Q_PROPERTY(PropertyItem * textLineSpacing READ textLineSpacing CONSTANT)
    Q_PROPERTY(PropertyItem * horizontalAlignment READ horizontalAlignment CONSTANT)
    Q_PROPERTY(PropertyItem * verticalAlignment READ verticalAlignment CONSTANT)

    Q_PROPERTY(PropertyItem * isSizeSpatiumDependent READ isSizeSpatiumDependent CONSTANT)
    Q_PROPERTY(PropertyItem * borderType READ borderType CONSTANT)
    Q_PROPERTY(PropertyItem * borderColor READ borderColor CONSTANT)
    Q_PROPERTY(PropertyItem * borderFillColor READ borderFillColor CONSTANT)
    Q_PROPERTY(PropertyItem * borderThickness READ borderThickness CONSTANT)
    Q_PROPERTY(PropertyItem * borderMargin READ borderMargin CONSTANT)
    Q_PROPERTY(PropertyItem * borderCornerRadius READ borderCornerRadius CONSTANT)

    Q_PROPERTY(PropertyItem * textType READ textType CONSTANT)
    Q_PROPERTY(PropertyItem * textPlacement READ textPlacement CONSTANT)
    Q_PROPERTY(PropertyItem * textScriptAlignment READ textScriptAlignment CONSTANT)

    Q_PROPERTY(QVariantList textStyles READ textStyles NOTIFY textStylesChanged)

    Q_PROPERTY(bool areStaffTextPropertiesAvailable READ areStaffTextPropertiesAvailable NOTIFY areStaffTextPropertiesAvailableChanged)
    Q_PROPERTY(
        bool isSpecialCharactersInsertionAvailable READ isSpecialCharactersInsertionAvailable NOTIFY isSpecialCharactersInsertionAvailableChanged)
    Q_PROPERTY(bool isDynamicSpecificSettings READ isDynamicSpecificSettings NOTIFY isDynamicSpecificSettingsChanged)
    Q_PROPERTY(bool isHorizontalAlignmentAvailable READ isHorizontalAlignmentAvailable NOTIFY isHorizontalAlignmentAvailableChanged)

public:
    explicit TextSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void insertSpecialCharacters();
    Q_INVOKABLE void showStaffTextProperties();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    PropertyItem* fontFamily() const;
    PropertyItem* fontStyle() const;
    PropertyItem* fontSize() const;
    PropertyItem* textLineSpacing() const;
    PropertyItem* horizontalAlignment() const;
    PropertyItem* verticalAlignment() const;

    PropertyItem* isSizeSpatiumDependent() const;
    PropertyItem* borderType() const;
    PropertyItem* borderColor() const;
    PropertyItem* borderFillColor() const;
    PropertyItem* borderThickness() const;
    PropertyItem* borderMargin() const;
    PropertyItem* borderCornerRadius() const;

    PropertyItem* textType() const;
    PropertyItem* textPlacement() const;
    PropertyItem* textScriptAlignment() const;

    QVariantList textStyles();

    bool areStaffTextPropertiesAvailable() const;
    bool isSpecialCharactersInsertionAvailable() const;
    bool isDynamicSpecificSettings() const;
    bool isHorizontalAlignmentAvailable() const;

public slots:
    void setAreStaffTextPropertiesAvailable(bool areStaffTextPropertiesAvailable);
    void setIsSpecialCharactersInsertionAvailable(bool isSpecialCharactersInsertionAvailable);
    void setIsDynamicSpecificSettings(bool isOnlyDynamics);
    void setIsHorizontalAlignmentAvailable(bool isHorizontalAlignmentAvailable);

signals:
    void textStylesChanged();

    void areStaffTextPropertiesAvailableChanged(bool areStaffTextPropertiesAvailable);
    void isSpecialCharactersInsertionAvailableChanged(bool isSpecialCharactersInsertionAvailable);
    void isDynamicSpecificSettingsChanged(bool isDynamicSpecificSettings);
    void isHorizontalAlignmentAvailableChanged(bool isHorizontalAlignmentAvailable);

private:
    bool isTextEditingStarted() const;
    muse::async::Notification isTextEditingChanged() const;

    void updateBorderPropertiesAvailability();
    void updateStaffPropertiesAvailability();
    void updateIsDynamicSpecificSettings();
    void updateIsHorizontalAlignmentAvailable();

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    PropertyItem* m_fontFamily = nullptr;
    PropertyItem* m_fontStyle = nullptr;
    PropertyItem* m_fontSize = nullptr;
    PropertyItem* m_textLineSpacing = nullptr;
    PropertyItem* m_horizontalAlignment = nullptr;
    PropertyItem* m_verticalAlignment = nullptr;

    PropertyItem* m_isSizeSpatiumDependent = nullptr;
    PropertyItem* m_borderType = nullptr;
    PropertyItem* m_borderColor = nullptr;
    PropertyItem* m_borderFillColor = nullptr;
    PropertyItem* m_borderThickness = nullptr;
    PropertyItem* m_borderMargin = nullptr;
    PropertyItem* m_borderCornerRadius = nullptr;

    PropertyItem* m_textType = nullptr;
    PropertyItem* m_textPlacement = nullptr;
    PropertyItem* m_textScriptAlignment = nullptr;

    QVariantList m_textStyles;

    bool m_areStaffTextPropertiesAvailable = false;
    bool m_isSpecialCharactersInsertionAvailable = false;
    bool m_isDynamicSpecificSettings = false;
    bool m_isHorizontalAlignmentAvailable = true;
};
}

#endif // MU_INSPECTOR_TEXTSETTINGSMODEL_H
