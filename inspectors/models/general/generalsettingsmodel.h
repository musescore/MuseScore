#ifndef GENERALSETTINGSMODEL_H
#define GENERALSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "playback/playbackproxymodel.h"

class GeneralSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem* isVisible READ isVisible CONSTANT)
    Q_PROPERTY(PropertyItem* isAutoPlaceAllowed READ isAutoPlaceAllowed CONSTANT)
    Q_PROPERTY(PropertyItem* isPlayable READ isPlayable CONSTANT)
    Q_PROPERTY(PropertyItem* isSmall READ isSmall CONSTANT)

    Q_PROPERTY(QObject* playbackProxyModel READ playbackProxyModel NOTIFY playbackProxyModelChanged)

    public:
        explicit GeneralSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* isVisible() const;
    PropertyItem* isAutoPlaceAllowed() const;
    PropertyItem* isPlayable() const;
    PropertyItem* isSmall() const;

    QObject* playbackProxyModel() const;

public slots:
    void setPlaybackProxyModel(PlaybackProxyModel* playbackProxyModel);

signals:
    void playbackProxyModelChanged(QObject* playbackProxyModel);

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_isVisible = nullptr;
    PropertyItem* m_isAutoPlaceAllowed = nullptr;
    PropertyItem* m_isPlayable = nullptr;
    PropertyItem* m_isSmall = nullptr;

    PlaybackProxyModel* m_playbackProxyModel = nullptr;
};

#endif // GENERALSETTINGSMODEL_H
