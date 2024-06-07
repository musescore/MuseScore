#include "dynamicpopupmodel.h"
#include "src/engraving/dom/dynamic.h"
#include "types/symnames.h"
#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

const QList<QStringList> DynamicPopupModel::DYN_NAMES = {
    { "dynamicPP", "dynamicPiano", "dynamicMP", "dynamicMF", "dynamicForte", "dynamicFF" },
    { "dynamicFFF", "dynamicFFFF", "dynamicFFFFF" },
    { "dynamicFortePiano", "dynamicPF", "dynamicSforzando1", "dynamicSforzato", "dynamicSforzatoPiano" },
    { "dynamicSforzatoFF", "dynamicSforzandoPiano", "dynamicRinforzando2", "dynamicRinforzando1", "dynamicForzando" },
    { "dynamicPPPPPP", "dynamicFFFFFF" },
    { "dynamicPPPPP", "dynamicPPPP", "dynamicPPP" },
    { "dynamicCrescendoHairpin", "dynamicDiminuendoHairpin" },
};

DynamicPopupModel::DynamicPopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_DYNAMIC, parent)
{
}

QVariantList DynamicPopupModel::pages() const
{
    QVariantList variantPages;
    for (const QStringList& page : m_pages) {
        variantPages.append(QVariant::fromValue(page));
    }
    return variantPages;
}

void DynamicPopupModel::init()
{
    AbstractElementPopupModel::init();

    m_pages.clear();

    if (!m_item) {
        return;
    }

    for (const QStringList& dynNames : DYN_NAMES) {
        QStringList page;
        for (const QString& name : dynNames) {
            SymId id = SymNames::symIdByName(name);
            IEngravingFontPtr m_engravingFont
                = engravingFonts()->fontByName(m_item->score()->style().styleSt(Sid::MusicalSymbolFont).toStdString());
            QString str = m_engravingFont->toString(id);
            page.append(str);
        }
        m_pages.append(page);
    }

    emit pagesChanged();
}
