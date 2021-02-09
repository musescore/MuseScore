#ifndef POPUPVIEW_H
#define POPUPVIEW_H

#include <QQuickItem>
#include <QQuickView>

class PopupView : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem * backgroundItem READ backgroundItem WRITE setBackgroundItem NOTIFY backgroundItemChanged)
    Q_PROPERTY(QQuickItem * contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged)
    Q_PROPERTY(ClosePolicy closePolicy READ closePolicy WRITE setClosePolicy NOTIFY closePolicyChanged)

    Q_PROPERTY(QPointF globalPos READ globalPos WRITE setGlobalPos NOTIFY globalPosChanged)
    Q_PROPERTY(qreal positionDisplacementX READ positionDisplacementX WRITE setPositionDisplacementX NOTIFY positionDisplacementXChanged)
    Q_PROPERTY(qreal positionDisplacementY READ positionDisplacementY WRITE setPositionDisplacementY NOTIFY positionDisplacementYChanged)

    Q_PROPERTY(bool isOpened READ isOpened WRITE setIsOpened NOTIFY isOpenedChanged)

    Q_PROPERTY(qreal padding READ padding WRITE setPadding NOTIFY paddingChanged)

    Q_CLASSINFO("DefaultProperty", "contentItem")

    Q_ENUMS(ClosePolicy)
public:
    enum ClosePolicy {
        NoAutoClose = 0,
        CloseOnPressOutsideParent,
        CloseOnReleaseOutsideParent
    };

    explicit PopupView(QQuickItem* parent = nullptr);

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

    QQuickItem* backgroundItem() const;
    QQuickItem* contentItem() const;
    ClosePolicy closePolicy() const;

    bool isOpened() const;

    qreal padding() const;

    QPointF globalPos() const;
    qreal positionDisplacementX() const;
    qreal positionDisplacementY() const;

public slots:
    void setBackgroundItem(QQuickItem* backgroundItem);
    void setContentItem(QQuickItem* contentItem);
    void setClosePolicy(ClosePolicy closePolicy);

    void setIsOpened(bool isOpened);
    void setPadding(qreal padding);

    void setGlobalPos(QPointF globalPos);
    void setPositionDisplacementX(qreal positionDisplacementX);
    void setPositionDisplacementY(qreal positionDisplacementY);

signals:
    void backgroundItemChanged(QQuickItem* backgroundItem);
    void contentItemChanged(QQuickItem* contentItem);
    void closePolicyChanged(ClosePolicy closePolicy);

    void isOpenedChanged(bool isOpened);

    void aboutToShow();
    void aboutToHide();
    void opened();
    void closed();

    void paddingChanged(qreal padding);

    void globalPosChanged(QPointF globalPos);
    void positionDisplacementXChanged(qreal positionDisplacementX);
    void positionDisplacementYChanged(qreal positionDisplacementY);

private:
    void componentComplete() override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    bool isMouseWithinBoundaries(const QPoint& mousePos) const;

    void setupContentComponent(QQmlEngine* engine);
    void setupContainerView(QQmlEngine* engine);
    QQuickItem* loadContentItem(QQmlContext* ctx);

    QQmlComponent* m_contentComponent = nullptr;
    QQuickView* m_containerView = nullptr;

    QQuickItem* m_backgroundItem = nullptr;
    QQuickItem* m_contentItem = nullptr;

    ClosePolicy m_closePolicy = ClosePolicy::CloseOnPressOutsideParent;
    bool m_isOpened = false;
    qreal m_padding = 0.0;
    QPointF m_globalPos;
    qreal m_positionDisplacementX = 0.0;
    qreal m_positionDisplacementY = 0.0;
};

#endif // POPUPVIEW_H
