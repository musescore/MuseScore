#ifndef MU_NOTATION_DYNAMICPOPUPMODEL_H
#define MU_NOTATION_DYNAMICPOPUPMODEL_H

#include <engraving/iengravingfontsprovider.h>
#include <engraving/style/style.h>
#include <view/abstractelementpopupmodel.h>

#include <QList>
#include <QObject>
#include <QVariantList>

using namespace mu::engraving;

namespace mu::notation {
class DynamicPopupModel : public AbstractElementPopupModel
{
    Q_OBJECT

    INJECT(engraving::IEngravingFontsProvider, engravingFonts)

    Q_PROPERTY(QVariantList pages READ pages NOTIFY pagesChanged)

public:
    explicit DynamicPopupModel(QObject *parent = nullptr);

    Q_INVOKABLE void init() override;

    QVariantList pages() const;

signals:
    void pagesChanged();

private:
    QList<QStringList> m_pages;

    static const QList<QStringList> DYN_NAMES;
};
} // namespace mu::notation

#endif // MU_NOTATION_DYNAMICPOPUPMODEL_H
