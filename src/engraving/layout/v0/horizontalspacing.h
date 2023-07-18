#ifndef MU_ENGRAVING_HORIZONTALSPACINGUTILS_H
#define MU_ENGRAVING_HORIZONTALSPACINGUTILS_H

namespace mu::engraving {
class Chord;
class EngravingItem;
class Note;
class Rest;
class Shape;
class StemSlash;
enum class ElementType;
enum class KerningType;
}

namespace mu::engraving::layout::v0 {
class HorizontalSpacing
{
public:
    static double computePadding(const EngravingItem* item1, const EngravingItem* item2);
    static KerningType computeKerning(const EngravingItem* item1, const EngravingItem* item2);

private:
    static bool isSpecialNotePaddingType(ElementType type);
    static void computeNotePadding(const Note* note, const EngravingItem* item2, double& padding, double scaling);
    static void computeLedgerRestPadding(const Rest* rest2, double& padding);

    static bool isSameVoiceKerningLimited(const EngravingItem* item);
    static bool isNeverKernable(const EngravingItem* item);
    static bool isAlwaysKernable(const EngravingItem* item);

    static KerningType doComputeKerningType(const EngravingItem* item1, const EngravingItem* item2);
    static KerningType computeNoteKerningType(const Note* note, const EngravingItem* item2);
    static KerningType computeStemSlashKerningType(const StemSlash* stemSlash, const EngravingItem* item2);
};
} // namespace mu::engraving::layout
#endif // MU_ENGRAVING_HORIZONTALSPACINGUTILS_H
