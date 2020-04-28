#ifndef FRETDIAGRAMSETTINGSMODEL_H
#define FRETDIAGRAMSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "types/fretdiagramtypes.h"

#include "libmscore/fret.h"
#include "libmscore/score.h"

class FretDiagramSettingsModel : public AbstractInspectorModel {
    Q_OBJECT

    Q_PROPERTY(PropertyItem* strings READ strings CONSTANT)
    Q_PROPERTY(PropertyItem* frets READ frets CONSTANT)
    Q_PROPERTY(PropertyItem* showNut READ showNut CONSTANT)
    Q_PROPERTY(PropertyItem* offset READ offset CONSTANT)
    Q_PROPERTY(PropertyItem* numPos READ numPos CONSTANT)

public:
    explicit FretDiagramSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* strings() const;
    PropertyItem* frets() const;
    PropertyItem* showNut() const;
    PropertyItem* offset() const;
    PropertyItem* numPos() const;

    Q_INVOKABLE bool canvasVisible() const;

    Ms::FretDiagram* fretDiagram() const;
    Q_INVOKABLE FretDiagramTypes::FretDot dot(int string, int fret) const;
    Q_INVOKABLE FretDiagramTypes::FretMarker marker(int string) const;
    Q_INVOKABLE bool barreExists(int fret) const;
    Q_INVOKABLE int barreStartString(int fret) const;
    Q_INVOKABLE int barreEndString(int fret) const;
    Q_INVOKABLE qreal barreLineWidth() const;

public slots:
    void setDot(int string, int fret, bool add, FretDiagramTypes::FretDot dType);
    void setMarker(int string, FretDiagramTypes::FretMarker mType);
    void onElementsUpdated();

signals:
    void selectionChanged();

private:
    PropertyItem* m_strings = nullptr;
    PropertyItem* m_frets = nullptr;
    PropertyItem* m_showNut = nullptr;
    PropertyItem* m_offset = nullptr;
    PropertyItem* m_numPos = nullptr;
};

#endif // FRETDIAGRAMSETTINGSMODEL_H
