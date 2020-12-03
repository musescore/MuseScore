#ifndef PLAYBACKPROXYMODEL_H
#define PLAYBACKPROXYMODEL_H

#include <QObject>
#include "models/abstractinspectormodel.h"

#include "internal_models/noteplaybackmodel.h"
#include "internal_models/arpeggioplaybackmodel.h"
#include "internal_models/fermataplaybackmodel.h"
#include "internal_models/breathplaybackmodel.h"
#include "internal_models/glissandoplaybackmodel.h"
#include "internal_models/dynamicplaybackmodel.h"
#include "internal_models/hairpinplaybackmodel.h"

class PlaybackProxyModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject * notePlaybackModel READ notePlaybackModel NOTIFY notePlaybackModelChanged)
    Q_PROPERTY(QObject * arpeggioPlaybackModel READ arpeggioPlaybackModel NOTIFY arpeggioPlaybackModelChanged)
    Q_PROPERTY(QObject * fermataPlaybackModel READ fermataPlaybackModel NOTIFY fermataPlaybackModelChanged)
    Q_PROPERTY(QObject * breathPlaybackModel READ breathPlaybackModel NOTIFY breathPlaybackModelChanged)
    Q_PROPERTY(QObject * glissandoPlaybackModel READ glissandoPlaybackModel NOTIFY glissandoPlaybackModelChanged)
    Q_PROPERTY(QObject * dynamicPlaybackModel READ dynamicPlaybackModel NOTIFY dynamicPlaybackModelChanged)
    Q_PROPERTY(QObject * hairpinPlaybackModel READ hairpinPlaybackModel NOTIFY hairpinPlaybackModelChanged)

public:
    explicit PlaybackProxyModel(QObject* parent, IElementRepositoryService* repository);

    void requestResetToDefaults() override;
    bool hasAcceptableElements() const override;

    QObject* notePlaybackModel() const;
    QObject* arpeggioPlaybackModel() const;
    QObject* fermataPlaybackModel() const;
    QObject* breathPlaybackModel() const;
    QObject* glissandoPlaybackModel() const;
    QObject* dynamicPlaybackModel() const;
    QObject* hairpinPlaybackModel() const;

public slots:
    void setNotePlaybackModel(NotePlaybackModel* notePlaybackModel);
    void setArpeggioPlaybackModel(ArpeggioPlaybackModel* arpeggioPlaybackModel);
    void setFermataPlaybackModel(FermataPlaybackModel* fermataPlaybackModel);
    void setBreathPlaybackModel(BreathPlaybackModel* breathPlaybackModel);
    void setGlissandoPlaybackModel(GlissandoPlaybackModel* glissandoPlaybackModel);
    void setDynamicPlaybackModel(DynamicPlaybackModel* dynamicPlaybackModel);
    void setHairpinPlaybackModel(HairpinPlaybackModel* hairpinPlaybackModel);

signals:
    void notePlaybackModelChanged(QObject* notePlaybackModel);
    void arpeggioPlaybackModelChanged(QObject* arpeggioPlaybackModel);
    void fermataPlaybackModelChanged(QObject* fermataPlaybackModel);
    void breathPlaybackModelChanged(QObject* breathPlaybackModel);
    void glissandoPlaybackModelChanged(QObject* glissandoPlaybackModel);
    void dynamicPlaybackModelChanged(QObject* dynamicPlaybackModel);
    void hairpinPlaybackModelChanged(QObject* hairpinPlaybackModel);

private:
    void createProperties() override {}
    void requestElements() override {}
    void loadProperties() override {}
    void resetProperties() override {}

    NotePlaybackModel* m_notePlaybackModel = nullptr;
    ArpeggioPlaybackModel* m_arpeggioPlaybackModel = nullptr;
    FermataPlaybackModel* m_fermataPlaybackModel = nullptr;
    BreathPlaybackModel* m_breathPlaybackModel = nullptr;
    GlissandoPlaybackModel* m_glissandoPlaybackModel = nullptr;
    DynamicPlaybackModel* m_dynamicPlaybackModel = nullptr;
    HairpinPlaybackModel* m_hairpinPlaybackModel = nullptr;
};

#endif // PLAYBACKPROXYMODEL_H
