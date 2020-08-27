#ifndef NOTEHEADSETTINGSMODEL_H
#define NOTEHEADSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "noteheadtypesmodel.h"

class NoteheadSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject * noteheadTypesModel READ noteheadTypesModel NOTIFY noteheadTypesModelChanged)
    Q_PROPERTY(PropertyItem * isHeadHidden READ isHeadHidden CONSTANT)
    Q_PROPERTY(PropertyItem * headDirection READ headDirection CONSTANT)
    Q_PROPERTY(PropertyItem * headGroup READ headGroup CONSTANT)
    Q_PROPERTY(PropertyItem * headType READ headType CONSTANT)
    Q_PROPERTY(PropertyItem * dotPosition READ dotPosition CONSTANT)
    Q_PROPERTY(PropertyItem * horizontalOffset READ horizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * verticalOffset READ verticalOffset CONSTANT)

public:
    explicit NoteheadSettingsModel(QObject* parent, IElementRepositoryService* repository);

public:

    QObject* noteheadTypesModel() const;

    PropertyItem* isHeadHidden() const;
    PropertyItem* headDirection() const;
    PropertyItem* headGroup() const;
    PropertyItem* headType() const;
    PropertyItem* dotPosition() const;
    PropertyItem* horizontalOffset() const;
    PropertyItem* verticalOffset() const;

public slots:
    void setNoteheadTypesModel(NoteheadTypesModel* noteheadTypesModel);

signals:
    void noteheadTypesModelChanged(QObject* noteheadTypesModel);

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    NoteheadTypesModel* m_noteheadTypesModel = nullptr;

    PropertyItem* m_isHeadHidden = nullptr;
    PropertyItem* m_headDirection = nullptr;
    PropertyItem* m_headGroup = nullptr;
    PropertyItem* m_headType = nullptr;
    PropertyItem* m_dotPosition = nullptr;
    PropertyItem* m_horizontalOffset = nullptr;
    PropertyItem* m_verticalOffset = nullptr;
};

#endif // NOTEHEADSETTINGSMODEL_H
