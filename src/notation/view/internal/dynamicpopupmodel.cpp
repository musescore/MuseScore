#include "dynamicpopupmodel.h"
#include "dom/factory.h"
#include "src/engraving/dom/dynamic.h"
#include "types/symnames.h"
#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

static const QList<QList<DynamicPopupModel::PageItem> > DYN_POPUP_PAGES = {
    {   // Page 1
        { "dynamicPP",                30, 1.5, DynamicPopupModel::Dynamic,     DynamicType::PP },
        { "dynamicPiano",             21, 1.0, DynamicPopupModel::Dynamic,     DynamicType::P },
        { "dynamicMP",                29, 0.5, DynamicPopupModel::Dynamic,     DynamicType::MP },
        { "dynamicMF",                30, 0.5, DynamicPopupModel::Dynamic,     DynamicType::MF },
        { "dynamicForte",             23, 2.5, DynamicPopupModel::Dynamic,     DynamicType::F },
        { "dynamicFF",                30, 2.5, DynamicPopupModel::Dynamic,     DynamicType::FF },
    },
    {   // Page 2
        { "dynamicFFF",               38, 2.5, DynamicPopupModel::Dynamic,     DynamicType::FFF },
        { "dynamicFFFF",              45, 2.5, DynamicPopupModel::Dynamic,     DynamicType::FFFF },
        { "dynamicFFFFF",             53, 2.5, DynamicPopupModel::Dynamic,     DynamicType::FFFFF },
    },
    {   // Page 3
        { "dynamicFortePiano",        30, 2.5, DynamicPopupModel::Dynamic,     DynamicType::FP },
        { "dynamicPF",                32, 2.0, DynamicPopupModel::Dynamic,     DynamicType::PF },
        { "dynamicSforzando1",        25, 0.5, DynamicPopupModel::Dynamic,     DynamicType::SF },
        { "dynamicSforzato",          29, 0.5, DynamicPopupModel::Dynamic,     DynamicType::SFZ },
        { "dynamicSforzatoFF",        37, 0.5, DynamicPopupModel::Dynamic,     DynamicType::SFFZ },
    },
    {   // Page 4
        { "dynamicSforzatoPiano",     38, 0.5, DynamicPopupModel::Dynamic,     DynamicType::OTHER },
        { "dynamicSforzandoPiano",    33, 0.5, DynamicPopupModel::Dynamic,     DynamicType::SFP },
        { "dynamicRinforzando2",      30, 0.5, DynamicPopupModel::Dynamic,     DynamicType::RFZ },
        { "dynamicRinforzando1",      26, 0.5, DynamicPopupModel::Dynamic,     DynamicType::RF },
        { "dynamicForzando",          26, 2.5, DynamicPopupModel::Dynamic,     DynamicType::FZ },
    },
    {   // Page 5
        { "dynamicPPPPPP",            74, 2.0, DynamicPopupModel::Dynamic,     DynamicType::PPPPPP },
        { "dynamicFFFFFF",            60, 2.5, DynamicPopupModel::Dynamic,     DynamicType::FFFFFF },
    },
    {   // Page 6
        { "dynamicPPPPP",             64, 2.0, DynamicPopupModel::Dynamic,     DynamicType::PPPPP },
        { "dynamicPPPP",              52, 2.0, DynamicPopupModel::Dynamic,     DynamicType::PPPP },
        { "dynamicPPP",               44, 2.0, DynamicPopupModel::Dynamic,     DynamicType::PPP },
    },
    {   // Page 7 - Hairpins
        { "dynamicCrescendoHairpin",  62, 0.0, DynamicPopupModel::Crescendo,   DynamicType::OTHER },
        { "dynamicDiminuendoHairpin", 62, 0.0, DynamicPopupModel::Decrescendo, DynamicType::OTHER },
    },
};

DynamicPopupModel::DynamicPopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_DYNAMIC, parent)
{
}

QString DynamicPopupModel::fontFamily() const
{
    return QString::fromStdString(m_item->score()->engravingFont()->family());
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
        for (const DynamicPopupModel::PageItem& item : page) {
            SymId id = SymNames::symIdByName(item.name);
            QVariantMap variantMap {
                { "symbol", engravingFont->toString(id).toQString() },
                { "width", item.width },
                { "offset", item.offset },
                { "type", item.itemType },
            };
            variantPage.append(variantMap);
        }
        m_pages.append(QVariant::fromValue(variantPage));
    }

    emit pagesChanged();
}

void DynamicPopupModel::addOrChangeDynamic(int page, int index)
{
    IF_ASSERT_FAILED(m_item) {
        return;
    }

    if (!m_item->isDynamic()) {
        return;
    }

    beginCommand();
    m_item->undoChangeProperty(Pid::TEXT, Dynamic::dynamicText(DYN_POPUP_PAGES[page][index].dynType));
    m_item->undoChangeProperty(Pid::DYNAMIC_TYPE, DYN_POPUP_PAGES[page][index].dynType);
    endCommand();

    if (m_item->flag(ElementFlag::IS_PREVIEW)) {
        m_item->setFlag(ElementFlag::IS_PREVIEW, false);
    }

    INotationInteractionPtr interaction = currentNotation()->interaction();

    interaction->flipHairpinsType(toDynamic(m_item));

    // Hide the bounding box which appears when called using Ctrl+D shortcut
    if (interaction->isTextEditingStarted()) {
        interaction->endEditText();
        interaction->startEditGrip(m_item, Grip::DRAG);
    }

    updateNotation();
}

void DynamicPopupModel::addHairpinToDynamic(ItemType itemType)
{
    IF_ASSERT_FAILED(m_item) {
        return;
    }

    if (!m_item->isDynamic()) {
        return;
    }

    beginCommand();

    Hairpin* pin = Factory::createHairpin(m_item->score()->dummy()->segment());
    if (itemType == ItemType::Crescendo) {
        pin->setHairpinType(HairpinType::CRESC_HAIRPIN);
    } else if (itemType == ItemType::Decrescendo) {
        pin->setHairpinType(HairpinType::DECRESC_HAIRPIN);
    }

    m_item->score()->addHairpinToDynamic(pin, toDynamic(m_item));

    endCommand();
    updateNotation();
}

void DynamicPopupModel::showPreview(int page, int index)
{
    IF_ASSERT_FAILED(m_item) {
        return;
    }

    if (!m_item->isDynamic() || !m_item->flag(ElementFlag::IS_PREVIEW)) {
        return;
    }

    beginCommand();
    m_item->setProperty(Pid::TEXT, Dynamic::dynamicText(DYN_POPUP_PAGES[page][index].dynType));
    m_item->setProperty(Pid::DYNAMIC_TYPE, DYN_POPUP_PAGES[page][index].dynType);
    endCommand();
    updateNotation();
}

void DynamicPopupModel::hidePreview()
{
    IF_ASSERT_FAILED(m_item) {
        return;
    }

    if (!m_item->isDynamic() || !m_item->flag(ElementFlag::IS_PREVIEW)) {
        return;
    }

    beginCommand();
    m_item->setProperty(Pid::TEXT, String());
    m_item->setProperty(Pid::DYNAMIC_TYPE, DynamicType::OTHER);
    endCommand();
    updateNotation();
}
