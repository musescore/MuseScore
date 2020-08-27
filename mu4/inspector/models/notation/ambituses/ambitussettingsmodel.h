#ifndef AMBITUSSETTINGSMODEL_H
#define AMBITUSSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "models/notation/notes/noteheads/noteheadtypesmodel.h"

class AmbitusSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(NoteheadTypesModel * noteheadGroupsModel READ noteheadGroupsModel NOTIFY noteheadGroupsModelChanged)
    Q_PROPERTY(PropertyItem * noteheadGroup READ noteheadGroup CONSTANT)
    Q_PROPERTY(PropertyItem * noteheadType READ noteheadType CONSTANT)

    Q_PROPERTY(PropertyItem * topTpc READ topTpc CONSTANT)
    Q_PROPERTY(PropertyItem * bottomTpc READ bottomTpc CONSTANT)
    Q_PROPERTY(PropertyItem * topOctave READ topOctave CONSTANT)
    Q_PROPERTY(PropertyItem * bottomOctave READ bottomOctave CONSTANT)
    Q_PROPERTY(PropertyItem * topPitch READ topPitch CONSTANT)
    Q_PROPERTY(PropertyItem * bottomPitch READ bottomPitch CONSTANT)

    Q_PROPERTY(PropertyItem * direction READ direction CONSTANT)
    Q_PROPERTY(PropertyItem * lineThickness READ lineThickness CONSTANT)

public:
    explicit AmbitusSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void matchRangesToStaff();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    NoteheadTypesModel* noteheadGroupsModel() const;
    PropertyItem* noteheadGroup() const;
    PropertyItem* noteheadType() const;

    PropertyItem* topTpc() const;
    PropertyItem* bottomTpc() const;
    PropertyItem* topOctave() const;
    PropertyItem* bottomOctave() const;
    PropertyItem* topPitch() const;
    PropertyItem* bottomPitch() const;

    PropertyItem* direction() const;
    PropertyItem* lineThickness() const;

public slots:
    void setNoteheadGroupsModel(NoteheadTypesModel* noteheadGroupsModel);

signals:
    void noteheadGroupsModelChanged(NoteheadTypesModel* noteheadGroupsModel);

private:
    NoteheadTypesModel* m_noteheadGroupsModel = nullptr;
    PropertyItem* m_noteheadGroup = nullptr;
    PropertyItem* m_noteheadType = nullptr;

    PropertyItem* m_topTpc = nullptr;
    PropertyItem* m_bottomTpc = nullptr;
    PropertyItem* m_topOctave = nullptr;
    PropertyItem* m_bottomOctave = nullptr;
    PropertyItem* m_topPitch = nullptr;
    PropertyItem* m_bottomPitch = nullptr;

    PropertyItem* m_direction = nullptr;
    PropertyItem* m_lineThickness = nullptr;
};

#endif // AMBITUSSETTINGSMODEL_H
