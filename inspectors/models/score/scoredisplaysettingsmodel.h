#ifndef scoreSETTINGSMODEL_H
#define scoreSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

#include <QAction>

class scoreSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(bool shouldShowInvisible READ shouldShowInvisible WRITE setShouldShowInvisible NOTIFY shouldShowInvisibleChanged)
    Q_PROPERTY(bool shouldShowUnprintable READ shouldShowUnprintable WRITE setShouldShowUnprintable NOTIFY shouldShowUnprintableChanged)
    Q_PROPERTY(bool shouldShowFrames READ shouldShowFrames WRITE setShouldShowFrames NOTIFY shouldShowFramesChanged)
    Q_PROPERTY(bool shouldShowPageMargins READ shouldShowPageMargins WRITE setShouldShowPageMargins NOTIFY shouldShowPageMarginsChanged)
public:
    explicit scoreSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    bool hasAcceptableElements() const override;

    bool shouldShowInvisible() const;
    bool shouldShowUnprintable() const;
    bool shouldShowFrames() const;
    bool shouldShowPageMargins() const;

public slots:
    void setShouldShowInvisible(bool shouldShowInvisible);
    void setShouldShowUnprintable(bool shouldShowUnprintable);
    void setShouldShowFrames(bool shouldShowFrames);
    void setShouldShowPageMargins(bool shouldShowPageMargins);

signals:
    void shouldShowInvisibleChanged(bool shouldShowInvisible);
    void shouldShowUnprintableChanged(bool shouldShowUnprintable);
    void shouldShowFramesChanged(bool shouldShowFrames);
    void shouldShowPageMarginsChanged(bool shouldShowPageMargins);

private:
    QAction* m_shouldShowInvisible = nullptr;
    QAction* m_shouldShowUnprintable = nullptr;
    QAction* m_shouldShowFrames = nullptr;
    QAction* m_shouldShowPageMargins = nullptr;
};

#endif // scoreSETTINGSMODEL_H
