#ifndef POPUPVIEW_H
#define POPUPVIEW_H

#include <QQuickItem>
#include <QQuickView>

class PopupView : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QQmlComponent* contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged)

    Q_CLASSINFO("DefaultProperty", "contentItem")

public:
    explicit PopupView(QQuickItem* parent = nullptr);

    QQmlComponent* contentItem() const;

    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();

public slots:
    void setContentItem(QQmlComponent* contentItem);

signals:
    void contentItemChanged(QQmlComponent* contentItem);

private:
    void componentComplete() override;

    QQmlComponent* m_contentItem = nullptr;
    QQuickView* m_view = nullptr;
};

#endif // POPUPVIEW_H
