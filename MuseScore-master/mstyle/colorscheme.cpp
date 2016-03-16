/* This file is part of the KDE project
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "colorscheme.h"
#include "colorutils.h"

static const double CONTRAST = .5;

//---------------------------------------------------------
//   StateEffects
//---------------------------------------------------------

class StateEffects {
      public:
            StateEffects(QPalette::ColorGroup state);
            ~StateEffects() {}

            QBrush brush(const QBrush& background) const;
            QBrush brush(const QBrush& foreground, const QBrush& background) const;

      private:
            enum Effects {
                  Intensity = 0,
                  Color     = 1,
                  Contrast  = 2,
                  // Intensity
                  IntensityNoEffect = 0,
                  IntensityShade = 1,
                  IntensityDarken = 2,
                  IntensityLighten = 3,
                  // Color
                  ColorNoEffect = 0,
                  ColorDesaturate = 1,
                  ColorFade = 2,
                  ColorTint = 3,
                  // Contrast
                  ContrastNoEffect = 0,
                  ContrastFade = 1,
                  ContrastTint = 2
                  };

            int _effects[3];
            double _amount[3];
            QColor _color;
      };

StateEffects::StateEffects(QPalette::ColorGroup state)
      : _color(0, 0, 0, 0) {
      QString group;
      if (state == QPalette::Disabled)
            group = "ColorEffects:Disabled";
      else if (state == QPalette::Inactive)
            group = "ColorEffects:Inactive";
      else {
            _effects[0] = 0;
            _effects[1] = 0;
            _effects[2] = 0;
            }

      // NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
      if (!group.isEmpty()) {
            bool dp = state == QPalette::Disabled;
            _effects[Intensity] = (int)(dp ?  IntensityDarken : IntensityNoEffect);
            _effects[Color]     = (int)(dp ?  ColorNoEffect : ColorFade);
            _effects[Contrast]  = (int)(dp ?  ContrastFade : ContrastTint);
            _amount[Intensity]  = dp ? 0.10 : 0.0;
            _amount[Color]      = dp ? 0.0  : 0.025;
            _amount[Contrast]   = dp ? 0.65 : 0.10;
            if (_effects[Color] > ColorNoEffect)
                  _color = dp ?  QColor(56, 56, 56) : QColor(112, 111, 110);
            }
      }

//---------------------------------------------------------
//   brush
//---------------------------------------------------------

QBrush StateEffects::brush(const QBrush& background) const {
      QColor color = background.color(); // TODO - actually work on brushes
      switch (_effects[Intensity]) {
            case IntensityShade:
                  color = ColorUtils::shade(color, _amount[Intensity]);
                  break;
            case IntensityDarken:
                  color = ColorUtils::darken(color, _amount[Intensity]);
                  break;
            case IntensityLighten:
                  color = ColorUtils::lighten(color, _amount[Intensity]);
                  break;
            }
      switch (_effects[Color]) {
            case ColorDesaturate:
                  color = ColorUtils::darken(color, 0.0, 1.0 - _amount[Color]);
                  break;
            case ColorFade:
                  color = ColorUtils::mix(color, _color, _amount[Color]);
                  break;
            case ColorTint:
                  color = ColorUtils::tint(color, _color, _amount[Color]);
                  break;
            }
      return QBrush(color);
      }

QBrush StateEffects::brush(const QBrush& foreground, const QBrush& background) const {
      QColor color = foreground.color(); // TODO - actually work on brushes
      QColor bg = background.color();
      // Apply the foreground effects
      switch (_effects[Contrast]) {
            case ContrastFade:
                  color = ColorUtils::mix(color, bg, _amount[Contrast]);
                  break;
            case ContrastTint:
                  color = ColorUtils::tint(color, bg, _amount[Contrast]);
                  break;
            }
      // Now apply global effects
      return brush(color);
      }

//---------------------------------------------------------
//   SetDefaultColors
//---------------------------------------------------------

struct SetDefaultColors {
      int NormalBackground[3];
      int AlternateBackground[3];
      int NormalText[3];
      int InactiveText[3];
      int ActiveText[3];
      int LinkText[3];
      int VisitedText[3];
      int NegativeText[3];
      int NeutralText[3];
      int PositiveText[3];
      };

//---------------------------------------------------------
//   DecoDefaultColors
//---------------------------------------------------------

struct DecoDefaultColors {
      int Hover[3];
      int Focus[3];
      };

//---------------------------------------------------------
//   defaultViewColors
//---------------------------------------------------------

SetDefaultColors defaultViewColors = {
            { 255, 255, 255 }, // Background
            { 246, 247, 248 }, // Alternate
            {  21,  22,  24 }, // Normal
            { 135, 136, 137 }, // Inactive
            { 255, 128, 224 }, // Active
            {   0,  87, 174 }, // Link
            { 100,  74, 155 }, // Visited
            { 201,  23,  10 }, // Negative
            { 146, 137,   0 }, // Neutral
            {   0, 110,  40 }  // Positive
      };

SetDefaultColors defaultWindowColors = {
            { 207, 209, 213 }, // Background
            { 216, 217, 218 }, // Alternate
            {  24,  25,  27 }, // Normal
            { 135, 136, 137 }, // Inactive
            { 255, 128, 224 }, // Active
            {   0,  87, 174 }, // Link
            { 100,  74, 155 }, // Visited
            { 201,  23,  10 }, // Negative
            { 146, 137,   0 }, // Neutral
            { 0,   110,  40 }  // Positive
      };

SetDefaultColors defaultButtonColors = {
            { 201, 204, 207 }, // Background
            { 222, 223, 224 }, // Alternate
            {  24,  25,  27 }, // Normal
            { 135, 136, 137 }, // Inactive
            { 255, 128, 224 }, // Active
            {   0,  87, 174 }, // Link
            { 100,  74, 155 }, // Visited
            { 201,  23,  10 }, // Negative
            { 146, 137,   0 }, // Neutral
            {   0, 110,  40 }  // Positive
      };

SetDefaultColors defaultSelectionColors = {
            {  23, 120, 219 }, // Background
            {  65, 113, 162 }, // Alternate
            { 255, 255, 255 }, // Normal
            { 165, 193, 228 }, // Inactive
            { 255, 128, 224 }, // Active
            {   0,  49, 110 }, // Link
            {  69,  40, 134 }, // Visited
            { 201,  23,  10 }, // Negative
            { 251, 240,   0 }, // Neutral
            { 173, 212, 186 }  // Positive
      };

SetDefaultColors defaultTooltipColors = {
            { 190, 223, 255 }, // Background
            { 196, 224, 255 }, // Alternate
            {  33,  35,  37 }, // Normal
            { 135, 136, 137 }, // Inactive
            { 255, 128, 224 }, // Active
            {   0,  87, 174 }, // Link
            { 100,  74, 155 }, // Visited
            { 201,  23,  10 }, // Negative
            { 171, 161,   0 }, // Neutral
            {   0, 110,  40 }  // Positive
      };

DecoDefaultColors defaultDecorationColors = {
            {  23, 120, 219 }, // Hover
            { 149, 156, 164 }, // Focus
      };

//---------------------------------------------------------
//   ColorSchemePrivate
//---------------------------------------------------------

class ColorSchemePrivate : public QSharedData {
            struct {
                  QBrush fg[8], bg[8], deco[2];
                  } _brushes;
            qreal _contrast;
            void init(QPalette::ColorGroup, const char*, SetDefaultColors);

      public:
            explicit ColorSchemePrivate(QPalette::ColorGroup, const char*, SetDefaultColors);
            explicit ColorSchemePrivate(QPalette::ColorGroup, const char*, SetDefaultColors, const QBrush&);
            ~ColorSchemePrivate() {}

            QBrush background(ColorScheme::BackgroundRole) const;
            QBrush foreground(ColorScheme::ForegroundRole) const;
            QBrush decoration(ColorScheme::DecorationRole) const;
            qreal contrast() const;
      };

#define DEFAULT(c)      QColor(c[0], c[1], c[2])
#define SET_DEFAULT(a)  DEFAULT(defaults.a)
#define DECO_DEFAULT(a) DEFAULT(defaultDecorationColors.a)

ColorSchemePrivate::ColorSchemePrivate(QPalette::ColorGroup state, const char* group, SetDefaultColors defaults) {
      _contrast = CONTRAST;    // KGlobalSettings::contrastF( config );

      // loaded-from-config colors (no adjustment)
      _brushes.bg[0] = SET_DEFAULT(NormalBackground);
      _brushes.bg[1] = SET_DEFAULT(AlternateBackground);

      // the rest
      init(state, group, defaults);
      }

ColorSchemePrivate::ColorSchemePrivate(QPalette::ColorGroup state,
                                       const char* group, SetDefaultColors defaults, const QBrush& tint) {
      _contrast = CONTRAST; // KGlobalSettings::contrastF( config );

      // loaded-from-config colors
      _brushes.bg[0] = SET_DEFAULT(NormalBackground);
      _brushes.bg[1] = SET_DEFAULT(AlternateBackground);

      // adjustment
      _brushes.bg[0] = ColorUtils::tint(_brushes.bg[0].color(), tint.color(), 0.4);
      _brushes.bg[1] = ColorUtils::tint(_brushes.bg[1].color(), tint.color(), 0.4);

      // the rest
      init(state, group, defaults);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void ColorSchemePrivate::init(QPalette::ColorGroup state, const char* /*group*/, SetDefaultColors defaults) {
      // loaded-from-config colors
      _brushes.fg[0] = SET_DEFAULT(NormalText);
      _brushes.fg[1] = SET_DEFAULT(InactiveText);
      _brushes.fg[2] = SET_DEFAULT(ActiveText);
      _brushes.fg[3] = SET_DEFAULT(LinkText);
      _brushes.fg[4] = SET_DEFAULT(VisitedText);
      _brushes.fg[5] = SET_DEFAULT(NegativeText);
      _brushes.fg[6] = SET_DEFAULT(NeutralText);
      _brushes.fg[7] = SET_DEFAULT(PositiveText);

      _brushes.deco[0] = DECO_DEFAULT(Hover);
      _brushes.deco[1] = DECO_DEFAULT(Focus);

      // apply state adjustments
      if (state != QPalette::Active) {
            StateEffects effects(state);
            for (int i = 0; i < 8; i++) {
                  _brushes.fg[i] = effects.brush(_brushes.fg[i], _brushes.bg[0]);
                  }
            _brushes.deco[0] = effects.brush(_brushes.deco[0], _brushes.bg[0]);
            _brushes.deco[1] = effects.brush(_brushes.deco[1], _brushes.bg[0]);
            _brushes.bg[0] = effects.brush(_brushes.bg[0]);
            _brushes.bg[1] = effects.brush(_brushes.bg[1]);
            }

      // calculated backgrounds
      _brushes.bg[2] = ColorUtils::tint(_brushes.bg[0].color(), _brushes.fg[2].color());
      _brushes.bg[3] = ColorUtils::tint(_brushes.bg[0].color(), _brushes.fg[3].color());
      _brushes.bg[4] = ColorUtils::tint(_brushes.bg[0].color(), _brushes.fg[4].color());
      _brushes.bg[5] = ColorUtils::tint(_brushes.bg[0].color(), _brushes.fg[5].color());
      _brushes.bg[6] = ColorUtils::tint(_brushes.bg[0].color(), _brushes.fg[6].color());
      _brushes.bg[7] = ColorUtils::tint(_brushes.bg[0].color(), _brushes.fg[7].color());
      }

//---------------------------------------------------------
//   background
//---------------------------------------------------------

QBrush ColorSchemePrivate::background(ColorScheme::BackgroundRole role) const {
      switch (role) {
            case ColorScheme::AlternateBackground:
                  return _brushes.bg[1];
            case ColorScheme::ActiveBackground:
                  return _brushes.bg[2];
            case ColorScheme::LinkBackground:
                  return _brushes.bg[3];
            case ColorScheme::VisitedBackground:
                  return _brushes.bg[4];
            case ColorScheme::NegativeBackground:
                  return _brushes.bg[5];
            case ColorScheme::NeutralBackground:
                  return _brushes.bg[6];
            case ColorScheme::PositiveBackground:
                  return _brushes.bg[7];
            default:
                  return _brushes.bg[0];
            }
      }

//---------------------------------------------------------
//   foreground
//---------------------------------------------------------

QBrush ColorSchemePrivate::foreground(ColorScheme::ForegroundRole role) const {
      switch (role) {
            case ColorScheme::InactiveText:
                  return _brushes.fg[1];
            case ColorScheme::ActiveText:
                  return _brushes.fg[2];
            case ColorScheme::LinkText:
                  return _brushes.fg[3];
            case ColorScheme::VisitedText:
                  return _brushes.fg[4];
            case ColorScheme::NegativeText:
                  return _brushes.fg[5];
            case ColorScheme::NeutralText:
                  return _brushes.fg[6];
            case ColorScheme::PositiveText:
                  return _brushes.fg[7];
            default:
                  return _brushes.fg[0];
            }
      }

//---------------------------------------------------------
//   decoration
//---------------------------------------------------------

QBrush ColorSchemePrivate::decoration(ColorScheme::DecorationRole role) const {
      switch (role) {
            case ColorScheme::FocusColor:
                  return _brushes.deco[1];
            default:
                  return _brushes.deco[0];
            }
      }

//---------------------------------------------------------
//   contrast
//---------------------------------------------------------

qreal ColorSchemePrivate::contrast() const {
      return _contrast;
      }

//---------------------------------------------------------
//   ColorScheme
//---------------------------------------------------------

ColorScheme::ColorScheme(const ColorScheme& other)
      : d(other.d) {
      }

ColorScheme& ColorScheme::operator=(const ColorScheme& other) {
      d = other.d;
      return *this;
      }

ColorScheme::~ColorScheme() {
      }

ColorScheme::ColorScheme(QPalette::ColorGroup state, ColorSet set) {
      switch (set) {
            case Window:
                  d = new ColorSchemePrivate(state, "Colors:Window", defaultWindowColors);
                  break;
            case Button:
                  d = new ColorSchemePrivate(state, "Colors:Button", defaultButtonColors);
                  break;
            case Selection: {
                  // KConfigGroup group(config,"ColorEffects:Inactive");
                  // NOTE: keep this in sync with kdebase/workspace/kcontrol/colors/colorscm.cpp
                  bool inactiveSelectionEffect = true; // group.readEntry("ChangeSelectionColor", group.readEntry("Enable", true));
                  // if enabled, inactiver/disabled uses Window colors instead, ala gtk
                  // ...except tinted with the Selection:NormalBackground color so it looks more like selection
                  if (state == QPalette::Active || (state == QPalette::Inactive && !inactiveSelectionEffect))
                        d = new ColorSchemePrivate(state, "Colors:Selection", defaultSelectionColors);
                  else if (state == QPalette::Inactive)
                        d = new ColorSchemePrivate(state, "Colors:Window", defaultWindowColors,
                                                   ColorScheme(QPalette::Active, Selection).background());
                  else // disabled (...and still want this branch when inactive+disabled exists)
                        d = new ColorSchemePrivate(state, "Colors:Window", defaultWindowColors);
                  }
            break;
            case Tooltip:
                  d = new ColorSchemePrivate(state, "Colors:Tooltip", defaultTooltipColors);
                  break;
            default:
                  d = new ColorSchemePrivate(state, "Colors:View", defaultViewColors);
            }
      }

//---------------------------------------------------------
//   background
//---------------------------------------------------------

QBrush ColorScheme::background(BackgroundRole role) const {
      return d->background(role);
      }

//---------------------------------------------------------
//   foreground
//---------------------------------------------------------

QBrush ColorScheme::foreground(ForegroundRole role) const {
      return d->foreground(role);
      }

//---------------------------------------------------------
//   decoration
//---------------------------------------------------------

QBrush ColorScheme::decoration(DecorationRole role) const {
      return d->decoration(role);
      }

//---------------------------------------------------------
//   shade
//---------------------------------------------------------

QColor ColorScheme::shade(ShadeRole role) const {
      return shade(background().color(), role, d->contrast());
      }

QColor ColorScheme::shade(const QColor& color, ShadeRole role) {
      return shade(color, role, 1.0);
      }

QColor ColorScheme::shade(const QColor& color, ShadeRole role, qreal contrast, qreal chromaAdjust) {
      // nan -> 1.0
      contrast = (1.0 > contrast ? (-1.0 < contrast ? contrast : -1.0) : 1.0);
      qreal y = ColorUtils::luma(color), yi = 1.0 - y;

      // handle very dark colors (base, mid, dark, shadow == midlight, light)
      if (y < 0.006) {
            switch (role) {
                  case ColorScheme::LightShade:
                        return ColorUtils::shade(color, 0.05 + 0.95 * contrast, chromaAdjust);
                  case ColorScheme::MidShade:
                        return ColorUtils::shade(color, 0.01 + 0.20 * contrast, chromaAdjust);
                  case ColorScheme::DarkShade:
                        return ColorUtils::shade(color, 0.02 + 0.40 * contrast, chromaAdjust);
                  default:
                        return ColorUtils::shade(color, 0.03 + 0.60 * contrast, chromaAdjust);
                  }
            }

      // handle very light colors (base, midlight, light == mid, dark, shadow)
      if (y > 0.93) {
            switch (role) {
                  case ColorScheme::MidlightShade:
                        return ColorUtils::shade(color, -0.02 - 0.20 * contrast, chromaAdjust);
                  case ColorScheme::DarkShade:
                        return ColorUtils::shade(color, -0.06 - 0.60 * contrast, chromaAdjust);
                  case ColorScheme::ShadowShade:
                        return ColorUtils::shade(color, -0.10 - 0.90 * contrast, chromaAdjust);
                  default:
                        return ColorUtils::shade(color, -0.04 - 0.40 * contrast, chromaAdjust);
                  }
            }

      // handle everything else
      qreal lightAmount = (0.05 + y * 0.55) * (0.25 + contrast * 0.75);
      qreal darkAmount =  (     - y       ) * (0.55 + contrast * 0.35);
      switch (role) {
            case ColorScheme::LightShade:
                  return ColorUtils::shade(color, lightAmount, chromaAdjust);
            case ColorScheme::MidlightShade:
                  return ColorUtils::shade(color, (0.15 + 0.35 * yi) * lightAmount, chromaAdjust);
            case ColorScheme::MidShade:
                  return ColorUtils::shade(color, (0.35 + 0.15 * y) * darkAmount, chromaAdjust);
            case ColorScheme::DarkShade:
                  return ColorUtils::shade(color, darkAmount, chromaAdjust);
            default:
                  return ColorUtils::darken(ColorUtils::shade(color, darkAmount, chromaAdjust), 0.5 + 0.3 * y);
            }
      }

//---------------------------------------------------------
//   adjustBackground
//---------------------------------------------------------

void ColorScheme::adjustBackground(QPalette& palette, BackgroundRole newRole, QPalette::ColorRole color, ColorSet set) {
      palette.setBrush(QPalette::Active,   color, ColorScheme(QPalette::Active,   set).background(newRole));
      palette.setBrush(QPalette::Inactive, color, ColorScheme(QPalette::Inactive, set).background(newRole));
      palette.setBrush(QPalette::Disabled, color, ColorScheme(QPalette::Disabled, set).background(newRole));
      }

//---------------------------------------------------------
//   adjustForeground
//---------------------------------------------------------

void ColorScheme::adjustForeground(QPalette& palette, ForegroundRole newRole, QPalette::ColorRole color, ColorSet set) {
      palette.setBrush(QPalette::Active,   color, ColorScheme(QPalette::Active,   set).foreground(newRole));
      palette.setBrush(QPalette::Inactive, color, ColorScheme(QPalette::Inactive, set).foreground(newRole));
      palette.setBrush(QPalette::Disabled, color, ColorScheme(QPalette::Disabled, set).foreground(newRole));
      }

//---------------------------------------------------------
//   StatefulBrushPrivate
//---------------------------------------------------------

class StatefulBrushPrivate : public QBrush {
      public:
            StatefulBrushPrivate() : QBrush() {}
            StatefulBrushPrivate(const QBrush& brush) : QBrush(brush) {} // not explicit
      };

StatefulBrush::StatefulBrush() {
      d = new StatefulBrushPrivate[3];
      }

StatefulBrush::StatefulBrush(ColorScheme::ColorSet set, ColorScheme::ForegroundRole role) {
      d = new StatefulBrushPrivate[3];
      d[0] = ColorScheme(QPalette::Active,   set).foreground(role);
      d[1] = ColorScheme(QPalette::Disabled, set).foreground(role);
      d[2] = ColorScheme(QPalette::Inactive, set).foreground(role);
      }

StatefulBrush::StatefulBrush(ColorScheme::ColorSet set, ColorScheme::BackgroundRole role) {
      d = new StatefulBrushPrivate[3];
      d[0] = ColorScheme(QPalette::Active,   set).background(role);
      d[1] = ColorScheme(QPalette::Disabled, set).background(role);
      d[2] = ColorScheme(QPalette::Inactive, set).background(role);
      }

StatefulBrush::StatefulBrush(ColorScheme::ColorSet set, ColorScheme::DecorationRole role) {
      d = new StatefulBrushPrivate[3];
      d[0] = ColorScheme(QPalette::Active,   set).decoration(role);
      d[1] = ColorScheme(QPalette::Disabled, set).decoration(role);
      d[2] = ColorScheme(QPalette::Inactive, set).decoration(role);
      }

StatefulBrush::StatefulBrush(const QBrush& brush) {
      d = new StatefulBrushPrivate[3];
      d[0] = brush;
      d[1] = StateEffects(QPalette::Disabled).brush(brush);
      d[2] = StateEffects(QPalette::Inactive).brush(brush);
      }

StatefulBrush::StatefulBrush(const QBrush& brush, const QBrush& background) {
      d = new StatefulBrushPrivate[3];
      d[0] = brush;
      d[1] = StateEffects(QPalette::Disabled).brush(brush, background);
      d[2] = StateEffects(QPalette::Inactive).brush(brush, background);
      }

StatefulBrush::StatefulBrush(const StatefulBrush& other) {
      d = new StatefulBrushPrivate[3];
      d[0] = other.d[0];
      d[1] = other.d[1];
      d[2] = other.d[2];
      }

StatefulBrush::~StatefulBrush() {
      delete[] d;
      }

//---------------------------------------------------------
//   operator=
//---------------------------------------------------------

StatefulBrush& StatefulBrush::operator=(const StatefulBrush& other) {
      d[0] = other.d[0];
      d[1] = other.d[1];
      d[2] = other.d[2];
      return *this;
      }

//---------------------------------------------------------
//   brush
//---------------------------------------------------------

QBrush StatefulBrush::brush(QPalette::ColorGroup state) const {
      switch (state) {
            case QPalette::Inactive:
                  return d[2];
            case QPalette::Disabled:
                  return d[1];
            default:
                  return d[0];
            }
      }

//---------------------------------------------------------
//   brush
//---------------------------------------------------------

QBrush StatefulBrush::brush(const QPalette& pal) const {
      return brush(pal.currentColorGroup());
      }

//---------------------------------------------------------
//   brush
//---------------------------------------------------------

QBrush StatefulBrush::brush(const QWidget* widget) const {
      return widget ? brush(widget->palette()) : QBrush();
      }

