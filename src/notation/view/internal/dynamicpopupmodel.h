#ifndef MU_NOTATION_DYNAMICPOPUPMODEL_H
#define MU_NOTATION_DYNAMICPOPUPMODEL_H

#include <QList>
#include <QObject>
#include <QVariantList>

#include "view/abstractelementpopupmodel.h"

namespace mu::notation {
class DynamicPopupModel : public AbstractElementPopupModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList pages READ pages NOTIFY pagesChanged)

public:
    explicit DynamicPopupModel(QObject* parent = nullptr);

    enum ItemType {
        Dynamic,
        Crescendo,
        Decrescendo
    };
    Q_ENUM(ItemType)

    struct PageItem {
        QString name;
        double width;
        double offset;
        ItemType itemType;
        engraving::DynamicType dynType;
    };

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void addOrChangeDynamic(int page, int index);
    Q_INVOKABLE void addHairpinToDynamic(ItemType itemType);

    Q_INVOKABLE void showPreview(int page, int index);
    Q_INVOKABLE void hidePreview();

    QVariantList pages() const;

signals:
    void pagesChanged();

private:
    // Represents different pages of the popup, each containing dynamic/hairpin symbols as strings, width, offset and ItemType
    QVariantList m_pages;
};
} // namespace mu::notation

#endif // MU_NOTATION_DYNAMICPOPUPMODEL_H
