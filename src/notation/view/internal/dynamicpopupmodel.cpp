#include "dynamicpopupmodel.h"
#include "src/engraving/dom/dynamic.h"
#include "types/symnames.h"
#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

static const QList<QList<DynamicPopupModel::PageItem>> DYN_POPUP_PAGES = {
    {   // Page 1
        { "dynamicPP",                30, 1.5, DynamicPopupModel::Dynamic },
        { "dynamicPiano",             21, 1.0, DynamicPopupModel::Dynamic },
        { "dynamicMP",                29, 0.5, DynamicPopupModel::Dynamic },
        { "dynamicMF",                30, 0.5, DynamicPopupModel::Dynamic },
        { "dynamicForte",             23, 2.5, DynamicPopupModel::Dynamic },
        { "dynamicFF",                30, 2.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 2
        { "dynamicFFF",               38, 2.5, DynamicPopupModel::Dynamic },
        { "dynamicFFFF",              45, 2.5, DynamicPopupModel::Dynamic },
        { "dynamicFFFFF",             53, 2.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 3
        { "dynamicFortePiano",        30, 2.5, DynamicPopupModel::Dynamic },
        { "dynamicPF",                32, 2.0, DynamicPopupModel::Dynamic },
        { "dynamicSforzando1",        25, 0.5, DynamicPopupModel::Dynamic },
        { "dynamicSforzato",          29, 0.5, DynamicPopupModel::Dynamic },
        { "dynamicSforzatoFF",        37, 0.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 4
        { "dynamicSforzatoPiano",     38, 0.5, DynamicPopupModel::Dynamic },
        { "dynamicSforzandoPiano",    33, 0.5, DynamicPopupModel::Dynamic },
        { "dynamicRinforzando2",      30, 0.5, DynamicPopupModel::Dynamic },
        { "dynamicRinforzando1",      26, 0.5, DynamicPopupModel::Dynamic },
        { "dynamicForzando",          26, 2.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 5
         { "dynamicPPPPPP",           74, 2.0, DynamicPopupModel::Dynamic },
         { "dynamicFFFFFF",           60, 2.5, DynamicPopupModel::Dynamic },
    },
    {   // Page 6
        { "dynamicPPPPP",             64, 2.0, DynamicPopupModel::Dynamic },
        { "dynamicPPPP",              52, 2.0, DynamicPopupModel::Dynamic },
        { "dynamicPPP",               44, 2.0, DynamicPopupModel::Dynamic },
    },
    {   // Page 7 - Hairpins
        { "dynamicCrescendoHairpin",  62, 0.0, DynamicPopupModel::Crescendo },
        { "dynamicDiminuendoHairpin", 62, 0.0, DynamicPopupModel::Decrescendo },
    },
};

DynamicPopupModel::DynamicPopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_DYNAMIC, parent)
{
}

QVariantList DynamicPopupModel::pages() const
{
    return m_pages;
}

void DynamicPopupModel::init()
{
    AbstractElementPopupModel::init();

    m_pages.clear();

    if (!m_item) {
        return;
    }

    IEngravingFontPtr engravingFont = m_item->score()->engravingFont();

    for (const QList<DynamicPopupModel::PageItem>& page : DYN_POPUP_PAGES) {
        QVariantList variantPage;
        for(const DynamicPopupModel::PageItem& item : page) {
            SymId id = SymNames::symIdByName(item.name);
            QVariantMap variantMap {
                { "symbol", engravingFont->toString(id).toQString() },
                { "width", item.width },
                { "offset", item.offset },
                { "type", item.type }
            };
            variantPage.append(variantMap);
        }
        m_pages.append(QVariant::fromValue(variantPage));
    }

    emit pagesChanged();
}
