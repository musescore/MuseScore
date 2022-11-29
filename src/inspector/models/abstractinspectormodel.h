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

#include "libmscore/engravingitem.h"
#include "libmscore/property.h"

#include "internal/interfaces/ielementrepositoryservice.h"
#include "notation/inotation.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"
#include "modularity/ioc.h"
#include "models/propertyitem.h"
#include "models/pointfpropertyitem.h"
#include "ui/view/iconcodes.h"
#include "types/commontypes.h"

namespace mu::inspector {
class AbstractInspectorModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(inspector, context::IGlobalContext, context)
    INJECT(inspector, actions::IActionsDispatcher, dispatcher)

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
        SECTION_SCORE_APPEARANCE
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
        TYPE_TUPLET,
        TYPE_TEXT_LINE,
        TYPE_GRADUAL_TEMPO_CHANGE,
        TYPE_INSTRUMENT_NAME,
        TYPE_LYRICS,
    };
    Q_ENUM(InspectorModelType)

    explicit AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository = nullptr,
                                    mu::engraving::ElementType elementType = mu::engraving::ElementType::INVALID);

    Q_INVOKABLE virtual void requestResetToDefaults();

    QString title() const;
    int icon() const;
    InspectorSectionType sectionType() const;
    InspectorModelType modelType() const;

    static InspectorModelType modelTypeByElementKey(const ElementKey& elementKey);
    static QSet<InspectorModelType> modelTypesByElementKeys(const ElementKeySet& elementKeySet);
    static QSet<InspectorSectionType> sectionTypesByElementKeys(const ElementKeySet& elementKeySet, bool isRange);

    virtual bool isEmpty() const;

    virtual void createProperties() = 0;
    virtual void loadProperties() = 0;
    virtual void resetProperties() = 0;

    virtual void requestElements();

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
    void setElementType(mu::engraving::ElementType type);

    PropertyItem* buildPropertyItem(const mu::engraving::Pid& pid, std::function<void(const mu::engraving::Pid propertyId,
                                                                                      const QVariant& newValue)> onPropertyChangedCallBack = nullptr);
    PointFPropertyItem* buildPointFPropertyItem(const mu::engraving::Pid& pid, std::function<void(const mu::engraving::Pid propertyId,
                                                                                                  const QVariant& newValue)> onPropertyChangedCallBack = nullptr);

    void loadPropertyItem(PropertyItem* propertyItem, std::function<QVariant(const QVariant&)> convertElementPropertyValueFunc = nullptr);

    bool isNotationExisting() const;

    engraving::PropertyValue valueToElementUnits(const mu::engraving::Pid& pid, const QVariant& value,
                                                 const mu::engraving::EngravingItem* element) const;
    QVariant valueFromElementUnits(const mu::engraving::Pid& pid, const engraving::PropertyValue& value,
                                   const mu::engraving::EngravingItem* element) const;

    notation::INotationStylePtr style() const;
    bool updateStyleValue(const mu::engraving::Sid& sid, const QVariant& newValue);
    QVariant styleValue(const mu::engraving::Sid& sid) const;

    notation::INotationUndoStackPtr undoStack() const;
    void beginCommand();
    void endCommand();

    void updateNotation();
    notation::INotationPtr currentNotation() const;
    async::Notification currentNotationChanged() const;
    bool isMasterNotation() const;

    notation::INotationSelectionPtr selection() const;

    virtual void onCurrentNotationChanged();
    virtual void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                                   const mu::engraving::StyleIdSet& changedStyleIdSet);

    IElementRepositoryService* m_repository = nullptr;

    QList<mu::engraving::EngravingItem*> m_elementList;

protected slots:
    void onPropertyValueChanged(const mu::engraving::Pid pid, const QVariant& newValue);
    void updateProperties();

private:
    void setupCurrentNotationChangedConnection();

    void initPropertyItem(PropertyItem* propertyItem, std::function<void(const mu::engraving::Pid propertyId,
                                                                         const QVariant& newValue)> onPropertyChangedCallBack = nullptr);

    mu::engraving::Sid styleIdByPropertyId(const mu::engraving::Pid pid) const;
    mu::engraving::PropertyIdSet propertyIdSetFromStyleIdSet(const mu::engraving::StyleIdSet& styleIdSet) const;

    QString m_title;
    ui::IconCode::Code m_icon = ui::IconCode::Code::NONE;
    InspectorSectionType m_sectionType = InspectorSectionType::SECTION_UNDEFINED;
    InspectorModelType m_modelType = InspectorModelType::TYPE_UNDEFINED;
    mu::engraving::ElementType m_elementType = mu::engraving::ElementType::INVALID;
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
