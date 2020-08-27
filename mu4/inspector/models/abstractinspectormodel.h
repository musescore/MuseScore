#ifndef ABSTRACTINSPECTORMODEL_H
#define ABSTRACTINSPECTORMODEL_H

#include <QList>
#include <functional>
#include "libmscore/element.h"
#include "libmscore/score.h"
#include "libmscore/style.h"
#include "libmscore/property.h"

#include "internal/interfaces/ielementrepositoryservice.h"
#include "iinspectoradapter.h"
#include "notation/inotation.h"
#include "modularity/ioc.h"
#include "models/propertyitem.h"

class AbstractInspectorModel : public QObject
{
    Q_OBJECT

    INJECT(inspector, mu::inspector::IInspectorAdapter, adapter)

    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(InspectorSectionType sectionType READ sectionType CONSTANT)
    Q_PROPERTY(InspectorModelType modelType READ modelType CONSTANT)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)

    Q_ENUMS(InspectorSectionType)
    Q_ENUMS(InspectorModelType)

public:
    enum InspectorSectionType {
        SECTION_UNDEFINED = -1,
        SECTION_GENERAL,
        SECTION_TEXT,
        SECTION_NOTATION,
        SECTION_SCORE_DISPLAY,
        SECTION_SCORE_APPEARANCE
    };

    enum InspectorModelType {
        TYPE_UNDEFINED = -1,
        TYPE_NOTE,
        TYPE_BEAM,
        TYPE_NOTEHEAD,
        TYPE_STEM,
        TYPE_HOOK,
        TYPE_FERMATA,
        TYPE_TEMPO,
        TYPE_GLISSANDO,
        TYPE_BARLINE,
        TYPE_STAFF,
        TYPE_MARKER,
        TYPE_SECTIONBREAK,
        TYPE_JUMP,
        TYPE_KEYSIGNATURE,
        TYPE_ACCIDENTAL,
        TYPE_FRET_DIAGRAM,
        TYPE_PEDAL,
        TYPE_SPACER,
        TYPE_CLEF,
        TYPE_HAIRPIN,
        TYPE_CRESCENDO,
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
        TYPE_BRACE,
        TYPE_TIME_SIGNATURE,
        TYPE_MMREST,
        TYPE_BEND,
        TYPE_TREMOLOBAR,
        TYPE_TREMOLO
    };

    explicit AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository = nullptr);

    Q_INVOKABLE virtual void requestResetToDefaults();

    QString title() const;
    InspectorSectionType sectionType() const;
    InspectorModelType modelType() const;

    bool isEmpty() const;

    static QList<Ms::ElementType> supportedElementTypesBySectionType(const InspectorSectionType sectionType);
    static InspectorSectionType sectionTypeFromElementType(const Ms::ElementType elementType);

    virtual bool hasAcceptableElements() const;

    virtual void createProperties() = 0;
    virtual void requestElements() = 0;
    virtual void loadProperties() = 0;
    virtual void resetProperties() = 0;

public slots:
    void setTitle(QString title);
    void setSectionType(InspectorSectionType sectionType);
    void setModelType(InspectorModelType modelType);
    void setIsEmpty(bool isEmpty);

signals:
    void elementsModified();
    void modelReseted();
    void isEmptyChanged(bool isEmpty);

    void requestReloadPropertyItems();

protected:
    PropertyItem* buildPropertyItem(const Ms::Pid& pid,std::function<void(const int propertyId,
                                                                          const QVariant& newValue)> onPropertyChangedCallBack = nullptr);

    void loadPropertyItem(PropertyItem* propertyItem,std::function<QVariant(const QVariant&)> convertElementPropertyValueFunc = nullptr);

    bool isNotationExisting() const;

    QVariant valueToElementUnits(const Ms::Pid& pid, const QVariant& value, const Ms::Element* element) const;
    QVariant valueFromElementUnits(const Ms::Pid& pid, const QVariant& value, const Ms::Element* element) const;

    void updateStyleValue(const Ms::Sid& sid, const QVariant& newValue);
    QVariant styleValue(const Ms::Sid& sid) const;

    IElementRepositoryService* m_repository;

    QList<Ms::Element*> m_elementList;

protected slots:
    void onResetToDefaults(const QList<Ms::Pid>& pidList);
    void onPropertyValueChanged(const Ms::Pid pid, const QVariant& newValue);
    void updateProperties();

private:
    Ms::Sid styleIdByPropertyId(const Ms::Pid pid) const;

    QString m_title;
    InspectorSectionType m_sectionType = SECTION_UNDEFINED;
    InspectorModelType m_modelType = TYPE_UNDEFINED;
    bool m_isEmpty = false;
};

#endif // ABSTRACTINSPECTORMODEL_H
