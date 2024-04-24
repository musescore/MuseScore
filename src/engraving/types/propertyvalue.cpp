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
#include "propertyvalue.h"

#include "realfn.h"

#include "log.h"

using namespace mu::engraving;

bool PropertyValue::isValid() const
{
    return m_type != P_TYPE::UNDEFINED;
}

P_TYPE PropertyValue::type() const
{
    return m_type;
}

bool PropertyValue::operator ==(const PropertyValue& v) const
{
    if (v.m_type == P_TYPE::UNDEFINED || m_type == P_TYPE::UNDEFINED) {
        return v.m_type == m_type;
    }

    //! HACK Temporary hack for bool comparisons (maybe one type is bool and another type is int)
    if (v.m_type == P_TYPE::BOOL || m_type == P_TYPE::BOOL) {
        return v.value<bool>() == value<bool>();
    }

    //! HACK Temporary hack for int comparisons (maybe one type is int and another type is enum)
    if (v.m_type == P_TYPE::INT || m_type == P_TYPE::INT) {
        return v.value<int>() == value<int>();
    }

    //! HACK Temporary hack for Spatium comparisons (maybe one type is Spatium and another type is real)
    if (v.m_type == P_TYPE::SPATIUM || m_type == P_TYPE::SPATIUM) {
        return muse::RealIsEqual(v.value<double>(), value<double>());
    }

    //! HACK Temporary hack for Fraction comparisons
    if (v.m_type == P_TYPE::FRACTION) {
        assert(m_type == P_TYPE::FRACTION);
        return v.value<Fraction>().identical(value<Fraction>());
    }

    if (v.m_type == P_TYPE::REAL) {
        assert(m_type == P_TYPE::REAL);
        return muse::RealIsEqual(v.value<double>(), value<double>());
    }

    assert(m_data);
    if (!m_data) {
        return false;
    }

    assert(v.m_data);
    if (!v.m_data) {
        return false;
    }

    return v.m_type == m_type && v.m_data->equal(m_data.get());
}

#ifndef NO_QT_SUPPORT
QVariant PropertyValue::toQVariant() const
{
    switch (m_type) {
    case P_TYPE::UNDEFINED:   return QVariant();
    // Base
    case P_TYPE::BOOL:        return value<bool>();
    case P_TYPE::INT:         return value<int>();
    case P_TYPE::INT_VEC: {
        std::vector<int> vec = value<std::vector<int> >();
        return QVariant::fromValue(QList<int>(vec.begin(), vec.end()));
    } break;
    case P_TYPE::SIZE_T:      return static_cast<int>(value<size_t>());
    case P_TYPE::REAL:        return value<double>();
    case P_TYPE::STRING:      return value<QString>();

    // Geometry
    case P_TYPE::POINT:       return value<PointF>().toQPointF();
    case P_TYPE::SIZE:        return value<SizeF>().toQSizeF();
    case P_TYPE::DRAW_PATH: {
        NOT_SUPPORTED;
    }
    break;
    case P_TYPE::SCALE:       return value<ScaleF>().toQSizeF();
    case P_TYPE::SPATIUM:     return value<Spatium>().val();
    case P_TYPE::MILLIMETRE:  return value<Millimetre>().val();
    case P_TYPE::PAIR_REAL:   return QVariant::fromValue(value<PairF>().toQPairF());

    // Draw
    case P_TYPE::SYMID:       return static_cast<int>(value<SymId>());
    case P_TYPE::COLOR:       return value<Color>().toQColor();
    case P_TYPE::ORNAMENT_STYLE: return static_cast<int>(value<OrnamentStyle>());
    case P_TYPE::ORNAMENT_INTERVAL: {
        OrnamentInterval interval = value<OrnamentInterval>();
        return QVariantList({ static_cast<int>(interval.step), static_cast<int>(interval.type) });
    } break;
    case P_TYPE::ORNAMENT_SHOW_ACCIDENTAL: return static_cast<int>(value<OrnamentShowAccidental>());
    case P_TYPE::GLISS_STYLE: return static_cast<int>(value<GlissandoStyle>());
    case P_TYPE::GLISS_TYPE: return static_cast<int>(value<GlissandoType>());

    // Layout
    case P_TYPE::ALIGN: {
        Align a = value<Align>();
        return QVariantList({ static_cast<int>(a.horizontal), static_cast<int>(a.vertical) });
    } break;
    case P_TYPE::PLACEMENT_V: return static_cast<int>(value<PlacementV>());
    case P_TYPE::PLACEMENT_H: return static_cast<int>(value<PlacementH>());
    case P_TYPE::TEXT_PLACE:  return static_cast<int>(value<TextPlace>());
    case P_TYPE::DIRECTION_V: return static_cast<int>(value<DirectionV>());
    case P_TYPE::DIRECTION_H: return static_cast<int>(value<DirectionH>());
    case P_TYPE::ORIENTATION: return static_cast<int>(value<Orientation>());
    case P_TYPE::BEAM_MODE:   return static_cast<int>(value<BeamMode>());
    case P_TYPE::ACCIDENTAL_ROLE: return static_cast<int>(value<AccidentalRole>());
    case P_TYPE::ARTIC_STEM_H_ALIGN: return static_cast<int>(value<ArticulationStemSideAlign>());

    // Sound
    case P_TYPE::FRACTION:    return value<Fraction>().toString().toQString();
    case P_TYPE::DURATION_TYPE_WITH_DOTS: {
        DurationTypeWithDots d = value<DurationTypeWithDots>();
        return QVariantMap({ { "type", static_cast<int>(d.type) }, { "dots", d.dots } });
    }
    case P_TYPE::CHANGE_METHOD:    return static_cast<int>(value<ChangeMethod>());
    case P_TYPE::PITCH_VALUES:     return pitchValuesToQVariant(value<PitchValues>());
    case P_TYPE::TEMPO:            return value<BeatsPerSecond>().val;

    // Types
    case P_TYPE::LAYOUTBREAK_TYPE: return static_cast<int>(value<LayoutBreakType>());
    case P_TYPE::VELO_TYPE:        return static_cast<int>(value<VeloType>());
    case P_TYPE::BARLINE_TYPE:     return static_cast<int>(value<BarLineType>());
    case P_TYPE::NOTEHEAD_TYPE:    return static_cast<int>(value<NoteHeadType>());
    case P_TYPE::NOTEHEAD_SCHEME:  return static_cast<int>(value<NoteHeadScheme>());
    case P_TYPE::NOTEHEAD_GROUP:   return static_cast<int>(value<NoteHeadGroup>());
    case P_TYPE::CLEF_TYPE:        return static_cast<int>(value<ClefType>());
    case P_TYPE::CLEF_TO_BARLINE_POS: return static_cast<int>(value<ClefToBarlinePosition>());
    case P_TYPE::DYNAMIC_TYPE:     return static_cast<int>(value<DynamicType>());
    case P_TYPE::DYNAMIC_SPEED:    return static_cast<int>(value<DynamicSpeed>());
    case P_TYPE::LINE_TYPE:        return static_cast<int>(value<LineType>());
    case P_TYPE::HOOK_TYPE:        return static_cast<int>(value<HookType>());
    case P_TYPE::KEY_MODE:         return static_cast<int>(value<KeyMode>());
    case P_TYPE::TEXT_STYLE:       return static_cast<int>(value<TextStyleType>());
    case P_TYPE::PLAYTECH_TYPE:    return static_cast<int>(value<PlayingTechniqueType>());
    case P_TYPE::TEMPOCHANGE_TYPE: return static_cast<int>(value<GradualTempoChangeType>());
    case P_TYPE::SLUR_STYLE_TYPE:  return static_cast<int>(value<SlurStyleType>());
    case P_TYPE::NOTELINE_PLACEMENT_TYPE: return static_cast<int>(value<NoteLineEndPlacement>());
    case P_TYPE::TIE_PLACEMENT:    return static_cast<int>(value<TiePlacement>());
    case P_TYPE::TIE_DOTS_PLACEMENT: return static_cast<int>(value<TieDotsPlacement>());
    case P_TYPE::LYRICS_DASH_SYSTEM_START_TYPE: return static_cast<int>(value<LyricsDashSystemStart>());
    case P_TYPE::PARTIAL_SPANNER_DIRECTION: return static_cast<int>(value<PartialSpannerDirection>());
    case P_TYPE::TIMESIG_PLACEMENT: return static_cast<int>(value<TimeSigPlacement>());
    case P_TYPE::TIMESIG_STYLE:    return static_cast<int>(value<TimeSigStyle>());
    case P_TYPE::TIMESIG_MARGIN: return static_cast<int>(value<TimeSigVSMargin>());

    case P_TYPE::VOICE_ASSIGNMENT: return static_cast<int>(value<VoiceAssignment>());
    case P_TYPE::AUTO_ON_OFF:       return static_cast<int>(value<AutoOnOff>());

    // Other
    case P_TYPE::GROUPS: {
        NOT_SUPPORTED;
    }
    break;
    }

    return QVariant();
}

PropertyValue PropertyValue::fromQVariant(const QVariant& v, P_TYPE type)
{
    switch (type) {
    case P_TYPE::UNDEFINED:
        break;

    // Base
    case P_TYPE::BOOL:          return PropertyValue(v.toBool());
    case P_TYPE::INT:           return PropertyValue(v.toInt());
    case P_TYPE::INT_VEC: {
        QList<int> l = v.value<QList<int> >();
        return PropertyValue(std::vector<int>(l.begin(), l.end()));
    }
    case P_TYPE::SIZE_T:        return PropertyValue(static_cast<int>(v.toInt()));
    case P_TYPE::REAL:          return PropertyValue(v.toReal());
    case P_TYPE::STRING:        return PropertyValue(v.toString());

    // Geometry
    case P_TYPE::POINT:         return PropertyValue(PointF::fromQPointF(v.value<QPointF>()));
    case P_TYPE::SIZE:          return PropertyValue(SizeF::fromQSizeF(v.value<QSizeF>()));
    case P_TYPE::DRAW_PATH: {
        NOT_SUPPORTED;
    }
    break;
    case P_TYPE::SCALE:         return PropertyValue(ScaleF::fromQSizeF(v.value<QSizeF>()));
    case P_TYPE::SPATIUM:       return PropertyValue(Spatium(v.toReal()));
    case P_TYPE::MILLIMETRE:    return PropertyValue(Millimetre(v.toReal()));
    case P_TYPE::PAIR_REAL:     return PropertyValue(PairF::fromQPairF(v.value<QPair<double, double> >()));

    // Draw
    case P_TYPE::SYMID:         return PropertyValue(SymId(v.toInt()));
    case P_TYPE::COLOR:         return PropertyValue(Color::fromQColor(v.value<QColor>()));
    case P_TYPE::ORNAMENT_STYLE: return PropertyValue(OrnamentStyle(v.toInt()));
    case P_TYPE::ORNAMENT_INTERVAL: {
        QVariantList l = v.toList();
        IF_ASSERT_FAILED(l.size() == 2) {
            return PropertyValue();
        }
        return PropertyValue(OrnamentInterval(IntervalStep(l.at(0).toInt()), IntervalType(l.at(1).toInt())));
    } break;
    case P_TYPE::ORNAMENT_SHOW_ACCIDENTAL: return PropertyValue(OrnamentShowAccidental(v.toInt()));
    case P_TYPE::GLISS_STYLE:   return PropertyValue(GlissandoStyle(v.toInt()));
    case P_TYPE::GLISS_TYPE:   return PropertyValue(GlissandoType(v.toInt()));

    // Layout
    case P_TYPE::ALIGN: {
        QVariantList l = v.toList();
        IF_ASSERT_FAILED(l.size() == 2) {
            return PropertyValue();
        }
        return PropertyValue(Align(AlignH(l.at(0).toInt()), AlignV(l.at(1).toInt())));
    } break;
    case P_TYPE::PLACEMENT_V:   return PropertyValue(PlacementV(v.toInt()));
    case P_TYPE::PLACEMENT_H:   return PropertyValue(PlacementH(v.toInt()));
    case P_TYPE::TEXT_PLACE:    return PropertyValue(TextPlace(v.toInt()));
    case P_TYPE::DIRECTION_V:   return PropertyValue(DirectionV(v.toInt()));
    case P_TYPE::DIRECTION_H:   return PropertyValue(DirectionH(v.toInt()));
    case P_TYPE::ORIENTATION:   return PropertyValue(Orientation(v.toInt()));
    case P_TYPE::BEAM_MODE:     return PropertyValue(BeamMode(v.toInt()));
    case P_TYPE::ACCIDENTAL_ROLE: return PropertyValue(AccidentalRole(v.toInt()));
    case P_TYPE::ARTIC_STEM_H_ALIGN: return PropertyValue(ArticulationStemSideAlign(v.toInt()));

    // Duration
    case P_TYPE::FRACTION:      return PropertyValue(Fraction::fromString(v.toString()));
    case P_TYPE::DURATION_TYPE_WITH_DOTS: {
        NOT_SUPPORTED;
    }
    break;
    case P_TYPE::CHANGE_METHOD:    return PropertyValue(ChangeMethod(v.toInt()));
    case P_TYPE::PITCH_VALUES:     return pitchValuesFromQVariant(v);
    case P_TYPE::TEMPO:            return PropertyValue(BeatsPerSecond(v.toDouble()));

    // Types
    case P_TYPE::LAYOUTBREAK_TYPE: return PropertyValue(LayoutBreakType(v.toInt()));
    case P_TYPE::VELO_TYPE:        return PropertyValue(VeloType(v.toInt()));
    case P_TYPE::BARLINE_TYPE:     return PropertyValue(BarLineType(v.toInt()));
    case P_TYPE::NOTEHEAD_TYPE:    return PropertyValue(NoteHeadType(v.toInt()));
    case P_TYPE::NOTEHEAD_SCHEME:  return PropertyValue(NoteHeadScheme(v.toInt()));
    case P_TYPE::NOTEHEAD_GROUP:   return PropertyValue(NoteHeadGroup(v.toInt()));
    case P_TYPE::CLEF_TYPE:        return PropertyValue(ClefType(v.toInt()));
    case P_TYPE::CLEF_TO_BARLINE_POS: return PropertyValue(ClefToBarlinePosition(v.toInt()));
    case P_TYPE::DYNAMIC_TYPE:     return PropertyValue(DynamicType(v.toInt()));
    case P_TYPE::DYNAMIC_SPEED:    return PropertyValue(DynamicSpeed(v.toInt()));
    case P_TYPE::LINE_TYPE:        return PropertyValue(LineType(v.toInt()));
    case P_TYPE::HOOK_TYPE:        return PropertyValue(HookType(v.toInt()));
    case P_TYPE::KEY_MODE:         return PropertyValue(KeyMode(v.toInt()));
    case P_TYPE::TEXT_STYLE:       return PropertyValue(TextStyleType(v.toInt()));
    case P_TYPE::PLAYTECH_TYPE:    return PropertyValue(PlayingTechniqueType(v.toInt()));
    case P_TYPE::TEMPOCHANGE_TYPE: return PropertyValue(GradualTempoChangeType(v.toInt()));
    case P_TYPE::SLUR_STYLE_TYPE:  return PropertyValue(SlurStyleType(v.toInt()));
    case P_TYPE::NOTELINE_PLACEMENT_TYPE:    return PropertyValue(NoteLineEndPlacement(v.toInt()));
    case P_TYPE::TIE_PLACEMENT:    return PropertyValue(TiePlacement(v.toInt()));
    case P_TYPE::TIE_DOTS_PLACEMENT: return PropertyValue(TieDotsPlacement(v.toInt()));
    case P_TYPE::LYRICS_DASH_SYSTEM_START_TYPE:    return PropertyValue(LyricsDashSystemStart(v.toInt()));
    case P_TYPE::PARTIAL_SPANNER_DIRECTION:    return PropertyValue(PartialSpannerDirection(v.toInt()));
    case P_TYPE::VOICE_ASSIGNMENT: return PropertyValue(VoiceAssignment(v.toInt()));
    case P_TYPE::AUTO_ON_OFF:      return PropertyValue(AutoOnOff(v.toInt()));
    case P_TYPE::TIMESIG_PLACEMENT: return PropertyValue(TimeSigPlacement(v.toInt()));
    case P_TYPE::TIMESIG_STYLE:    return PropertyValue(TimeSigStyle(v.toInt()));
    case P_TYPE::TIMESIG_MARGIN:   return PropertyValue(TimeSigVSMargin(v.toInt()));

    // Other
    case P_TYPE::GROUPS: {
        NOT_SUPPORTED;
    }
    break;
    }

    //! NOTE Try determinate type by QVariant type
    switch (v.typeId()) {
    case QMetaType::UnknownType: return PropertyValue();
    case QMetaType::Bool:        return PropertyValue(v.toBool());
    case QMetaType::Int:         return PropertyValue(v.toInt());
    case QMetaType::UInt:        return PropertyValue(v.toInt());
    case QMetaType::LongLong:    return PropertyValue(v.toInt());
    case QMetaType::ULongLong:   return PropertyValue(v.toInt());
    case QMetaType::Double:      return PropertyValue(v.toReal());
    case QMetaType::Char:        return PropertyValue(v.toInt());
    case QMetaType::QString:     return PropertyValue(v.toString());
    case QMetaType::QSize:       return PropertyValue(SizeF::fromQSizeF(QSizeF(v.toSize())));
    case QMetaType::QSizeF:      return PropertyValue(SizeF::fromQSizeF(v.toSizeF()));
    case QMetaType::QPoint:      return PropertyValue(PointF::fromQPointF(QPointF(v.toPoint())));
    case QMetaType::QPointF:     return PropertyValue(PointF::fromQPointF(v.toPointF()));
    case QMetaType::QColor:      return PropertyValue(Color::fromQColor(v.value<QColor>()));
    default:
        break;
    }

    return PropertyValue();
}

#endif // NO_QT_SUPPORT
