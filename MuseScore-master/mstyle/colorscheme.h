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

#ifndef __COLORSCHEME_H__
#define __COLORSCHEME_H__

class StatefulBrushPrivate;
class ColorSchemePrivate;

/**
 * A set of methods used to work with colors.
 *
 * ColorScheme currently provides access to the system color palette that the
 * user has selected (in the future, it is expected to do more). As of KDE4,
 * this class is the correct way to look up colors from the system palette,
 * as opposed to KGlobalSettings (such usage is deprecated). It greatly expands
 * on KGlobalSettings and QPalette by providing five distinct "sets" with
 * several color choices each, covering background, foreground, and decoration
 * colors.
 *
 * A ColorScheme instance represents colors corresponding to a "set", where a
 * set consists of those colors used to draw a particular type of element, such
 * as a menu, button, view, selected text, or tooltip. Each set has a distinct
 * set of colors, so you should always use the correct set for drawing and
 * never assume that a particular foreground for one set is the same as the
 * foreground for any other set. Individual colors may be quickly referenced by
 * creating an anonymous instance and invoking a lookup member.
 *
 * @note
 * Historically, it was not needed for applications to give much concern to the
 * state of a widget (active, inactive, disabled) since only the disabled state
 * was different, and only slightly. As a result, the old KGlobalSettings color
 * getters did not care about the widget state. However, starting with KDE4,
 * the color palettes for the various states may be wildly different.
 * Therefore, it is important to take the state into account. This is why the
 * ColorScheme constructor requires a QPalette::ColorGroup as an argument.
 *
 * To facilitate working with potentially-varying states, two convenience API's
 * are provided. These are ColorScheme::adjustBackground and its sister
 * ColorScheme::adjustForeground, and the helper class ::StatefulBrush.
 *
 * @see ColorScheme::ColorSet, ColorScheme::ForegroundRole,
 * ColorScheme::BackgroundRole, ColorScheme::DecorationRole,
 * ColorScheme::ShadeRole
 */
class ColorScheme {
      public:

            /**
             * This enumeration describes the color set for which a color is being
             * selected.
             *
             * Color sets define a color "environment", suitable for drawing all parts
             * of a given region. Colors from different sets should not be combined.
             */
            enum ColorSet {
                  /**
                   * Views; for example, frames, input fields, etc.
                   *
                   * If it contains things that can be selected, it is probably a View.
                   */
                  View,
                  /**
                   * Non-editable window elements; for example, menus.
                   *
                   * If it isn't a Button, View, or Tooltip, it is probably a Window.
                   */
                  Window,
                  /**
                   * Buttons and button-like controls.
                   *
                   * In addition to buttons, "button-like" controls such as non-editable
                   * dropdowns, scrollbar sliders, slider handles, etc. should also use
                   * this role.
                   */
                  Button,
                  /**
                   * Selected items in views.
                   *
                   * Note that unfocused or disabled selections should use the Window
                   * role. This makes it more obvious to the user that the view
                   * containing the selection does not have input focus.
                   */
                  Selection,
                  /**
                   * Tooltips.
                   *
                   * The tooltip set can often be substituted for the view
                   * set when editing is not possible, but the Window set is deemed
                   * inappropriate. "What's This" help is an excellent example, another
                   * might be pop-up notifications (depending on taste).
                   */
                  Tooltip
                  };

            /**
             * This enumeration describes the background color being selected from the
             * given set.
             *
             * Background colors are suitable for drawing under text, and should never
             * be used to draw text. In combination with one of the overloads of
             * ColorScheme::shade, they may be used to generate colors for drawing
             * frames, bevels, and similar decorations.
             */
            enum BackgroundRole {
                  /**
                   * Normal background.
                   */
                  NormalBackground = 0,
                  /**
                   * Alternate background; for example, for use in lists.
                   *
                   * This color may be the same as BackgroundNormal, especially in sets
                   * other than View and Window.
                   */
                  AlternateBackground = 1,
                  /**
                   * Third color; for example, items which are new, active, requesting
                   * attention, etc.
                   *
                   * Alerting the user that a certain field must be filled out would be a
                   * good usage (although NegativeBackground could be used to the same
                   * effect, depending on what you are trying to achieve). Unlike
                   * ActiveText, this should not be used for mouseover effects.
                   */
                  ActiveBackground = 2,
                  /**
                   * Fourth color; corresponds to (unvisited) links.
                   *
                   * Exactly what this might be used for is somewhat harder to qualify;
                   * it might be used for bookmarks, as a 'you can click here' indicator,
                   * or to highlight recent content (i.e. in a most-recently-accessed
                   * list).
                   */
                  LinkBackground = 3,
                  /**
                   * Fifth color; corresponds to visited links.
                   *
                   * This can also be used to indicate "not recent" content, especially
                   * when a color is needed to denote content which is "old" or
                   * "archival".
                   */
                  VisitedBackground = 4,
                  /**
                   * Sixth color; for example, errors, untrusted content, etc.
                   */
                  NegativeBackground = 5,
                  /**
                   * Seventh color; for example, warnings, secure/encrypted content.
                   */
                  NeutralBackground = 6,
                  /**
                   * Eigth color; for example, success messages, trusted content.
                   */
                  PositiveBackground = 7
                  };

            /**
             * This enumeration describes the foreground color being selected from the
             * given set.
             *
             * Foreground colors are suitable for drawing text or glyphs (such as the
             * symbols on window decoration buttons, assuming a suitable background
             * brush is used), and should never be used to draw backgrounds.
             *
             * For window decorations, the following is suggested, but not set in
             * stone:
             * @li Maximize - PositiveText
             * @li Minimize - NeutralText
             * @li Close - NegativeText
             * @li WhatsThis - LinkText
             * @li Sticky - ActiveText
             */
            enum ForegroundRole {
                  /**
                   * Normal foreground.
                   */
                  NormalText = 0,
                  /**
                   * Second color; for example, comments, items which are old, inactive
                   * or disabled. Generally used for things that are meant to be "less
                   * important". InactiveText is not the same role as NormalText in the
                   * inactive state.
                   */
                  InactiveText = 1,
                  /**
                   * Third color; for example items which are new, active, requesting
                   * attention, etc. May be used as a hover color for clickable items.
                   */
                  ActiveText = 2,
                  /**
                   * Fourth color; use for (unvisited) links. May also be used for other
                   * clickable items or content that indicates relationships, items that
                   * indicate somewhere the user can visit, etc.
                   */
                  LinkText = 3,
                  /**
                   * Fifth color; used for (visited) links. As with LinkText, may be used
                   * for items that have already been "visited" or accessed. May also be
                   * used to indicate "historical" (i.e. "old") items or information,
                   * especially if InactiveText is being used in the same context to
                   * express something different.
                   */
                  VisitedText = 4,
                  /**
                   * Sixth color; for example, errors, untrusted content, deletions,
                   * etc.
                   */
                  NegativeText = 5,
                  /**
                   * Seventh color; for example, warnings, secure/encrypted content.
                   */
                  NeutralText = 6,
                  /**
                   * Eigth color; for example, additions, success messages, trusted
                   * content.
                   */
                  PositiveText = 7
                  };

            /**
             * This enumeration describes the decoration color being selected from the
             * given set.
             *
             * Decoration colors are used to draw decorations (such as frames) for
             * special purposes. Like color shades, they are neither foreground nor
             * background colors. Text should not be painted over a decoration color,
             * and decoration colors should not be used to draw text.
             */
            enum DecorationRole {
                  /**
                   * Color used to draw decorations for items which have input focus.
                   */
                  FocusColor,
                  /**
                   * Color used to draw decorations for items which will be activated by
                   * clicking.
                   */
                  HoverColor
                  };

            /**
             * This enumeration describes the color shade being selected from the given
             * set.
             *
             * Color shades are used to draw "3d" elements, such as frames and bevels.
             * They are neither foreground nor background colors. Text should not be
             * painted over a shade, and shades should not be used to draw text.
             */
            enum ShadeRole {
                  /**
                   * The light color is lighter than dark() or shadow() and contrasts
                   * with the base color.
                   */
                  LightShade,
                  /**
                   * The midlight color is in between base() and light().
                   */
                  MidlightShade,
                  /**
                   * The mid color is in between base() and dark().
                   */
                  MidShade,
                  /**
                   * The dark color is in between mid() and shadow().
                   */
                  DarkShade,
                  /**
                   * The shadow color is darker than light() or midlight() and contrasts
                   * the base color.
                   */
                  ShadowShade
                  };

            /** Construct a copy of another ColorScheme. */
            ColorScheme(const ColorScheme&);

            /** Destructor */
            virtual ~ColorScheme();

            /** Standard assignment operator */
            ColorScheme& operator=(const ColorScheme&);

            /**
             * Construct a palette from given color set and state, using the colors
             * from the given KConfig (if null, the system colors are used).
             *
             * @note ColorScheme provides direct access to the color scheme for users
             * that deal directly with widget states. Unless you are a low-level user
             * or have a legitimate reason to only care about a fixed, limited number
             * of states (e.g. windows that cannot be inactive), consider using a
             * ::StatefulBrush instead.
             */
            explicit ColorScheme(QPalette::ColorGroup, ColorSet = View);

            /**
             * Retrieve the requested background brush.
             */
            QBrush background(BackgroundRole = NormalBackground) const;

            /**
             * Retrieve the requested foreground brush.
             */
            QBrush foreground(ForegroundRole = NormalText) const;

            /**
             * Retrieve the requested decoration brush.
             */
            QBrush decoration(DecorationRole) const;

            /**
             * Retrieve the requested shade color, using
             * ColorScheme::background(ColorScheme::NormalBackground)
             * as the base color and the contrast setting from the KConfig used to
             * create this ColorScheme instance (the system contrast setting, if no
             * KConfig was specified).
             *
             * @note Shades are chosen such that all shades would contrast with the
             * base color. This means that if base is very dark, the 'dark' shades will
             * be lighter than the base color, with midlight() == shadow().
             * Conversely, if the base color is very light, the 'light' shades will be
             * darker than the base color, with light() == mid().
             */
            QColor shade(ShadeRole) const;

            /**
             * Retrieve the requested shade color, using the specified color as the
             * base color and the system contrast setting.
             *
             * @note Shades are chosen such that all shades would contrast with the
             * base color. This means that if base is very dark, the 'dark' shades will
             * be lighter than the base color, with midlight() == shadow().
             * Conversely, if the base color is very light, the 'light' shades will be
             * darker than the base color, with light() == mid().
             */
            static QColor shade(const QColor&, ShadeRole);

            /**
             * Retrieve the requested shade color, using the specified color as the
             * base color and the specified contrast.
             *
             * @param contrast Amount roughly specifying the contrast by which to
             * adjust the base color, between -1.0 and 1.0 (values between 0.0 and 1.0
             * correspond to the value from KGlobalSettings::contrastF)
             * @param chromaAdjust (optional) Amount by which to adjust the chroma of
             * the shade (1.0 means no adjustment)
             *
             * @note Shades are chosen such that all shades would contrast with the
             * base color. This means that if base is very dark, the 'dark' shades will
             * be lighter than the base color, with midlight() == shadow().
             * Conversely, if the base color is very light, the 'light' shades will be
             * darker than the base color, with light() == mid().
             *
             * @see KColorUtils::shade
             */
            static QColor shade(const QColor&, ShadeRole, qreal contrast, qreal chromaAdjust = 0.0);

            /**
             * Adjust a QPalette by replacing the specified QPalette::ColorRole with
             * the requested background color for all states. Using this method is
             * safer than replacing individual states, as it insulates you against
             * changes in QPalette::ColorGroup.
             *
             * @note Although it is possible to replace a foreground color using this
             * method, it's bad usability to do so. Just say "no".
             */
            static void adjustBackground(QPalette&,
                                         BackgroundRole newRole = NormalBackground,
                                         QPalette::ColorRole color = QPalette::Base,
                                         ColorSet set = View);

            /**
             * Adjust a QPalette by replacing the specified QPalette::ColorRole with
             * the requested foreground color for all states. Using this method is
             * safer than replacing individual states, as it insulates you against
             * changes in QPalette::ColorGroup.
             *
             * @note Although it is possible to replace a background color using this
             * method, it's bad usability to do so. Just say "no".
             */
            static void adjustForeground(QPalette&,
                                         ForegroundRole newRole = NormalText,
                                         QPalette::ColorRole color = QPalette::Text,
                                         ColorSet set = View);

      private:
            QExplicitlySharedDataPointer<ColorSchemePrivate> d;
      };

/**
 * A container for a "state-aware" brush.
 *
 * StatefulBrush provides an easy and safe way to store a color for use in a
 * user interface. It is "safe" both in that it will make it easy to deal with
 * widget states in a correct manner, and that it insulates you against changes
 * in QPalette::ColorGroup.
 *
 * Basically, a stateful brush is used to cache a particular "color" from the
 * KDE system palette (usually, one which does not live in QPalette) in the way
 * you would have used a QColor in KDE3. When you are ready to draw using the
 * brush, you use the current state to retrieve the appropriate brush.
 *
 * Stateful brushes can also be used to apply state effects to arbitrary
 * brushes, for example when working with a application specific user-defined
 * color palette.
 *
 * @note As of Qt 4.3, QPalette::ColorGroup is missing a state for disabled
 * widgets in an inactive window. Hopefully Trolltech will fix this bug, at
 * which point ColorScheme and StatefulBrush will be updated to recognize the
 * new state. Using StatefulBrush will allow your application to inherit these
 * changes "for free", without even recompiling.
 */
class StatefulBrush {
      public:
            /**
             * Construct a "default" stateful brush. For such an instance, all
             * overloads of StatefulBrush::brush will return a default brush (i.e.
             * <tt>QBrush()</tt>).
             */
            explicit StatefulBrush();

            /**
             * Construct a stateful brush from given color set and foreground role,
             * using the colors from the given KConfig (if null, the system colors are
             * used).
             */
            explicit StatefulBrush(ColorScheme::ColorSet, ColorScheme::ForegroundRole);

            /**
             * Construct a stateful brush from given color set and background role,
             * using the colors from the given KConfig (if null, the system colors are
             * used).
             */
            explicit StatefulBrush(ColorScheme::ColorSet, ColorScheme::BackgroundRole);

            /**
             * Construct a stateful brush from given color set and decoration role,
             * using the colors from the given KConfig (if null, the system colors are
             * used).
             */
            explicit StatefulBrush(ColorScheme::ColorSet, ColorScheme::DecorationRole);

            /**
             * Construct a stateful background brush from a specified QBrush (or
             * QColor, via QBrush's implicit constructor). The various states are
             * determined from the base QBrush (which fills in the Active state)
             * according to the same rules used to build stateful color schemes from
             * the system color scheme. The state effects from the given KConfig are
             * used (if null, the system state effects are used).
             */
            explicit StatefulBrush(const QBrush&);

            /**
             * Construct a stateful foreground/decoration brush from a specified
             * QBrush (or QColor, via QBrush's implicit constructor). The various
             * states are determined from the base QBrush (which fills in the Active
             * state) according to the same rules used to build stateful color schemes
             * from the system color scheme. The state effects from the given KConfig
             * are used (if null, the system state effects are used).
             *
             * @param background The background brush (or color) corresponding to the
             * ColorScheme::NormalBackground role and QPalette::Active state for this
             * foreground/decoration color.
             */
            explicit StatefulBrush(const QBrush&, const QBrush& background);

            /** Construct a copy of another StatefulBrush. */
            StatefulBrush(const StatefulBrush&);

            /** Destructor */
            ~StatefulBrush();

            /** Standard assignment operator */
            StatefulBrush& operator=(const StatefulBrush&);

            /**
             * Retrieve the brush for the specified widget state. This is used when you
             * know explicitly what state is wanted. Otherwise one of overloads is
             * often more convenient.
             */
            QBrush brush(QPalette::ColorGroup) const;

            /**
             * Retrieve the brush, using a QPalette reference to determine the correct
             * state. Use when your painting code has easy access to the QPalette that
             * it is supposed to be using. The state used in this instance is the
             * currentColorGroup of the palette.
             */
            QBrush brush(const QPalette&) const;

            /**
             * Retrieve the brush, using a QWidget pointer to determine the correct
             * state. Use when you have a pointer to the widget that you are painting.
             * The state used is the current state of the widget.
             *
             * @note If you pass an invalid widget, you will get a default brush (i.e.
             * <tt>QBrush()</tt>).
             */
            QBrush brush(const QWidget*) const;

      private:
            class StatefulBrushPrivate* d;
      };

Q_DECLARE_METATYPE(StatefulBrush); /* so we can pass it in QVariant's */

#endif // KCOLORSCHEME_H
