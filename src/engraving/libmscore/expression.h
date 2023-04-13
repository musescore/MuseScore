#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "textbase.h"

namespace mu::engraving {
class Dynamic;

class Expression final : public TextBase
{
    M_PROPERTY(bool, snapToDynamics, setSnapToDynamics)
private:
    Dynamic* _snappedDynamic = nullptr;
public:
    Expression(Segment* parent);
    Expression(const Expression& expression);
    Expression* clone() const override { return new Expression(*this); }

    Segment* segment() const { return toSegment(explicitParent()); }

    PropertyValue propertyDefault(Pid id) const override;

    void layout() override;
    double computeDynamicExpressionDistance() const;

    std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const EngravingItem*)> isDragged) override;

    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;

    bool acceptDrop(EditData& ed) const override;
    EngravingItem* drop(EditData& ed) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    void mapPropertiesFromOldExpressions(StaffText* staffText);

    Dynamic* snappedDynamic() const { return _snappedDynamic; }
};
} // namespace mu::engraving
#endif // EXPRESSION_H
