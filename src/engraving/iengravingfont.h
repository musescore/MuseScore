#ifndef MU_ENGRAVING_IENGRAVINGFONT_H
#define MU_ENGRAVING_IENGRAVINGFONT_H

#include <memory>

#include "global/types/string.h"
#include "draw/types/geometry.h"

#include "types/symid.h"
#include "style/styledef.h"

namespace muse::draw {
class Painter;
}

namespace mu::engraving {
class Shape;
class EngravingItem;

class IEngravingFont
{
public:

    virtual ~IEngravingFont() = default;

    virtual const std::string& name() const = 0;
    virtual const std::string& family() const = 0;

    // Misc
    virtual bool isValid(SymId id) const = 0;
    virtual char32_t symCode(SymId id) const = 0;
    virtual SymId fromCode(char32_t code) const = 0;
    virtual String toString(SymId id) const = 0;

    virtual std::unordered_map<Sid, PropertyValue> engravingDefaults() const = 0;

    // Metrics
    virtual double width(SymId id, double mag) const = 0;
    virtual double width(const SymIdList&, double mag) const = 0;
    virtual double height(SymId id, double mag) const = 0;
    virtual double advance(SymId id, double mag) const = 0;

    virtual RectF bbox(SymId id, double mag) const = 0;
    virtual RectF bbox(SymId id, const SizeF&) const = 0;
    virtual RectF bbox(const SymIdList& sl, double mag) const = 0;
    virtual RectF bbox(const SymIdList& s, const SizeF& mag) const = 0;
    virtual Shape shape(const SymIdList& s, double mag) const = 0;
    virtual Shape shape(const SymIdList& s, const SizeF& mag) const = 0;
    virtual Shape shapeWithCutouts(SymId id, double mag) = 0;
    virtual Shape shapeWithCutouts(SymId id, const SizeF& mag) = 0;

    virtual PointF smuflAnchor(SymId symId, SmuflAnchorId anchorId, double mag) const = 0;

    // Draw
    virtual void draw(SymId id, muse::draw::Painter* p, double mag, const PointF& pos, const double angle = 0) const = 0;
    virtual void draw(SymId id, muse::draw::Painter* p, const SizeF& mag, const PointF& pos, const double angle = 0) const = 0;
    virtual void draw(const SymIdList& ids, muse::draw::Painter* p, double mag, const PointF& pos, const double angle = 0) const = 0;
    virtual void draw(const SymIdList& ids, muse::draw::Painter* p, const SizeF& mag, const PointF& pos, const double angle = 0) const = 0;
};

using IEngravingFontPtr = std::shared_ptr<IEngravingFont>;
}

#endif // MU_ENGRAVING_IENGRAVINGFONT_H
