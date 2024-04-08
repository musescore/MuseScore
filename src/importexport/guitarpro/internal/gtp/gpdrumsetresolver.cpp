#include "gpdrumsetresolver.h"

#include "log.h"

namespace mu::iex::guitarpro {
void GPDrumSetResolver::initGPDrum()
{
    addDrum(GPDrum(35, 0, 0));
    addDrum(GPDrum(36, 0, 0));
    addDrum(GPDrum(36, 6, 1));
    addDrum(GPDrum(37, 1, 2));
    addDrum(GPDrum(38, 1, 0));
    addDrum(GPDrum(42, 10, 0));
    addDrum(GPDrum(43, 5, 0));
    addDrum(GPDrum(44, 11, 0));
    addDrum(GPDrum(45, 6, 0));
    addDrum(GPDrum(46, 10, 2));
    addDrum(GPDrum(47, 7, 0));
    addDrum(GPDrum(48, 8, 0));
    addDrum(GPDrum(49, 13, 0));
    addDrum(GPDrum(50, 9, 0));
    addDrum(GPDrum(51, 15, 0));
    addDrum(GPDrum(52, 16, 0));
    addDrum(GPDrum(53, 15, 2));
    addDrum(GPDrum(55, 14, 0));
    addDrum(GPDrum(56, 3, 0));
    addDrum(GPDrum(57, 12, 0));
    addDrum(GPDrum(91, 1, 1));
    addDrum(GPDrum(92, 10, 1));
    addDrum(GPDrum(93, 15, 1));
    addDrum(GPDrum(99, 2, 0));
    addDrum(GPDrum(102, 4, 0));

    addDrum(GPDrum(0, 0, 0,  u"rvs-cymb"));

    addDrum(GPDrum(37, 0, 0,  u"snr"));
    addDrum(GPDrum(38, 1, 0,  u"snr"));

    addDrum(GPDrum(39, 0, 0,  u"hclap"));

    addDrum(GPDrum(87, 2, 1,  u"latinoKit"));
    addDrum(GPDrum(54, 3, 0,  u"latinoKit"));
    addDrum(GPDrum(54, 3, 1,  u"latinoKit"));
    addDrum(GPDrum(112, 3, 3, u"latinoKit"));

    addDrum(GPDrum(58, 0, 0,  u"vbrslp"));

    addDrum(GPDrum(67, 0, 0,  u"agogoKit"));
    addDrum(GPDrum(68, 1, 0,  u"agogoKit"));

    addDrum(GPDrum(63, 4, 0,  u"africaKit"));
    addDrum(GPDrum(73, 2, 0,  u"africaKit"));
    addDrum(GPDrum(74, 2, 2,  u"africaKit"));
    addDrum(GPDrum(109, 3, 2, u"africaKit"));
    addDrum(GPDrum(110, 4, 1, u"africaKit"));

    addDrum(GPDrum(62, 1, 2,  u"cngKit"));
    addDrum(GPDrum(63, 1, 0,  u"cngKit"));
    addDrum(GPDrum(64, 0, 0,  u"cngKit"));

    addDrum(GPDrum(65, 0, 0,  u"snthdrm"));

    addDrum(GPDrum(65, 1, 0,  u"tmblKit"));
    addDrum(GPDrum(66, 0, 0,  u"tmblKit"));

    addDrum(GPDrum(65, 0, 0,  u"mldctm"));

    addDrum(GPDrum(66, 0, 0,  u"taiko"));

    addDrum(GPDrum(69, 0, 0,  u"cbs"));
    addDrum(GPDrum(69, 0, 1,  u"cbs"));
    addDrum(GPDrum(117, 0, 2, u"cbs"));

    addDrum(GPDrum(70, 0, 0,  u"mrcs"));

    addDrum(GPDrum(71, 0, 0,  u"whstlKit"));
    addDrum(GPDrum(72, 1, 0,  u"whstlKit"));

    addDrum(GPDrum(73, 0, 0,  u"guiro"));
    addDrum(GPDrum(73, 0, 1,  u"guiro"));
    addDrum(GPDrum(74, 0, 2,  u"guiro"));

    addDrum(GPDrum(75, 0, 0,  u"clvs"));
    addDrum(GPDrum(75, 5, 0,  u"clvs"));

    addDrum(GPDrum(76, 0, 0,  u"wdblckKit"));
    addDrum(GPDrum(77, 1, 0,  u"wdblckKit"));

    addDrum(GPDrum(78, 0, 0,  u"cuicaKit"));
    addDrum(GPDrum(79, 1, 0,  u"cuicaKit"));

    addDrum(GPDrum(80, 0, 1,  u"trngl"));
    addDrum(GPDrum(81, 0, 0,  u"trngl"));

    addDrum(GPDrum(82, 0, 0,  u"shkr"));
    addDrum(GPDrum(82, 0, 1,  u"shkr"));

    addDrum(GPDrum(83, 0, 0,  u"jngl-bell"));

    addDrum(GPDrum(84, 0, 0,  u"bell-tree"));
    addDrum(GPDrum(123, 0, 2, u"bell-tree"));

    addDrum(GPDrum(85, 0, 0,  u"cstnt"));

    addDrum(GPDrum(86, 0, 1,  u"surdo"));
    addDrum(GPDrum(86, 0, 0,  u"surdo"));

    addDrum(GPDrum(102, 0, 0,  u"cowbell"));

    addDrum(GPDrum(51, 0, 0,  u"sambaKit"));
    addDrum(GPDrum(93, 0, 1,  u"sambaKit"));
    addDrum(GPDrum(53, 0, 2,  u"sambaKit"));
    addDrum(GPDrum(102, 1, 0, u"sambaKit"));
    addDrum(GPDrum(56, 2, 0,  u"sambaKit"));
    addDrum(GPDrum(99, 3, 0,  u"sambaKit"));
    addDrum(GPDrum(99, 3, 0,  u"sambaKit"));
    addDrum(GPDrum(77, 4, 0,  u"sambaKit"));
    addDrum(GPDrum(76, 5, 0,  u"sambaKit"));
    addDrum(GPDrum(66, 6, 0,  u"sambaKit"));
    addDrum(GPDrum(65, 7, 0,  u"sambaKit"));

    addDrum(GPDrum(61, 0, 0,   u"bngKit"));
    addDrum(GPDrum(106, 0, 2,  u"bngKit"));
    addDrum(GPDrum(107, 0, 1,  u"bngKit"));
    addDrum(GPDrum(60, 1, 0,   u"bngKit"));
    addDrum(GPDrum(104, 1, 2,  u"bngKit"));
    addDrum(GPDrum(105, 1, 1,  u"bngKit"));
    addDrum(GPDrum(108, 2, 1,  u"bngKit"));
    addDrum(GPDrum(64, 2, 0,   u"bngKit"));
    addDrum(GPDrum(109, 2, 2,  u"bngKit"));
    addDrum(GPDrum(63, 3, 0,   u"bngKit"));
    addDrum(GPDrum(110, 3, 1,  u"bngKit"));
    addDrum(GPDrum(62, 3, 2,   u"bngKit"));

    addDrum(GPDrum(61, 0, 0,   u"conbon"));
    addDrum(GPDrum(106, 0, 2,  u"conbon"));
    addDrum(GPDrum(107, 0, 1,  u"conbon"));
    addDrum(GPDrum(60, 1, 0,   u"conbon"));
    addDrum(GPDrum(104, 1, 2,  u"conbon"));
    addDrum(GPDrum(105, 1, 1,  u"conbon"));
    addDrum(GPDrum(108, 2, 1,  u"conbon"));
    addDrum(GPDrum(64, 2, 0,   u"conbon"));
    addDrum(GPDrum(109, 2, 2,  u"conbon"));
    addDrum(GPDrum(63, 3, 0,   u"conbon"));
    addDrum(GPDrum(110, 3, 1,  u"conbon"));
    addDrum(GPDrum(62, 3, 2,   u"conbon"));

    addDrum(GPDrum(108, 0, 1,  u"cngKit"));
    addDrum(GPDrum(109, 0, 2,  u"cngKit"));
    addDrum(GPDrum(110, 1, 1,  u"cngKit"));
    addDrum(GPDrum(111, 0, 2,  u"tmbrn"));
    addDrum(GPDrum(112, 0, 3,  u"tmbrn"));

    addDrum(GPDrum(113, 0, 4,  u"tmbrn"));
    addDrum(GPDrum(54, 0, 0,   u"tmbrn"));

    addDrum(GPDrum(114, 0, 0,  u"ptt"));
    addDrum(GPDrum(115, 0, 0,  u"grcss"));
    addDrum(GPDrum(118, 0, 2,  u"mrcs"));

    addDrum(GPDrum(119, 0, 0,  u"2Mrcs"));
    addDrum(GPDrum(122, 0, 2,  u"shkr"));
}

int32_t GPDrumSetResolver::pitch(int32_t element,
                                 int32_t variation,
                                 const muse::String& name) const
{
    if (_drum.find(name) == _drum.end()) {
        return 0;
    }

    for (const auto& drum : _drum.at(name)) {
        if (drum._name.contains(name)) {
            if (drum.element == element && drum.variation == variation) {
                return drum.pitch;
            }
        }
    }

    LOGD() << "Absent drum " << "element: " << element << " variation: " << variation << " name: " << name << " key: " << 0;

    return 0;
}

void GPDrumSetResolver::addDrum(const GPDrum& drum)
{
    _drum[drum._name].push_back(drum);
}
} // namespace mu::iex::guitarpro
