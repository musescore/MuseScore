#ifndef MU_IMPORTEXPORT_INOTEPROPERTY_H
#define MU_IMPORTEXPORT_INOTEPROPERTY_H

namespace mu::iex::guitarpro {
class INoteProperty
{
public:
    INoteProperty() = default;
    virtual ~INoteProperty() = default;
};

class BendProperty : public INoteProperty
{
public:

    void setDestinationOffset(float v) { _destinationOffset = v; }
    void setDestinationValue(float v) { _destinationValue = v; }
    void setMiddleOffset1(float v) { _middleOffset1 = v; }
    void setMiddleOffset2(float v) { _middleOffset2 = v; }
    void setMiddleValue(float v) { _middleValue = v; }
    void setOriginOffset(float v) { _originOffset = v; }
    void setOriginValue(float v) { _originValue = v; }

    float destinationOffset() const { return _destinationOffset; }
    float destinationValue() const { return _destinationValue; }
    float middleOffset1() const { return _middleOffset1; }
    float middleOffset2() const { return _middleOffset2; }
    float middleValue() const { return _middleValue; }
    float originOffset() const { return _originOffset; }
    float originValue() const { return _originValue; }

private:
    float _destinationOffset{ -1 };
    float _destinationValue{ -1 };
    float _middleOffset1{ -1 };
    float _middleOffset2{ -1 };
    float _middleValue{ -1 };
    float _originOffset{ -1 };
    float _originValue{ -1 };
};
} // namespace mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_INOTEPROPERTY_H
