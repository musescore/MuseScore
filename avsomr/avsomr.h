//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef AVS_AVSOMR_H
#define AVS_AVSOMR_H

#include <memory>
#include <QList>
#include <QHash>
#include <QSet>
#include <QRect>
#include <QImage>
#include <QByteArray>

namespace Ms {
namespace Avs {
class MsmrFile;
class AvsOmr
{
public:
    AvsOmr();
    using ID = uint32_t;
    using Num = uint16_t;
    using Idx = uint16_t;

    // Model
    struct Barline {
        ID id{ 0 };
        ID glyphID{ 0 };
    };

    struct Staff {
        ID id{ 0 };
        QList<ID> barlines;

        struct {
            int32_t start{ 0 };
            int32_t stop{ 0 };
            ID clefID{ 0 };
            ID keyID{ 0 };
            ID timeID{ 0 };
        } header;

        bool isValid() const { return id > 0; }
    };

    struct MStack {
        ID id{ 0 };
        Idx idx{ 0 };   // resolved
        int32_t left{ 0 };
        int32_t right{ 0 };
        bool isValid() const { return right > 0; }
    };

    struct Part {
        QList<Staff> staffs;
    };

    struct Inters {
        QHash<ID, Barline> barlines;
        QSet<ID> usedglyphs;
    };

    struct System {
        QList<MStack> mstacks;
        Part part;
        Inters inters;
        QSet<ID> freeglyphs;

        // resolved
        int32_t top{ 0 };
        int32_t bottom{ 0 };

        const MStack& stackByIdx(Idx idx, Idx* idxInSys = nullptr) const;
    };

    struct Page {
        QList<System> systems;
    };

    enum class GlyphUsed {
        Undefined = 0,
        Used,
        Free,
        Free_Covered,       // Avs marked as free, but fully covered used
        Trash               // Detected as trash (too small)
    };

    struct Glyph {
        ID id{ 0 };
        QRect bbox;
        GlyphUsed used{ GlyphUsed::Undefined };
        QImage img;
    };

    struct Sheet {
        Num num{ 0 };
        Page page;
        Idx mbeginIdx{ 0 };
        Idx mendIdx{ 0 };
        QHash<ID, Glyph*> glyphs;

        bool isGlyphUsed(const ID& glypthID) const;
        bool isGlyphFree(const ID& glypthID) const;
    };

    struct Book {
        uint16_t sheets{ 0 };
    };

    void resolve();

    // Configure
    struct Config {
        bool isHiddenAll() const { return !_isShowRecognized && !_isShowNotRecognized; }
        void setIsShowRecognized(bool arg) { _isShowRecognized = arg; }
        bool isShowRecognized() const { return _isShowRecognized; }
        void setIsShowNotRecognized(bool arg) { _isShowNotRecognized = arg; }
        bool isShowNotRecognized() const { return _isShowNotRecognized; }

    private:
        bool _isShowRecognized{ true };
        bool _isShowNotRecognized{ true };
    };

    Config& config();
    const Config& config() const;

    // Access
    struct MMetrics {
        QRect bbox;      //! NOTE bbox of measure
        QRect ebbox;     //! NOTE bbox in which there may be elements belonging to measure
        QRect hbbox;     //! NOTE bbox of measure header (clef, key, time)

        QRect headerBBox() const
        {
            return hbbox;
        }

        QRect chordBBox() const
        {
            QRect c = bbox;
            c.setLeft(c.left() + hbbox.width());
            return c;
        }
    };

    Num sheetNumByMeausereIdx(const Idx& meausureIdx) const;
    MMetrics mmetrics(const Num& sheetNum, const Idx& meausureIdx) const;
    QList<const Glyph*> glyphsByBBox(const Num& sheetNum, const QRect& bbox, QList<AvsOmr::GlyphUsed>& accepted) const;

    // Data
    void setMsmrFile(std::shared_ptr<MsmrFile> file);   // keep data for saving
    std::shared_ptr<MsmrFile> msmrFile() const;

    // Info
    struct Info {
        QColor usedColor;
        uint32_t usedCount{ 0 };
        QColor freeColor;
        uint32_t freeCount{ 0 };
    };

    const Info& info() const;

private:
    friend class AvsOmrReader;

    const Sheet* sheet(const Num& sheetNum) const;

    Book _book;
    QList<Sheet*> _sheets;
    Config _config;
    std::shared_ptr<MsmrFile> _msmrFile;
    Info _info;
};
} // Avs
} // Ms

#endif // AVS_AVSOMR_H
