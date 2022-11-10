#include "symbolsinscoresstat.h"

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <iomanip>

#include "global/types/bytearray.h"
#include "global/io/buffer.h"
#include "global/io/dir.h"
#include "global/io/file.h"
#include "global/io/fileinfo.h"
#include "global/containers.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/rw/scorereader.h"
#include "engraving/libmscore/masterscore.h"
#include "engraving/libmscore/page.h"
#include "engraving/infrastructure/paint.h"
#include "engraving/infrastructure/localfileinfoprovider.h"

#include "importexport/guitarpro/internal/guitarproreader.h"

#include "fontface.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::draw;
using namespace mu::engraving;
using namespace mu::iex::guitarpro;

static const std::map<std::string, std::string> fonts = {
    { "MuseJazz Text", ":/fonts/musejazz/MuseJazzText.otf" },
    { "Leland Text", ":/fonts/leland/LelandText.otf" },
    { "Bravura Text", ":/fonts/bravura/BravuraText.otf" },
    { "MScore Text", ":/fonts/mscore/MScoreText.ttf" },
    { "Petaluma Text", ":/fonts/petaluma/PetalumaText.otf" },
    { "FinaleMaestro Text", ":/fonts/finalemaestro/FinaleMaestroText.otf" },
    { "FinaleBroadway Text", ":/fonts/finalebroadway/FinaleBroadwayText.otf" }
};

struct FontKey
{
    std::string family;
    Font::Type type = Font::Type::Undefined;

    bool operator==(const FontKey& k) const { return family == k.family && type == k.type; }
    bool operator!=(const FontKey& k) const { return !this->operator==(k); }

    bool operator<(const FontKey& k) const
    {
        if (family != k.family) {
            return family < k.family;
        }
        return type < k.type;
    }
};

struct Count {
    size_t count = 0;
};

using ByChar = std::map<char32_t, Count>;
using ByFamily = std::map<std::string, ByChar>;
using ByType = std::map<Font::Type, ByFamily>;

struct ST
{
    std::map<size_t, Count> sizes;
    std::set<mu::String> useLigatures;
};

struct SymbolsStat {
    ByType text;
    ByType symbols;
    ST symText;

    std::map<std::string, FontFace*> faces;

    FontFace* face(const std::string& fa)
    {
        auto it = faces.find(fa);
        if (it != faces.end()) {
            return it->second;
        }

        auto fit = fonts.find(fa);
        if (fit == fonts.end()) {
            return nullptr;
        }

        FontFace* f = new FontFace();
        f->load(QString::fromStdString(fit->second));
        faces[fa] = f;
        return f;
    }
};

class SymbolsStatPaintProvider : public draw::IPaintProvider
{
public:
    bool isActive() const { return true; }
    void beginTarget(const std::string& name) { UNUSED(name); }
    void beforeEndTargetHook(Painter* painter) { UNUSED(painter); }
    bool endTarget(bool endDraw = false) { UNUSED(endDraw); return true; }
    void beginObject(const std::string& name, const PointF& pagePos) { UNUSED(name); UNUSED(pagePos); }
    void endObject() {}

    void setAntialiasing(bool arg) { UNUSED(arg); }
    void setCompositionMode(CompositionMode mode) { UNUSED(mode); }

    void setFont(const Font& font) { m_font = font; }
    const Font& font() const { return m_font; }

    void setPen(const Pen& pen) { m_pen = pen; }
    void setNoPen() { m_pen = Pen(); }
    const Pen& pen() const { return m_pen; }

    void setBrush(const Brush& brush) { m_brush = brush; }
    const Brush& brush() const { return m_brush; }

    void save() {}
    void restore() {}

    void setTransform(const Transform& transform) { m_transform = transform; }
    const Transform& transform() const { return m_transform; }

    // drawing functions
    void drawPath(const PainterPath&) { }
    void drawPolygon(const PointF*, size_t, PolygonMode) { }

    void addText(const Font& f, const String& text)
    {
        ByFamily& byFamily = stat.text[f.type()];
        ByChar& byChar = byFamily[f.family().toStdString()];
        std::u32string str = text.toStdU32String();
        for (char32_t ch : str) {
            byChar[ch].count++;
        }

        if (f.type() == Font::Type::MusicSymbolText) {
            stat.symText.sizes[text.size()].count++;

            if (text.size() > 1) {
                std::u32string str = text.toStdU32String();
                FontFace* face = stat.face(f.family().toStdString());
                if (face) {
                    std::vector<glyph_idx_t> withLigatures = face->glyphs(&str[0], str.size(), true);
                    std::vector<glyph_idx_t> noLigatures = face->glyphs(&str[0], str.size(), false);

                    if (withLigatures != noLigatures) {
                        stat.symText.useLigatures.insert(text);
                    }
                }
            }
        }
    }

    void drawText(const PointF&, const String& text) { addText(m_font, text); }

    void drawText(const RectF&, int, const String& text) { addText(m_font, text); }
    void drawTextWorkaround(const Font& f, const PointF&, const String& text) { addText(f, text); }

    void drawSymbol(const PointF&, char32_t ucs4Code)
    {
        ByFamily& byFamily = stat.symbols[m_font.type()];
        ByChar& byChar = byFamily[m_font.family().toStdString()];
        byChar[ucs4Code].count++;
    }

    void drawPixmap(const PointF&, const Pixmap&) { }
    void drawTiledPixmap(const RectF&, const Pixmap&, const PointF&) { }

#ifndef NO_QT_SUPPORT
    void drawPixmap(const PointF&, const QPixmap&) { }
    void drawTiledPixmap(const RectF&, const QPixmap&, const PointF&) { }
#endif

    void setClipRect(const RectF&) { }
    void setClipping(bool) { }

    SymbolsStat stat;

private:
    Font m_font;
    Pen m_pen;
    Brush m_brush;
    Transform m_transform;
};

SymbolsInScoresStat::SymbolsInScoresStat()
{
    m_provider = std::make_shared<SymbolsStatPaintProvider>();
}

void SymbolsInScoresStat::processDir(const path_t& scoreDir)
{
    static const std::vector<mu::String> ignore = {
        u"7501.mscz", u"684646.mscz", u"4535461.mscz",
        // /home/igor/Dev/extasy-tests-resources-master/test_files/tabs
        u"228316.gp4", u"1424009.gp4", u"451637.gp4", u"230366.gp4", u"228383.gp4", u"234709.gp4",
        u"467207.gp4", u"1143420.gp4",
        // /home/igor/Dev/mu_private_stuff/scores/v3/
        u"_323_33117007_5731240.mscz", u"BIG_SHOT_-_Deltarune_Chapter_2_362_14726851_7020504.mscz",
        u"Mad_at_Disney_350_18242996_6358659.mscz", u"The_Wellerman_360_6695411_6613120.mscz"
    };

    RetVal<io::paths_t> scores = Dir::scanFiles(scoreDir, { "*.mscz", "*.gp", "*.gpx", "*.gp4", "*.gp5" });
    for (size_t i = 0; i < scores.val.size(); ++i) {
//        if (i < 3790) {
//            continue;
//        }

//        if (i > 100) {
//            break;
//        }

        std::cout << "processFile: " << (i + 1) << "/" << scores.val.size() << " " << scores.val.at(i).toStdString() << std::endl;

        if (mu::contains(ignore, FileInfo(scores.val.at(i)).fileName())) {
            LOGW() << "ignore: " << scores.val.at(i).toStdString();
            continue;
        }
        processFile(scores.val.at(i));
    }

    std::string str = statToString(m_provider->stat);
    std::cout << str;

    File file("symbols_in_scores.txt");
    file.open(IODevice::WriteOnly);
    file.write(reinterpret_cast<const uint8_t*>(&str[0]), str.size());
}

void SymbolsInScoresStat::processFile(const path_t& path)
{
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle();

    if (!loadScore(score, path)) {
        LOGE() << "failed load score: " << path;
        return;
    }

    score->doLayout();

    const int DEVICE_DPI = 300;
    MScore::pixelRatio = mu::engraving::DPI / DEVICE_DPI;

    const std::vector<Page*>& pages = score->pages();
    if (pages.empty()) {
        LOGE() << "no pages in score: " << path;
        return;
    }

    mu::draw::Painter painter(m_provider, "symbolstat");

    for (const Page* page : pages) {
        std::vector<EngravingItem*> elements = page->elements();
        engraving::Paint::paintElements(painter, elements, false);
    }

    delete score;
}

static std::string toString(Font::Type t)
{
    switch (t) {
    case Font::Type::Undefined: return "Undefined";
    case Font::Type::Unknown: return "Unknown";
    case Font::Type::Icon: return "Icon";
    case Font::Type::Text: return "Text";
    case Font::Type::MusicSymbolText: return "MusicSymbolText";
    case Font::Type::MusicSymbol: return "MusicSymbol";
    case Font::Type::Tablature: return "Tablature";
    case Font::Type::Harmony: return "Harmony";
    }
    return "";
}

std::string SymbolsInScoresStat::statToString(const SymbolsStat& stat)
{
    std::stringstream stream;

    auto toStream = [](std::stringstream& stream, const ByType& byType) {
        for (auto tit = byType.begin(); tit != byType.end(); ++tit) {
            Font::Type type = tit->first;
            const ByFamily& byFamily= tit->second;

            std::set<char32_t> chars;
            for (auto tcit = byFamily.begin(); tcit != byFamily.end(); ++tcit) {
                const ByChar& byChar = tcit->second;
                for (auto tccit = byChar.begin(); tccit != byChar.end(); ++tccit) {
                    chars.insert(tccit->first);
                }
            }

            stream << "\n";
            stream << "  Type: " << toString(type) << " | total: " << chars.size() << "\n";

            for (auto fit = byFamily.begin(); fit != byFamily.end(); ++fit) {
                const std::string& family = fit->first;
                const ByChar& byChar = fit->second;

                struct Ch
                {
                    char32_t ch = 0;
                    size_t count = 0;
                };

                std::list<Ch> chars;
                for (auto cit = byChar.begin(); cit != byChar.end(); ++cit) {
                    chars.push_back(Ch { cit->first, cit->second.count });
                }

                chars.sort([](const Ch& c1, const Ch& c2) {
                    return c1.count > c2.count;
                });

                stream << "    Family: " << family << " | total: " << chars.size() << "\n";
                for (const Ch& ch : chars) {
                    stream << "      " << std::setfill('0') << std::setw(4) << std::hex << ch.ch << ": "
                           << std::dec << ch.count << "\n";
                }
            }
        }
    };

    stream << "Text: \n";
    toStream(stream, stat.text);

    stream << "\n";
    stream << "Symbols: \n";
    toStream(stream, stat.symbols);

    stream << "\n";
    stream << "MusicSymbolText text sizes: \n";

    for (auto it = stat.symText.sizes.begin(); it != stat.symText.sizes.end(); ++it) {
        stream << "  size: " << it->first << ", count: " << it->second.count << "\n";
    }

    stream << "\n";
    stream << "  useLigatures: " << stat.symText.useLigatures.size() << "\n";
    for (const mu::String& str : stat.symText.useLigatures) {
        stream << "    ";
        for (size_t i = 0; i < str.size(); ++i) {
            stream << std::setfill('0') << std::setw(4) << std::hex << str.at(i).unicode() << ", ";
        }
        stream << "\n";
    }

    return stream.str();
}

bool SymbolsInScoresStat::loadScore(mu::engraving::MasterScore* score, const mu::io::path_t& path)
{
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));

    std::string suffix = io::suffix(path);
    if (suffix == "mscz") {
        // Load

        MscReader::Params params;
        params.filePath = path.toQString();
        params.mode = MscIoMode::Zip;

        MscReader reader(params);
        if (!reader.open()) {
            return false;
        }

        ScoreReader scoreReader;
        Err err = scoreReader.loadMscz(score, reader, true);
        if (err != Err::NoError) {
            LOGE() << "failed read file: " << path;
            return false;
        }
    } else {
        // Import

        GuitarProReader reader;
        Ret ret = reader.read(score, path);
        if (!ret) {
            LOGE() << "failed read file: " << path;
            return false;
        }
    }

    return true;
}
