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

    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)
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
        engraving::DynamicType dynType;
        double width;
        double offset;
        ItemType itemType;
        const std::string text;
    };

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void addOrChangeDynamic(int page, int index);
    Q_INVOKABLE void addHairpinToDynamic(ItemType itemType);

    Q_INVOKABLE void showPreview(int page, int index);
    Q_INVOKABLE void hidePreview();

    QString fontFamily() const;
    QVariantList pages() const;

signals:
    void pagesChanged();

private:
    // Represents different pages of the popup, each containing dynamic/hairpin symbols as strings, width, offset and ItemType
    QVariantList m_pages;

    QString xmlTextToQString(const std::string text, engraving::IEngravingFontPtr engravingFont) const;
};
} // namespace mu::notation

#endif // MU_NOTATION_DYNAMICPOPUPMODEL_H
