/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_INSPECTOR_ABSTRACTINSPECTORMODEL_H
#define MU_INSPECTOR_ABSTRACTINSPECTORMODEL_H

#include <QList>
#include <functional>

#include "async/asyncable.h"

#include "engraving/dom/engravingitem.h"
#include "engraving/dom/property.h"

#include "internal/interfaces/ielementrepositoryservice.h"
#include "notation/inotation.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"
#include "modularity/ioc.h"
#include "models/propertyitem.h"
#include "ui/view/iconcodes.h"
#include "types/commontypes.h"

namespace mu::inspector {
class AbstractInspectorModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)
    INJECT(actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(int icon READ icon CONSTANT)
    Q_PROPERTY(InspectorSectionType sectionType READ sectionType CONSTANT)
    Q_PROPERTY(InspectorModelType modelType READ modelType CONSTANT)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)

public:
    enum class InspectorSectionType {
        SECTION_UNDEFINED = -1,
        SECTION_GENERAL,
        SECTION_MEASURES,
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
        TYPE_GLISSANDO,
        TYPE_BARLINE,
        TYPE_BREATH,
        TYPE_STAFF,
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
        TYPE_TIE,
        TYPE_CRESCENDO,
        TYPE_DIMINUENDO,
        TYPE_STAFF_TYPE_CHANGES,
        TYPE_TEXT_FRAME,
        TYPE_VERTICAL_FRAME,
        TYPE_HORIZONTAL_FRAME,
        TYPE_ARTICULATION,
        TYPE_ORNAMENT,
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
        TYPE_STRING_TUNINGS,
    };
    Q_ENUM(InspectorModelType)

    explicit AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository = nullptr,
                                    engraving::ElementType elementType = engraving::ElementType::INVALID);

    void init();

    QString title() const;
    int icon() const;
    InspectorSectionType sectionType() const;
    InspectorModelType modelType() const;

    static InspectorModelType modelTypeByElementKey(const ElementKey& elementKey);
    static QSet<InspectorModelType> modelTypesByElementKeys(const ElementKeySet& elementKeySet);
    static QSet<InspectorSectionType> sectionTypesByElementKeys(const ElementKeySet& elementKeySet, bool isRange,
                                                                const QList<mu::engraving::EngravingItem*>& selectedElementList = {});
    static bool showPartsSection(const QList<mu::engraving::EngravingItem*>& selectedElementList);

    virtual bool isEmpty() const;

    virtual void createProperties() = 0;
    virtual void loadProperties() = 0;
    virtual void resetProperties() = 0;

    virtual void requestElements();

    virtual void onCurrentNotationChanged();

public slots:
    void setTitle(QString title);
    void setIcon(ui::IconCode::Code icon);
    void setSectionType(InspectorSectionType sectionType);
    void setModelType(InspectorModelType modelType);

signals:
    void titleChanged();

    void modelReseted();
    void isEmptyChanged();

    void requestReloadPropertyItems();

protected:
    void setElementType(engraving::ElementType type);

    using ElementList = QList<engraving::EngravingItem*>;

    // Internal-to-UI converters
    using GeneralInternalToUiConverter = std::function<QVariant(const engraving::PropertyValue& propertyValue)>;
    using ElementInternalToUiConverter = std::function<QVariant(const engraving::PropertyValue& propertyValue,
                                                                const engraving::EngravingItem* element)>;

    static ElementInternalToUiConverter defaultElementInternalToUiConverter(engraving::Pid pid);
    static ElementInternalToUiConverter roundedDoubleElementInternalToUiConverter(engraving::Pid pid);
    // ---

    // UI-to-internal converters
    using GeneralUiToInternalConvertor = std::function<engraving::PropertyValue(const QVariant& value)>;
    using ElementUiToInternalConvertor = std::function<engraving::PropertyValue(const QVariant& value,
                                                                                const engraving::EngravingItem* element)>;
    using StyleUiToInternalConvertor = std::function<engraving::PropertyValue(const QVariant& value)>;

    static ElementUiToInternalConvertor defaultElementUiToInternalConvertor(engraving::Pid pid);
    static StyleUiToInternalConvertor defaultStyleUiToInternalConvertor(engraving::Pid pid);
    // ---

    // Callbacks
    using SetPropertyCallback = std::function<void (engraving::Pid pid, const QVariant& newValue)>;
    using SetStyleValueCallback = std::function<void (engraving::Sid sid, const QVariant& newValue)>;

    SetPropertyCallback defaultSetPropertyCallback(engraving::Pid pid);
    SetStyleValueCallback defaultSetStyleValueCallback(engraving::Pid pid);
    // ---

    static engraving::PropertyValue valueToElementUnits(const engraving::Pid& pid, const QVariant& value,
                                                        const engraving::EngravingItem* element);
    static QVariant valueFromElementUnits(const engraving::Pid& pid, const engraving::PropertyValue& value,
                                          const engraving::EngravingItem* element);

    void setProperty(engraving::Pid pid, const QVariant& newValue, ElementUiToInternalConvertor convertor);
    void setProperty(const QList<engraving::EngravingItem*>& items, engraving::Pid pid, const QVariant& newValue,
                     ElementUiToInternalConvertor convertor);
    void setProperty(engraving::Pid pid, const engraving::PropertyValue& newValue);
    void setProperty(const QList<engraving::EngravingItem*>& items, engraving::Pid pid, const engraving::PropertyValue& newValue);

    void setStyleValue(engraving::Sid sid, const QVariant& newValue, StyleUiToInternalConvertor convertor);
    bool setStyleValue(engraving::Sid sid, const engraving::PropertyValue& newValue);

    PropertyItem* buildPropertyItem(const engraving::Pid& pid);
    PropertyItem* buildPropertyItem(const engraving::Pid& pid, GeneralUiToInternalConvertor convertor);
    PropertyItem* buildPropertyItem(const engraving::Pid& pid, ElementUiToInternalConvertor elementConvertor,
                                    StyleUiToInternalConvertor styleConvertor);
    PropertyItem* buildPropertyItem(const engraving::Pid& pid, SetPropertyCallback propertySetter,
                                    SetStyleValueCallback styleSetter = nullptr);

    void loadPropertyItem(PropertyItem* propertyItem, const QList<engraving::EngravingItem*>& items);
    void loadPropertyItem(PropertyItem* propertyItem, GeneralInternalToUiConverter convertor);
    void loadPropertyItem(PropertyItem* propertyItem, GeneralInternalToUiConverter convertor,
                          const QList<engraving::EngravingItem*>& items);
    void loadPropertyItem(PropertyItem* propertyItem, ElementInternalToUiConverter convertor = nullptr);
    void loadPropertyItem(PropertyItem* propertyItem, ElementInternalToUiConverter convertor,
                          const QList<engraving::EngravingItem*>& items);

    bool isNotationExisting() const;

    notation::INotationStylePtr style() const;
    QVariant styleValue(const engraving::Sid& sid) const;

    notation::INotationUndoStackPtr undoStack() const;
    void beginCommand();
    void endCommand();

    void updateNotation();
    notation::INotationPtr currentNotation() const;
    async::Notification currentNotationChanged() const;
    bool isMasterNotation() const;

    notation::INotationSelectionPtr selection() const;

    virtual void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                                   const mu::engraving::StyleIdSet& changedStyleIdSet);

    IElementRepositoryService* m_repository = nullptr;

    QList<engraving::EngravingItem*> m_elementList;

protected slots:
    void updateProperties();

private:
    void initPropertyItem(PropertyItem* propertyItem, std::function<void(const mu::engraving::Pid propertyId,
                                                                         const QVariant& newValue)> onPropertyChangedCallBack = nullptr,
                          std::function<void(const engraving::Sid styleId,
                                             const QVariant& newValue)> onStyleChangedCallBack = nullptr);

    engraving::Sid styleIdByPropertyId(const engraving::Pid pid) const;
    engraving::PropertyIdSet propertyIdSetFromStyleIdSet(const engraving::StyleIdSet& styleIdSet) const;

    QString m_title;
    ui::IconCode::Code m_icon = ui::IconCode::Code::NONE;
    InspectorSectionType m_sectionType = InspectorSectionType::SECTION_UNDEFINED;
    InspectorModelType m_modelType = InspectorModelType::TYPE_UNDEFINED;
    engraving::ElementType m_elementType = engraving::ElementType::INVALID;
    bool m_updatePropertiesAllowed = false;
};

using InspectorModelType = AbstractInspectorModel::InspectorModelType;
using InspectorSectionType = AbstractInspectorModel::InspectorSectionType;
using InspectorModelTypeSet = QSet<InspectorModelType>;
using InspectorSectionTypeSet = QSet<InspectorSectionType>;

inline uint qHash(InspectorSectionType key)
{
    return ::qHash(QString::number(static_cast<int>(key)));
}

inline uint qHash(InspectorModelType key)
{
    return ::qHash(QString::number(static_cast<int>(key)));
}
}

#endif // MU_INSPECTOR_ABSTRACTINSPECTORMODEL_H
