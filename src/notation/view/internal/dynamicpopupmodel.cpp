#include "dynamicpopupmodel.h"
#include "dom/factory.h"
#include "src/engraving/dom/dynamic.h"
#include "types/symnames.h"
#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

static const QList<QList<DynamicPopupModel::PageItem> > DYN_POPUP_PAGES = {
    {   // Page 1
        { DynamicType::PP,     30, 1.5, DynamicPopupModel::Dynamic, "<sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
        { DynamicType::P,      21, 1.0, DynamicPopupModel::Dynamic, "<sym>dynamicPiano</sym>" },
        { DynamicType::MP,     29, 0.5, DynamicPopupModel::Dynamic, "<sym>dynamicMezzo</sym><sym>dynamicPiano</sym>" },
        { DynamicType::MF,     30, 0.5, DynamicPopupModel::Dynamic, "<sym>dynamicMezzo</sym><sym>dynamicForte</sym>" },
        { DynamicType::F,      23, 2.5, DynamicPopupModel::Dynamic, "<sym>dynamicForte</sym>" },
        { DynamicType::FF,     30, 2.5, DynamicPopupModel::Dynamic, "<sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    },
    {   // Page 2
        { DynamicType::FFF,    38, 2.5, DynamicPopupModel::Dynamic,
          "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
        { DynamicType::FFFF,   45, 2.5, DynamicPopupModel::Dynamic,
          "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
        { DynamicType::FFFFF,  53, 2.5, DynamicPopupModel::Dynamic,
          "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    },
    {   // Page 3
        { DynamicType::FP,     30, 2.5, DynamicPopupModel::Dynamic, "<sym>dynamicForte</sym><sym>dynamicPiano</sym>" },
        { DynamicType::PF,     32, 2.0, DynamicPopupModel::Dynamic, "<sym>dynamicPiano</sym><sym>dynamicForte</sym>" },
        { DynamicType::SF,     25, 0.5, DynamicPopupModel::Dynamic, "<sym>dynamicSforzando</sym><sym>dynamicForte</sym>" },
        { DynamicType::SFZ,    29, 0.5, DynamicPopupModel::Dynamic,
          "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
        { DynamicType::SFF,    33, 0.5, DynamicPopupModel::Dynamic,
          "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    },
    {   // Page 4
        { DynamicType::SFFZ,   37, 0.5, DynamicPopupModel::Dynamic,
          "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
        { DynamicType::SFP,    33, 0.5, DynamicPopupModel::Dynamic,
          "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym>" },
        { DynamicType::RFZ,    30, 0.5, DynamicPopupModel::Dynamic,
          "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
        { DynamicType::RF,     26, 0.5, DynamicPopupModel::Dynamic, "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym>" },
        { DynamicType::FZ,     26, 2.5, DynamicPopupModel::Dynamic, "<sym>dynamicForte</sym><sym>dynamicZ</sym>" },
    },
    {   // Page 5
        { DynamicType::PPPPPP, 74, 2.0, DynamicPopupModel::Dynamic,
          "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
        { DynamicType::FFFFFF, 60, 2.5, DynamicPopupModel::Dynamic,
          "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    },
    {   // Page 6
        { DynamicType::PPPPP,  64, 2.0, DynamicPopupModel::Dynamic,
          "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
        { DynamicType::PPPP,   52, 2.0, DynamicPopupModel::Dynamic,
          "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
        { DynamicType::PPP,    44, 2.0, DynamicPopupModel::Dynamic,
          "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    },
    {   // Page 7 - Hairpins
        { DynamicType::OTHER,  62, 0.0, DynamicPopupModel::Crescendo, "" },
        { DynamicType::OTHER,  62, 0.0, DynamicPopupModel::Decrescendo, "" },
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

QString DynamicPopupModel::xmlTextToQString(const std::string text, IEngravingFontPtr engravingFont) const
{
    const std::string startTag = "<sym>";
    const std::string endTag = "</sym>";

    String str;
    size_t pos = 0;

    while (true) {
        size_t startPos = text.find(startTag, pos);
        if (startPos == std::string::npos) {
            break;
        }
        size_t symNameStart = startPos + startTag.length();
        size_t endPos = text.find(endTag, symNameStart);
        if (endPos == std::string::npos) {
            break;
        }
        SymId id = SymNames::symIdByName(text.substr(symNameStart, endPos - symNameStart));
        str += engravingFont->toString(id);
        pos = endPos + endTag.length();
    }

    return str.toQString();
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
            QVariantMap variantMap {
                { "text", xmlTextToQString(item.text, engravingFont) },
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
