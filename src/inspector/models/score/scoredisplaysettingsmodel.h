#ifndef scoreSETTINGSMODEL_H
#define scoreSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class ScoreSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(bool shouldShowInvisible READ shouldShowInvisible WRITE setShouldShowInvisible NOTIFY shouldShowInvisibleChanged)
    Q_PROPERTY(bool shouldShowUnprintable READ shouldShowUnprintable WRITE setShouldShowUnprintable NOTIFY shouldShowUnprintableChanged)
    Q_PROPERTY(bool shouldShowFrames READ shouldShowFrames WRITE setShouldShowFrames NOTIFY shouldShowFramesChanged)
    Q_PROPERTY(bool shouldShowPageMargins READ shouldShowPageMargins WRITE setShouldShowPageMargins NOTIFY shouldShowPageMarginsChanged)
public:
    explicit ScoreSettingsModel(QObject* parent, IElementRepositoryService* repository);

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
    bool m_shouldShowInvisible = true;
    bool m_shouldShowUnprintable = true;
    bool m_shouldShowFrames = true;
    bool m_shouldShowPageMargins = false;
};

#endif // scoreSETTINGSMODEL_H
