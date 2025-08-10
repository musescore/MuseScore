#pragma once

#include "view/abstractelementpopupmodel.h"
#include "framework/uicomponents/view/menuitem.h"
#include "dom/tie.h"

namespace mu::notation {
class PartialTiePopupModel : public AbstractElementPopupModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList items READ items NOTIFY itemsChanged)
    Q_PROPERTY(bool tieDirection READ tieDirection NOTIFY tieDirectionChanged)
    Q_PROPERTY(QPointF dialogPosition READ dialogPosition CONSTANT)

public:
    explicit PartialTiePopupModel(QObject* parent = nullptr);

    QVariantList items() const;
    bool tieDirection() const;
    static bool canOpen(const EngravingItem* element);
    QPointF dialogPosition() const;

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void toggleItemChecked(const QString& id);

    Q_INVOKABLE void onClosed();

signals:
    void tieDirectionChanged(bool direction);
    void itemsChanged();

private:
    void load();
    muse::uicomponents::MenuItemList makeMenuItems();
    muse::uicomponents::MenuItem* makeMenuItem(const engraving::TieJumpPoint* jumpPoint);
    mu::engraving::Tie* tie() const;

    muse::uicomponents::MenuItemList m_items;
};
}
