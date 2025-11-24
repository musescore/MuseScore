/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "engravingstylemodel.h"

#include "notation/inotation.h"
#include "notation/inotationstyle.h"

#include "global/types/string.h"

#include "engraving/style/styledef.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::notation;

EngravingStyleModel::EngravingStyleModel(QObject* parent)
    : QAbstractListModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void EngravingStyleModel::init()
{
    onNotationChanged();
    context()->currentNotationChanged().onNotify(this, [this] { onNotationChanged(); });
}

QVariant EngravingStyleModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const StyleDef::StyleValue& t = StyleDef::styleValues[static_cast<size_t>(index.row())];
    switch (role) {
    case TitleRole: return QString::fromStdString(t.xmlName.ascii());
    case KeyRole:   return QVariant(static_cast<int>(t.idx()));
    case TypeRole:  return typeToString(t.valueType());
    case ValueRole:
        if (INotationPtr notation = context()->currentNotation()) {
            return notation->style()->styleValue(t.sid).toQVariant();
        }
        break;
    case isDefaultRole:
        if (INotationPtr notation = context()->currentNotation()) {
            return QVariant(notation->style()->styleValue(t.sid) == notation->style()->defaultStyleValue(t.sid));
        }
        break;
    }
    return QVariant();
}

int EngravingStyleModel::rowCount(const QModelIndex&) const
{
    return int(Sid::STYLES);
}

QHash<int, QByteArray> EngravingStyleModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { TitleRole, "titleRole" },
        { KeyRole, "keyRole" },
        { TypeRole, "typeRole" },
        { ValueRole, "valueRole" },
        { isDefaultRole, "isDefaultRole" }
    };
    return roles;
}

void EngravingStyleModel::changeVal(int idx, QVariant newVal)
{
    if (!context()->currentNotation()) {
        return;
    }

    StyleId sid = StyleId(idx);

    if (newVal.isNull()) {
        newVal = context()->currentNotation()->style()->defaultStyleValue(sid).toQVariant();
    }

    if (context()->currentNotation()->style()->styleValue(sid).toQVariant() == newVal) {
        return;
    }

    LOGD() << "changeVal index: " << idx << ", newVal: " << newVal;
    context()->currentNotation()->undoStack()->prepareChanges(
        muse::TranslatableString::untranslatable("Edit style: set %1 to %2").arg(
            TranslatableString::untranslatable(String(StyleDef::styleValues[static_cast<size_t>(idx)].xmlName.ascii())),
            TranslatableString::untranslatable(String::fromQString(newVal.value<QString>()))
            ));

    context()->currentNotation()->style()->setStyleValue(sid, PropertyValue::fromQVariant(newVal, MStyle::valueType(sid)));

    emit dataChanged(index(idx), index(idx));

    context()->currentNotation()->undoStack()->commitChanges();
    context()->currentNotation()->style()->styleChanged().notify();
}

void EngravingStyleModel::onNotationChanged()
{
    beginResetModel();
    endResetModel();
}

QString EngravingStyleModel::typeToString(P_TYPE pt) const
{
    switch (pt) {
    case P_TYPE::BOOL:          return "Bool";
    case P_TYPE::INT:           return "Int";
    case P_TYPE::INT_VEC:       return "Undefined";
    case P_TYPE::SIZE_T:        return "Int";
    case P_TYPE::REAL:          return "Double";
    case P_TYPE::STRING:        return "String";

    // Geometry
    case P_TYPE::POINT:         return "Point";
    case P_TYPE::SIZE:          return "Size";
    case P_TYPE::DRAW_PATH:     return "Undefined";
    case P_TYPE::SCALE:         return "Size";
    case P_TYPE::SPATIUM:       return "Double";
    case P_TYPE::MILLIMETRE:    return "Double";
    case P_TYPE::PAIR_REAL:     return "Undefined";

    // Draw
    case P_TYPE::SYMID:         return "Int";
    case P_TYPE::COLOR:         return "Color";
    case P_TYPE::ORNAMENT_STYLE: return "Int";
    case P_TYPE::ORNAMENT_INTERVAL: return "Int";
    case P_TYPE::ORNAMENT_SHOW_ACCIDENTAL: return "Int";
    case P_TYPE::GLISS_STYLE:   return "Int";
    case P_TYPE::GLISS_TYPE:   return "Int";

    // Layout
    case P_TYPE::ALIGN:
    case P_TYPE::ALIGN_H:       return "Int";
    case P_TYPE::PLACEMENT_V:   return "Int";
    case P_TYPE::PLACEMENT_H:   return "Int";
    case P_TYPE::TEXT_PLACE:    return "Int";
    case P_TYPE::DIRECTION_V:   return "Int";
    case P_TYPE::DIRECTION_H:   return "Int";
    case P_TYPE::ORIENTATION:   return "Int";
    case P_TYPE::BEAM_MODE:     return "Int";
    case P_TYPE::ACCIDENTAL_ROLE: return "Int";

    // Duration
    case P_TYPE::FRACTION:      return "String";
    case P_TYPE::DURATION_TYPE_WITH_DOTS: return "Undefined";
    case P_TYPE::CHANGE_METHOD:    return "Int";
    case P_TYPE::PITCH_VALUES:     return "Undefined";
    case P_TYPE::TEMPO:            return "Double";

    // Types
    case P_TYPE::LAYOUTBREAK_TYPE: return "Int";
    case P_TYPE::VELO_TYPE:        return "Int";
    case P_TYPE::BARLINE_TYPE:     return "Int";
    case P_TYPE::NOTEHEAD_TYPE:    return "Int";
    case P_TYPE::NOTEHEAD_SCHEME:  return "Int";
    case P_TYPE::NOTEHEAD_GROUP:   return "Int";
    case P_TYPE::CLEF_TYPE:        return "Int";
    case P_TYPE::CLEF_TO_BARLINE_POS: return "Int";
    case P_TYPE::DYNAMIC_TYPE:     return "Int";
    case P_TYPE::DYNAMIC_SPEED:    return "Int";
    case P_TYPE::LINE_TYPE:        return "Int";
    case P_TYPE::HOOK_TYPE:        return "Int";
    case P_TYPE::KEY_MODE:         return "Int";
    case P_TYPE::TEXT_STYLE:       return "Int";
    case P_TYPE::PLAYTECH_TYPE:    return "Int";
    case P_TYPE::TEMPOCHANGE_TYPE: return "Int";
    case P_TYPE::SLUR_STYLE_TYPE:  return "Int";
    case P_TYPE::NOTELINE_PLACEMENT_TYPE:    return "Int";
    case P_TYPE::TIE_PLACEMENT:    return "Int";
    case P_TYPE::TIE_DOTS_PLACEMENT: return "Int";
    case P_TYPE::LYRICS_DASH_SYSTEM_START_TYPE:    return "Int";
    case P_TYPE::PARTIAL_SPANNER_DIRECTION:    return "Int";
    case P_TYPE::VOICE_ASSIGNMENT: return "Int";
    case P_TYPE::AUTO_ON_OFF:      return "Int";
    case P_TYPE::AUTO_CUSTOM_HIDE: return "Int";
    case P_TYPE::TIMESIG_PLACEMENT: return "Int";
    case P_TYPE::TIMESIG_STYLE:    return "Int";
    case P_TYPE::TIMESIG_MARGIN:   return "Int";
    case P_TYPE::NOTE_SPELLING_TYPE:   return "Int";
    case P_TYPE::CHORD_PRESET_TYPE:   return "Int";
    case P_TYPE::LH_TAPPING_SYMBOL: return "Int";
    case P_TYPE::RH_TAPPING_SYMBOL: return "Int";
    case P_TYPE::PARENTHESES_MODE:   return "Int";
    case P_TYPE::PLAY_COUNT_PRESET:   return "Int";
    case P_TYPE::MARKER_TYPE:         return "Int";
    case P_TYPE::MEASURE_NUMBER_PLACEMENT: return "Int";
    case P_TYPE::UNDEFINED: return "Undefined";
    default: break;
    }
    return "Undefined";
}
