#ifndef GP67DOMFIXER_HPP
#define GP67DOMFIXER_HPP

#include "gpdommodel.h"

namespace Ms {
//! NOTE Раннии версии файлов GP6 отличаются от текущих версий GP6
//! Например в ранних версиях не записывалась лирика разбитая по битам
//! - нужно самостоятельно её парсить и разбивать по битам
//! Не записывалась информация о диаграммах (аккордах)
//! Может чтото ещё обнаружится, этот класс для того,
//! чтобы привести старые дом модели GP6 к текущей

class GP67DomFixer
{
public:
    GP67DomFixer();

    static void fixGPDomModel(GPDomModel* gpDom);

private:

    static bool isLyricsOnBeats(const GPDomModel* gpDom);
    static void breakLyricsOnBeats(GPDomModel* gpDom);

    static bool isHasDiagrams(const GPDomModel* gpDom);
    static void createDiagrams(GPDomModel* gpDom);
    static std::string chordNameForBeatWithStrings(GPBeat* beat, const std::vector<GPTrack::String>& strings);
};
}

#endif // GP67DOMFIXER_HPP
