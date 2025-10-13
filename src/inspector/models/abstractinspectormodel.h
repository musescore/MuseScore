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

#include <functional>
#include <set>

#include <QList>

#include "async/asyncable.h"

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/property.h"

#include "internal/interfaces/ielementrepositoryservice.h"
#include "notation/inotation.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"
#include "shortcuts/ishortcutsregister.h"
#include "modularity/ioc.h"
#include "models/propertyitem.h"
#include "models/pointfpropertyitem.h"
#include "ui/view/iconcodes.h"
#include "ui/iuiactionsregister.h"
#include "types/commontypes.h"

namespace mu::inspector {
class AbstractInspectorModel : public QObject, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(int icon READ icon CONSTANT)
    Q_PROPERTY(InspectorSectionType sectionType READ sectionType CONSTANT)
    Q_PROPERTY(InspectorModelType modelType READ modelType CONSTANT)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)

    Q_PROPERTY(bool isSystemObjectBelowBottomStaff READ isSystemObjectBelowBottomStaff NOTIFY isSystemObjectBelowBottomStaffChanged)

public:
    muse::Inject<context::IGlobalContext> context;
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher;
    muse::Inject<muse::ui::IUiActionsRegister> uiActionsRegister;
    muse::Inject<muse::shortcuts::IShortcutsRegister> shortcutsRegister;

public:
    enum class InspectorSectionType {
        SECTION_UNDEFINED = -1,
        SECTION_GENERAL,
        SECTION_MEASURES,
        SECTION_EMPTY_STAVES,
        SECTION_NOTATION,
        SECTION_TEXT,
        SECTION_SCORE_DISPLAY,
        SECTION_SCORE_APPEARANCE,
        SECTION_PARTS,
    };
    Q_ENUM(InspectorSectionType)

    enum class InspectorModelType {
        TYPE_UNDEFINED = -1,
        TYPE_NOTE,
        TYPE_CHORD,
        TYPE_BEAM,
        TYPE_NOTEHEAD,
        TYPE_STEM,
        TYPE_HOOK,
        TYPE_FERMATA,
        TYPE_TEMPO,
        TYPE_A_TEMPO,
        TYPE_TEMPO_PRIMO,
        TYPE_GLISSANDO,
        TYPE_BARLINE,
        TYPE_BREATH,
        TYPE_MARKER,
        TYPE_SECTIONBREAK,
        TYPE_JUMP,
        TYPE_KEYSIGNATURE,
        TYPE_ACCIDENTAL,
        TYPE_ARPEGGIO,
        TYPE_FRET_DIAGRAM,
        TYPE_PEDAL,
        TYPE_SPACER,
        TYPE_CLEF,
        TYPE_HAIRPIN,
        TYPE_OTTAVA,
        TYPE_PALM_MUTE,
        TYPE_LET_RING,
        TYPE_VOLTA,
        TYPE_VIBRATO,
        TYPE_SLUR,
        TYPE_HAMMER_ON_PULL_OFF,
        TYPE_TIE,
        TYPE_LAISSEZ_VIB,
        TYPE_PARTIAL_TIE,
        TYPE_CRESCENDO,
        TYPE_DIMINUENDO,
        TYPE_STAFF_TYPE_CHANGES,
        TYPE_TEXT_FRAME,
        TYPE_VERTICAL_FRAME,
        TYPE_HORIZONTAL_FRAME,
        TYPE_FRET_FRAME,
        TYPE_FRET_FRAME_CHORDS,
        TYPE_FRET_FRAME_SETTINGS,
        TYPE_ARTICULATION,
        TYPE_ORNAMENT,
        TYPE_TAPPING,
        TYPE_AMBITUS,
        TYPE_IMAGE,
        TYPE_CHORD_SYMBOL,
        TYPE_BRACKET,
        TYPE_TIME_SIGNATURE,
        TYPE_MMREST,
        TYPE_BEND,
        TYPE_TREMOLOBAR,
        TYPE_TREMOLO,
        TYPE_MEASURE_REPEAT,
        TYPE_DYNAMIC,
        TYPE_EXPRESSION,
        TYPE_TUPLET,
        TYPE_TEXT_LINE,
        TYPE_GRADUAL_TEMPO_CHANGE,
        TYPE_INSTRUMENT_NAME,
        TYPE_LYRICS,
        TYPE_REST,
        TYPE_REST_BEAM,
        TYPE_REST_REST,
        TYPE_STRING_TUNINGS,
        TYPE_SYMBOL,
        TYPE_NOTELINE,
        TYPE_PLAY_COUNT_TEXT
    };
    Q_ENUM(InspectorModelType)

    explicit AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository = nullptr,
                                    mu::engraving::ElementType elementType = mu::engraving::ElementType::INVALID);

    void init();

    Q_INVOKABLE virtual void requestResetToDefaults();

    QString title() const;
    int icon() const;
    InspectorSectionType sectionType() const;
    InspectorModelType modelType() const;

    static ElementKey makeKey(const mu::engraving::EngravingItem* item);
    static InspectorModelType modelTypeByElementKey(const ElementKey& elementKey);
    static std::set<InspectorModelType> modelTypesByElementKeys(const ElementKeySet& elementKeySet);
    static std::set<InspectorSectionType> sectionTypesByElementKeys(const ElementKeySet& elementKeySet, bool isRange,
                                                                    const QList<mu::engraving::EngravingItem*>& selectedElementList = {});
    virtual bool isEmpty() const;

    virtual void createProperties() = 0;
    virtual void loadProperties() = 0;
    virtual void resetProperties() = 0;

    virtual void requestElements();

    virtual void onCurrentNotationChanged();

    virtual void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                                   const mu::engraving::StyleIdSet& changedStyleIdSet);

    bool isSystemObjectBelowBottomStaff() const;
    bool shouldUpdateOnScoreChange() const;
    virtual bool shouldUpdateOnEmptyPropertyAndStyleIdSets() const;

    mu::engraving::PropertyIdSet propertyIdSetFromStyleIdSet(const mu::engraving::StyleIdSet& styleIdSet) const;

public slots:
    void setTitle(QString title);
    void setIcon(muse::ui::IconCode::Code icon);
    void setSectionType(InspectorSectionType sectionType);
    void setModelType(InspectorModelType modelType);

signals:
    void titleChanged();

    void modelReseted();
    void isEmptyChanged();

    void requestReloadPropertyItems();

    void requestReloadInspectorListModel();

    void isSystemObjectBelowBottomStaffChanged(bool isSystemObjectBelowBottomStaff);

protected:
    void setElementType(mu::engraving::ElementType type);

    PropertyItem* buildPropertyItem(const mu::engraving::Pid& pid, std::function<void(const mu::engraving::Pid propertyId,
                                                                                      const QVariant& newValue)> onPropertyChangedCallBack =
                                    nullptr, std::function<void(const mu::engraving::Sid styleId,
                                                                const
                                                                QVariant& newValue)> onStyleChangedCallBack = nullptr,
                                    std::function<void(const mu::engraving::Pid propertyId)> onPropertyResetCallBack = nullptr);
    PointFPropertyItem* buildPointFPropertyItem(const mu::engraving::Pid& pid, std::function<void(const mu::engraving::Pid propertyId,
                                                                                                  const QVariant& newValue)>
                                                onPropertyChangedCallBack = nullptr,
                                                std::function<void(const mu::engraving::Pid propertyId)> onPropertyResetCallBack = nullptr);

    using ConvertPropertyValueFunc = std::function<QVariant (const QVariant&)>;
    void loadPropertyItem(PropertyItem* propertyItem, ConvertPropertyValueFunc convertElementPropertyValueFunc = nullptr);
    void loadPropertyItem(PropertyItem* propertyItem, const QList<engraving::EngravingItem*>& elements,
                          ConvertPropertyValueFunc convertElementPropertyValueFunc = nullptr);

    bool isNotationExisting() const;

    engraving::PropertyValue valueToElementUnits(const mu::engraving::Pid& pid, const QVariant& value,
                                                 const mu::engraving::EngravingItem* element) const;
    QVariant valueFromElementUnits(const mu::engraving::Pid& pid, const engraving::PropertyValue& value,
                                   const mu::engraving::EngravingItem* element) const;

    notation::INotationStylePtr style() const;
    bool updateStyleValue(const mu::engraving::Sid& sid, const QVariant& newValue);
    QVariant styleValue(const mu::engraving::Sid& sid) const;

    notation::INotationUndoStackPtr undoStack() const;
    void beginCommand(const muse::TranslatableString& actionName);
    void endCommand();

    void updateNotation();
    notation::INotationPtr currentNotation() const;
    bool isMasterNotation() const;

    notation::INotationSelectionPtr selection() const;

    IElementRepositoryService* m_repository = nullptr;

    QList<mu::engraving::EngravingItem*> m_elementList;

    QString shortcutsForActionCode(std::string code) const;

protected slots:
    void onPropertyValueChanged(const mu::engraving::Pid pid, const QVariant& newValue);
    void setPropertyValue(const QList<mu::engraving::EngravingItem*>& items, const mu::engraving::Pid pid, const QVariant& newValue);
    void onPropertyValueReset(const mu::engraving::Pid pid);
    void resetPropertyValue(const QList<mu::engraving::EngravingItem*>& items, const mu::engraving::Pid pid);
    void updateProperties();
    void updateIsSystemObjectBelowBottomStaff();

private:
    static bool showPartsSection(const QList<mu::engraving::EngravingItem*>& selectedElementList);

    void initPropertyItem(PropertyItem* propertyItem, std::function<void(const mu::engraving::Pid propertyId,
                                                                         const QVariant& newValue)> onPropertyChangedCallBack = nullptr,
                          std::function<void(const mu::engraving::Sid styleId,
                                             const
                                             QVariant
                                             & newValue)> onStyleChangedCallBack = nullptr,
                          std::function<void(const mu::engraving::Pid propertyId)> onPropertyResetCallBack = nullptr);

    mu::engraving::Sid styleIdByPropertyId(const mu::engraving::Pid pid) const;

    QString m_title;
    muse::ui::IconCode::Code m_icon = muse::ui::IconCode::Code::NONE;
    InspectorSectionType m_sectionType = InspectorSectionType::SECTION_UNDEFINED;
    InspectorModelType m_modelType = InspectorModelType::TYPE_UNDEFINED;
    mu::engraving::ElementType m_elementType = mu::engraving::ElementType::INVALID;
    bool m_shouldUpdateOnScoreChange = true;

    bool m_isSystemObjectBelowBottomStaff = false;
};

using InspectorModelType = AbstractInspectorModel::InspectorModelType;
using InspectorSectionType = AbstractInspectorModel::InspectorSectionType;
using InspectorModelTypeSet = std::set<InspectorModelType>;
using InspectorSectionTypeSet = std::set<InspectorSectionType>;
}
