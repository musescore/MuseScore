/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <memory>
#include <cassert>

#ifndef NO_QT_SUPPORT
#include <QVariant>
#endif

#include "global/types/string.h"
#include "global/logstream.h"

#include "../types/types.h"
#include "../types/symid.h"

namespace mu::engraving {
class Groups;
class TDuration;

enum class P_TYPE : unsigned char {
    UNDEFINED = 0,
    // Base
    BOOL,
    INT,
    INT_VEC, // std::vector<int>
    SIZE_T,  // size_t
    REAL,
    STRING,

    // Geometry
    POINT,              // point units, value saved as mm or spatium depending on EngravingItem->sizeIsSpatiumDependent()
    SIZE,
    DRAW_PATH,
    SCALE,
    SPATIUM,
    MILLIMETRE,
    PAIR_REAL,

    // Draw
    SYMID,
    COLOR,
    ORNAMENT_STYLE,
    ORNAMENT_INTERVAL,
    ORNAMENT_SHOW_ACCIDENTAL,
    GLISS_STYLE,
    GLISS_TYPE,

    // Layout
    ALIGN,
    PLACEMENT_V,
    PLACEMENT_H,
    TEXT_PLACE,
    DIRECTION_V,
    DIRECTION_H,
    ORIENTATION,
    BEAM_MODE,
    ACCIDENTAL_ROLE,
    TIE_PLACEMENT,
    TIE_DOTS_PLACEMENT,

    TIMESIG_PLACEMENT,
    TIMESIG_STYLE,
    TIMESIG_MARGIN,

    // Sound
    FRACTION,
    DURATION_TYPE_WITH_DOTS,
    CHANGE_METHOD,
    PITCH_VALUES,
    TEMPO,

    // Types
    LAYOUTBREAK_TYPE,
    VELO_TYPE,
    BARLINE_TYPE,
    NOTEHEAD_TYPE,
    NOTEHEAD_SCHEME,
    NOTEHEAD_GROUP,
    CLEF_TYPE,
    CLEF_TO_BARLINE_POS,
    DYNAMIC_TYPE,
    DYNAMIC_SPEED,
    LINE_TYPE,
    HOOK_TYPE,
    KEY_MODE,
    TEXT_STYLE,
    PLAYTECH_TYPE,
    TEMPOCHANGE_TYPE,
    SLUR_STYLE_TYPE,
    NOTELINE_PLACEMENT_TYPE,
    LYRICS_DASH_SYSTEM_START_TYPE,
    PARTIAL_SPANNER_DIRECTION,

    VOICE_ASSIGNMENT,
    AUTO_ON_OFF,

    // Other
    GROUPS,
};

class PropertyValue
{
public:
    PropertyValue() = default;

    // Base
    PropertyValue(bool v)
        : m_type(P_TYPE::BOOL), m_data(make_data<bool>(v)) {}

    PropertyValue(int v)
        : m_type(P_TYPE::INT), m_data(make_data<int>(v)) {}

    PropertyValue(const std::vector<int>& v)
        : m_type(P_TYPE::INT_VEC), m_data(make_data<std::vector<int> >(v)) {}

    PropertyValue(size_t v)
        : m_type(P_TYPE::SIZE_T), m_data(make_data<size_t>(v)) {}

    PropertyValue(double v)
        : m_type(P_TYPE::REAL), m_data(make_data<double>(v)) {}

    PropertyValue(const char* v)
        : m_type(P_TYPE::STRING), m_data(make_data<String>(String::fromUtf8(v))) {}

    PropertyValue(const String& v)
        : m_type(P_TYPE::STRING), m_data(make_data<String>(v)) {}

#ifndef NO_QT_SUPPORT
    PropertyValue(const QString& v)
        : m_type(P_TYPE::STRING), m_data(make_data<String>(String::fromQString(v))) {}
#endif

    // Geometry
    PropertyValue(const PointF& v)
        : m_type(P_TYPE::POINT), m_data(make_data<PointF>(v)) {}

    PropertyValue(const PairF& v)
        : m_type(P_TYPE::PAIR_REAL), m_data(make_data<PairF>(v)) {}

    PropertyValue(const SizeF& v)
        : m_type(P_TYPE::SIZE), m_data(make_data<SizeF>(v)) {}

    PropertyValue(const PainterPath& v)
        : m_type(P_TYPE::DRAW_PATH), m_data(make_data<PainterPath>(v)) {}

    PropertyValue(const ScaleF& v)
        : m_type(P_TYPE::SCALE), m_data(make_data<ScaleF>(v)) {}

    PropertyValue(const Spatium& v)
        : m_type(P_TYPE::SPATIUM), m_data(make_data<Spatium>(v)) {}

    PropertyValue(const Millimetre& v)
        : m_type(P_TYPE::MILLIMETRE), m_data(make_data<Millimetre>(v)) {}

    // Draw
    PropertyValue(SymId v)
        : m_type(P_TYPE::SYMID), m_data(make_data<SymId>(v)) {}

    PropertyValue(const Color& v)
        : m_type(P_TYPE::COLOR), m_data(make_data<Color>(v)) {}

    PropertyValue(OrnamentStyle v)
        : m_type(P_TYPE::ORNAMENT_STYLE), m_data(make_data<OrnamentStyle>(v)) {}

    PropertyValue(GlissandoStyle v)
        : m_type(P_TYPE::GLISS_STYLE), m_data(make_data<GlissandoStyle>(v)) {}

    PropertyValue(GlissandoType v)
        : m_type(P_TYPE::GLISS_TYPE), m_data(make_data<GlissandoType>(v)) {}

    // Layout
    PropertyValue(Align v)
        : m_type(P_TYPE::ALIGN), m_data(make_data<Align>(v)) {}

    PropertyValue(PlacementV v)
        : m_type(P_TYPE::PLACEMENT_V), m_data(make_data<PlacementV>(v)) {}
    PropertyValue(PlacementH v)
        : m_type(P_TYPE::PLACEMENT_H), m_data(make_data<PlacementH>(v)) {}

    PropertyValue(TextPlace v)
        : m_type(P_TYPE::TEXT_PLACE), m_data(make_data<TextPlace>(v)) {}

    PropertyValue(DirectionV v)
        : m_type(P_TYPE::DIRECTION_V), m_data(make_data<DirectionV>(v)) {}
    PropertyValue(DirectionH v)
        : m_type(P_TYPE::DIRECTION_H), m_data(make_data<DirectionH>(v)) {}

    PropertyValue(Orientation v)
        : m_type(P_TYPE::ORIENTATION), m_data(make_data<Orientation>(v)) {}

    PropertyValue(BeamMode v)
        : m_type(P_TYPE::BEAM_MODE), m_data(make_data<BeamMode>(v)) {}

    PropertyValue(const AccidentalRole& v)
        : m_type(P_TYPE::ACCIDENTAL_ROLE), m_data(make_data<AccidentalRole>(v)) {}

    PropertyValue(TiePlacement v)
        : m_type(P_TYPE::TIE_PLACEMENT), m_data(make_data<TiePlacement>(v)) {}

    PropertyValue(TieDotsPlacement v)
        : m_type(P_TYPE::TIE_DOTS_PLACEMENT), m_data(make_data<TieDotsPlacement>(v)) {}

    PropertyValue(TimeSigPlacement v)
        : m_type(P_TYPE::TIMESIG_PLACEMENT), m_data(make_data<TimeSigPlacement>(v)) {}

    PropertyValue(TimeSigStyle v)
        : m_type(P_TYPE::TIMESIG_STYLE), m_data(make_data<TimeSigStyle>(v)) {}

    PropertyValue(TimeSigVSMargin v)
        : m_type(P_TYPE::TIMESIG_MARGIN), m_data(make_data<TimeSigVSMargin>(v)) {}

    // Sound
    PropertyValue(const Fraction& v)
        : m_type(P_TYPE::FRACTION), m_data(make_data<Fraction>(v)) {}
    PropertyValue(const DurationTypeWithDots& v)
        : m_type(P_TYPE::DURATION_TYPE_WITH_DOTS), m_data(make_data<DurationTypeWithDots>(v)) {}
    PropertyValue(ChangeMethod v)
        : m_type(P_TYPE::CHANGE_METHOD), m_data(make_data<ChangeMethod>(v)) {}
    PropertyValue(const PitchValues& v)
        : m_type(P_TYPE::PITCH_VALUES), m_data(make_data<PitchValues>(v)) {}
    PropertyValue(const BeatsPerSecond& v)
        : m_type(P_TYPE::TEMPO), m_data(make_data<BeatsPerSecond>(v)) {}

    // Types
    PropertyValue(LayoutBreakType v)
        : m_type(P_TYPE::LAYOUTBREAK_TYPE), m_data(make_data<LayoutBreakType>(v)) {}

    PropertyValue(VeloType v)
        : m_type(P_TYPE::VELO_TYPE), m_data(make_data<VeloType>(v)) {}

    PropertyValue(BarLineType v)
        : m_type(P_TYPE::BARLINE_TYPE), m_data(make_data<BarLineType>(v)) {}

    PropertyValue(NoteHeadType v)
        : m_type(P_TYPE::NOTEHEAD_TYPE), m_data(make_data<NoteHeadType>(v)) {}
    PropertyValue(NoteHeadScheme v)
        : m_type(P_TYPE::NOTEHEAD_SCHEME), m_data(make_data<NoteHeadScheme>(v)) {}
    PropertyValue(NoteHeadGroup v)
        : m_type(P_TYPE::NOTEHEAD_GROUP), m_data(make_data<NoteHeadGroup>(v)) {}

    PropertyValue(ClefType v)
        : m_type(P_TYPE::CLEF_TYPE), m_data(make_data<ClefType>(v)) {}

    PropertyValue(ClefToBarlinePosition v)
        : m_type(P_TYPE::CLEF_TO_BARLINE_POS), m_data(make_data<ClefToBarlinePosition>(v)) {}

    PropertyValue(DynamicType v)
        : m_type(P_TYPE::DYNAMIC_TYPE), m_data(make_data<DynamicType>(v)) {}
    PropertyValue(DynamicSpeed v)
        : m_type(P_TYPE::DYNAMIC_SPEED), m_data(make_data<DynamicSpeed>(v)) {}

    PropertyValue(LineType v)
        : m_type(P_TYPE::LINE_TYPE), m_data(make_data<LineType>(v)) {}
    PropertyValue(HookType v)
        : m_type(P_TYPE::HOOK_TYPE), m_data(make_data<HookType>(v)) {}

    PropertyValue(KeyMode v)
        : m_type(P_TYPE::KEY_MODE), m_data(make_data<KeyMode>(v)) {}

    PropertyValue(TextStyleType v)
        : m_type(P_TYPE::TEXT_STYLE), m_data(make_data<TextStyleType>(v)) {}

    PropertyValue(PlayingTechniqueType v)
        : m_type(P_TYPE::PLAYTECH_TYPE), m_data(make_data<PlayingTechniqueType>(v)) {}

    PropertyValue(GradualTempoChangeType v)
        : m_type(P_TYPE::TEMPOCHANGE_TYPE), m_data(make_data<GradualTempoChangeType>(v)) {}

    PropertyValue(SlurStyleType v)
        : m_type(P_TYPE::SLUR_STYLE_TYPE), m_data(make_data<SlurStyleType>(v)) {}

    PropertyValue(const NoteLineEndPlacement& v)
        : m_type(P_TYPE::NOTELINE_PLACEMENT_TYPE), m_data(make_data<NoteLineEndPlacement>(v)) {}

    // Other
    PropertyValue(const GroupNodes& v)
        : m_type(P_TYPE::GROUPS), m_data(make_data<GroupNodes>(v)) {}

    PropertyValue(const OrnamentInterval& v)
        : m_type(P_TYPE::ORNAMENT_INTERVAL), m_data(make_data<OrnamentInterval>(v)) {}

    PropertyValue(const OrnamentShowAccidental& v)
        : m_type(P_TYPE::ORNAMENT_SHOW_ACCIDENTAL), m_data(make_data<OrnamentShowAccidental>(v)) {}

    PropertyValue(const LyricsDashSystemStart& v)
        : m_type(P_TYPE::LYRICS_DASH_SYSTEM_START_TYPE), m_data(make_data<LyricsDashSystemStart>(v)) {}

    PropertyValue(const PartialSpannerDirection& v)
        : m_type(P_TYPE::PARTIAL_SPANNER_DIRECTION), m_data(make_data<PartialSpannerDirection>(v)) {}

    PropertyValue(const VoiceAssignment& v)
        : m_type(P_TYPE::VOICE_ASSIGNMENT), m_data(make_data<VoiceAssignment>(v)) {}

    PropertyValue(const AutoOnOff& v)
        : m_type(P_TYPE::AUTO_ON_OFF), m_data(make_data<AutoOnOff>(v)) {}

    bool isValid() const;

    P_TYPE type() const;
    bool isEnum() const { return m_data ? m_data->isEnum() : false; }

    template<typename T>
    T value() const
    {
        if (m_type == P_TYPE::UNDEFINED) {
            return T();
        }

        assert(m_data);
        if (!m_data) {
            return T();
        }

        Arg<T>* at = get<T>();
        if (!at) {
            //! HACK Temporary hack for int to enum
            if constexpr (std::is_enum<T>::value) {
                if (P_TYPE::INT == m_type) {
                    return static_cast<T>(value<int>());
                }
            }

            //! HACK Temporary hack for enum to int
            if constexpr (std::is_same<T, int>::value) {
                if (m_data->isEnum()) {
                    return m_data->enumToInt();
                }
            }

            //! HACK Temporary hack for bool to int
            if constexpr (std::is_same<T, int>::value) {
                if (P_TYPE::BOOL == m_type) {
                    return value<bool>();
                }
            }

            //! HACK Temporary hack for int to bool
            if constexpr (std::is_same<T, bool>::value) {
                return value<int>();
            }

            //! HACK Temporary hack for int to size_t
            if constexpr (std::is_same<T, int>::value) {
                if (P_TYPE::SIZE_T == m_type) {
                    return static_cast<int>(value<size_t>());
                }
            }

            //! HACK Temporary hack for real to Spatium
            if constexpr (std::is_same<T, Spatium>::value) {
                if (P_TYPE::REAL == m_type) {
                    Arg<double>* srv = get<double>();
                    assert(srv);
                    return srv ? Spatium(srv->v) : Spatium();
                }
            }

            //! HACK Temporary hack for Spatium to real
            if constexpr (std::is_same<T, double>::value) {
                if (P_TYPE::SPATIUM == m_type) {
                    return value<Spatium>().val();
                }
            }

            //! HACK Temporary hack for real to Millimetre
            if constexpr (std::is_same<T, Millimetre>::value) {
                if (P_TYPE::REAL == m_type) {
                    Arg<double>* mrv = get<double>();
                    assert(mrv);
                    return mrv ? Millimetre(mrv->v) : Millimetre();
                }
            }

            //! HACK Temporary hack for Spatium to real
            if constexpr (std::is_same<T, double>::value) {
                if (P_TYPE::MILLIMETRE == m_type) {
                    return value<Millimetre>().val();
                }
            }

            if constexpr (std::is_same<T, String>::value) {
                //! HACK Temporary hack for Fraction to String
                if (P_TYPE::FRACTION == m_type) {
                    return value<Fraction>().toString();
                }
            }

#ifndef NO_QT_SUPPORT
            if constexpr (std::is_same<T, QString>::value) {
                //! HACK Temporary hack for Fraction to String
                if (P_TYPE::FRACTION == m_type) {
                    return value<Fraction>().toString();
                }

                //! HACK Temporary hack for String to QString
                if (P_TYPE::STRING == m_type) {
                    return value<String>().toQString();
                }
            }
#endif
        }

        assert(at);
        if (!at) {
            return T();
        }
        return at->v;
    }

    bool toBool() const { return value<bool>(); }
    int toInt() const { return value<int>(); }
    double toReal() const { return value<double>(); }
    double toDouble() const { return value<double>(); }

    bool operator ==(const PropertyValue& v) const;
    inline bool operator !=(const PropertyValue& v) const { return !this->operator ==(v); }

    template<typename T>
    static PropertyValue fromValue(const T& v) { return PropertyValue(v); }

#ifndef NO_QT_SUPPORT
    //! NOTE compat
    QVariant toQVariant() const;
    static PropertyValue fromQVariant(const QVariant& v, P_TYPE type);
#endif

private:
    struct IArg {
        virtual ~IArg() = default;

        virtual bool equal(const IArg* a) const = 0;

        virtual bool isEnum() const = 0;
        virtual int enumToInt() const = 0;
    };

    template<typename T>
    struct Arg : public IArg {
        T v;
        Arg(const T& v)
            : IArg(), v(v) {}

        bool equal(const IArg* a) const override
        {
            assert(a);
            const Arg<T>* at = dynamic_cast<const Arg<T>*>(a);
            assert(at);
            return at ? at->v == v : false;
        }

        //! HACK Temporary hack for enum to int
        bool isEnum() const override
        {
            return std::is_enum<T>::value;
        }

        int enumToInt() const override
        {
            if constexpr (std::is_enum<T>::value) {
                return static_cast<int>(v);
            } else {
                return -1;
            }
        }
    };

    template<typename T>
    inline std::shared_ptr<IArg> make_data(const T& v) const
    {
        return std::shared_ptr<IArg>(new Arg<T>(v));
    }

    template<typename T>
    inline Arg<T>* get() const
    {
        return dynamic_cast<Arg<T>*>(m_data.get());
    }

    P_TYPE m_type = P_TYPE::UNDEFINED;
    std::shared_ptr<IArg> m_data = nullptr;
};
}

inline muse::logger::Stream& operator<<(muse::logger::Stream& s, const mu::engraving::PropertyValue&)
{
    s << "property(not implemented log output)";
    return s;
}
