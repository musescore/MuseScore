#ifndef GENERALSETTINGSMODEL_H
#define GENERALSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"
#include "playback/playbackproxymodel.h"
#include "appearance/appearancesettingsmodel.h"

class GeneralSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isVisible READ isVisible CONSTANT)
    Q_PROPERTY(PropertyItem * isAutoPlaceAllowed READ isAutoPlaceAllowed CONSTANT)
    Q_PROPERTY(PropertyItem * isPlayable READ isPlayable CONSTANT)
    Q_PROPERTY(PropertyItem * isSmall READ isSmall CONSTANT)

    Q_PROPERTY(QObject * playbackProxyModel READ playbackProxyModel NOTIFY playbackProxyModelChanged)
    Q_PROPERTY(QObject * appearanceSettingsModel READ appearanceSettingsModel NOTIFY appearanceSettingsModelChanged)

public:
    explicit GeneralSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* isVisible() const;
    PropertyItem* isAutoPlaceAllowed() const;
    PropertyItem* isPlayable() const;
    PropertyItem* isSmall() const;

    QObject* playbackProxyModel() const;
    QObject* appearanceSettingsModel() const;

public slots:
    void setPlaybackProxyModel(PlaybackProxyModel* playbackProxyModel);
    void setAppearanceSettingsModel(AppearanceSettingsModel* appearanceSettingsModel);

signals:
    void playbackProxyModelChanged(QObject* playbackProxyModel);
    void appearanceSettingsModelChanged(QObject* appearanceSettingsModel);

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
    AppearanceSettingsModel* m_appearanceSettingsModel = nullptr;
};

#endif // GENERALSETTINGSMODEL_H
