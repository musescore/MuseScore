//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __MSTYLE_H__
#define __MSTYLE_H__

#include "tileset.h"
#include "stylehelper.h"

class StyleHelper;
class FrameShadowFactory;
class WidgetExplorer;
class Animations;
class Transitions;

//---------------------------------------------------------
//   MgStyle
//---------------------------------------------------------

class MgStyle : public QCommonStyle {
            Q_OBJECT

            //! arrow orientation
            enum ArrowOrientation {
                  ArrowNone, ArrowUp, ArrowDown, ArrowLeft, ArrowRight
                  };

            //! get polygon corresponding to generic arrow
            enum ArrowSize {
                  ArrowNormal, ArrowSmall, ArrowTiny
                  };

            //! internal option flags to pass arguments around
            enum StyleOption {
                  Sunken = 0x1,
                  Focus = 0x2,
                  Hover = 0x4,
                  Disabled = 0x8,
                  NoFill = 0x10,
                  SubtleShadow = 0x20
                  };

            Q_DECLARE_FLAGS(StyleOptions, StyleOption)

            //! used to store slab characteristics
            class SlabRect {
                  public:
                        //! constructor
                        SlabRect(void): _tiles( TileSet::Ring ) {}

                        //! constructor
                        SlabRect( const QRect& r, const int& tiles ): _r( r ), _tiles( TileSet::Tiles( tiles ) ) {}

                        QRect _r;
                        TileSet::Tiles _tiles;
                  };

            //! list of slabs
            typedef QList<SlabRect> SlabRectList;

            /*!
            tabBar data class needed for
            the rendering of tabbars when
            one tab is being drawn
            */
            class TabBarData: public QObject {
                        QPointer<const MgStyle> _style;   //! pointer to parent style object
                        QPointer<const QWidget> _tabBar; //! pointer to target tabBar
                        bool _dirty;                         //! if true, will paint on next TabBarTabShapeControlCall

                  public:
                        TabBarData(MgStyle* parent):
                              QObject(parent),
                              _style(parent),
                              _dirty(false)
                              {}

                        virtual ~TabBarData() {}

                        //! assign target tabBar
                        void lock(const QWidget* widget) {
                              _tabBar = widget;
                              }

                        //! true if tabbar is locked
                        bool locks( const QWidget* widget ) const {
                              return _tabBar && _tabBar.data() == widget;
                              }

                        void setDirty(const bool& value = true) {
                              _dirty = value;
                              }
                        void release()                          {
                              _tabBar.clear();
                              }
                        virtual void drawTabBarBaseControl(const QStyleOptionTab*, QPainter*, const QWidget*);
                  };

            bool emptyControl(const QStyleOption*, QPainter*, const QWidget*) const {
                  return true;
                  }
            bool drawMenuBarItem(const QStyleOption*, QPainter*, const QWidget*) const;

            //! metrics
            /*! these are copied from the old KStyle WidgetProperties */
            enum InternalMetrics {
                  GlowWidth = 1,

                  // checkbox. Do not change, unless
                  // changing the actual cached pixmap size
                  CheckBox_Size = 21,
                  CheckBox_BoxTextSpace = 4,

                  // combobox
                  ComboBox_FrameWidth = 3,
                  ComboBox_ButtonWidth = 19,
                  ComboBox_ButtonMargin = 2,
                  ComboBox_ButtonMargin_Left = 0,
                  ComboBox_ButtonMargin_Right = 4,
                  ComboBox_ButtonMargin_Top = 2,
                  ComboBox_ButtonMargin_Bottom = 1,

                  ComboBox_ContentsMargin = 0,
                  ComboBox_ContentsMargin_Left = 2,
                  ComboBox_ContentsMargin_Right = 0,
                  ComboBox_ContentsMargin_Top = 0,
                  ComboBox_ContentsMargin_Bottom = 0,

                  // dockwidgets
                  DockWidget_FrameWidth = 0,
                  DockWidget_SeparatorExtend = 3,
                  DockWidget_TitleMargin = 3,

                  // generic frames
                  Frame_FrameWidth = 3,

                  // group boxes
                  GroupBox_FrameWidth = 3,

                  // header
                  Header_TextToIconSpace = 3,
                  Header_ContentsMargin = 3,

                  // icon
                  Icon_SizeSmall = 16,
                  Icon_SizeButton = Icon_SizeSmall,
                  Icon_SizeToolBar = 22,
                  Icon_SizeLarge = 32,
                  Icon_SizeMessageBox = 48,

                  // line edit
                  LineEdit_FrameWidth = 3,

                  // menu item
                  MenuItem_AccelSpace = 16,
                  MenuItem_ArrowWidth = 11,
                  MenuItem_ArrowSpace = 3,
                  MenuItem_CheckWidth = 16,
                  MenuItem_CheckSpace = 8,
                  MenuItem_IconWidth = Icon_SizeToolBar,
                  MenuItem_IconSpace = 8,
                  MenuItem_Margin = 2,
                  MenuItem_MinHeight = 20,
                  MenuItem_MinLeftColWidth = 10,

                  // menu bar item
                  MenuBarItem_Margin = 3,
                  MenuBarItem_Margin_Left = 5,
                  MenuBarItem_Margin_Right = 5,

                  // pushbuttons
                  PushButton_ContentsMargin        = 5,
                  PushButton_ContentsMargin_Left   = 8,
                  PushButton_ContentsMargin_Top    = -1,
                  PushButton_ContentsMargin_Right  = 8,
                  PushButton_ContentsMargin_Bottom = 0,
                  PushButton_MenuIndicatorSize     = 8,
                  PushButton_TextToIconSpace       = 6,

                  // progress bar
                  ProgressBar_BusyIndicatorSize = 10,
                  ProgressBar_GrooveMargin = 2,

                  // scrollbar
                  ScrollBar_MinimumSliderHeight = 21,

                  // spin boxes
                  SpinBox_FrameWidth = 3,
                  SpinBox_ButtonWidth = 19,
                  SpinBox_ButtonMargin = 0,
                  SpinBox_ButtonMargin_Left = 2,
                  SpinBox_ButtonMargin_Right = 6,
                  SpinBox_ButtonMargin_Top = 4,
                  SpinBox_ButtonMargin_Bottom = 2,

                  // splitter
                  Splitter_Width = 3,

                  // tabs
                  TabBar_BaseOverlap = 7,
                  TabBar_BaseHeight = 2,
                  TabBar_ScrollButtonWidth = 18,
                  TabBar_TabContentsMargin = 4,
                  TabBar_TabContentsMargin_Left = 5,
                  TabBar_TabContentsMargin_Right = 5,
                  TabBar_TabContentsMargin_Top = 2,
                  TabBar_TabContentsMargin_Bottom = 4,
                  TabBar_TabOverlap = 0,

                  TabWidget_ContentsMargin = 4,

                  // toolbuttons
                  ToolButton_ContentsMargin = 4,
                  ToolButton_InlineMenuIndicatorSize = 8,
                  ToolButton_InlineMenuIndicatorXOff = -11,
                  ToolButton_InlineMenuIndicatorYOff = -10,
                  ToolButton_MenuIndicatorSize = 11,

                  Tree_MaxExpanderSize = 9
                  };

            //! scrollbar buttons
            enum ScrollBarButtonType {
                  NoButton,
                  SingleButton,
                  DoubleButton
                  };

            //! returns height for scrollbar buttons depending of button types
            int scrollBarButtonHeight( const ScrollBarButtonType& type ) const {
                  switch ( type ) {
                        case NoButton:
                              return _noButtonHeight;
                        case SingleButton:
                              return _singleButtonHeight;
                        case DoubleButton:
                              return _doubleButtonHeight;
                        default:
                              return 0;
                        }
                  }
            //! adjusted slabRect
            inline void adjustSlabRect( SlabRect& slab, const QRect&, bool documentMode, bool vertical ) const;

            //! qdial slab
            void renderDialSlab( QPainter* p, const QRect& r, const QColor& c, const QStyleOption* option, StyleOptions opts = 0 ) const {
                  renderDialSlab( p, r, c, option, opts, -1,  AnimationNone );
                  }

            //! qdial slab
            void renderDialSlab( QPainter*, const QRect&, const QColor&, const QStyleOption*, StyleOptions, qreal, AnimationMode ) const;

            //! generic button slab
            void renderButtonSlab( QPainter* p, QRect r, const QColor& c, StyleOptions opts = 0, TileSet::Tiles tiles = TileSet::Ring) const {
                  renderButtonSlab( p, r, c, opts, -1,  AnimationNone, tiles );
                  }

            //! generic button slab
            void renderButtonSlab( QPainter*, QRect, const QColor&, StyleOptions, qreal, AnimationMode, TileSet::Tiles ) const;

            //! generic slab
            void renderSlab( QPainter* painter, const SlabRect& slab, const QColor& color, StyleOptions options = 0 ) const {
                  renderSlab( painter, slab._r, color, options, slab._tiles );
                  }

            //! generic slab
            void renderSlab( QPainter* painter, QRect rect, const QColor& color, StyleOptions options = 0, TileSet::Tiles tiles = TileSet::Ring) const {
                  renderSlab( painter, rect, color, options, -1, AnimationNone, tiles );
                  }

            //! generic slab
            void renderSlab( QPainter* painter, const SlabRect& slab, const QColor& color, StyleOptions options, qreal opacity, AnimationMode mode ) const {
                  renderSlab( painter, slab._r, color, options, opacity, mode, slab._tiles );
                  }

            //! generic slab
            void renderSlab( QPainter*, QRect, const QColor&, StyleOptions, qreal, AnimationMode, TileSet::Tiles ) const;

            // render tab background
            void renderTabBackground( QPainter*, const QRect&, const QPalette&, const QTabBar::Shape, const QWidget* ) const;

            //! tab background
            /*! this paints window background behind tab when tab is being dragged */
            void fillTabBackground( QPainter*, const QRect&, const QColor&, const QTabBar::Shape, const QWidget* ) const;

            //! tab filling
            void fillTab( QPainter*, const QRect&, const QColor&, const QTabBar::Shape, bool active ) const;

            // scrollbar button types (for addLine and subLine )
            ScrollBarButtonType _addLineButtons;
            ScrollBarButtonType _subLineButtons;

            // metrics for scrollbar buttons
            int _noButtonHeight;
            int _singleButtonHeight;
            int _doubleButtonHeight;

            // mnemonic state
            Qt::TextFlag _mnemonic;

            StyleHelper _helper;
            Animations* _animations;
            Transitions* _transitions;
//      WindowManager* _windowManager;
            FrameShadowFactory* _frameShadowFactory;

            //! widget explorer
//      WidgetExplorer* _widgetExplorer;

            //! tabBar data
            TabBarData* _tabBarData;

            //! pointer to primitive specialized function
            typedef bool (MgStyle::*StylePrimitive)( const QStyleOption*, QPainter*, const QWidget* ) const;
            StylePrimitive _frameFocusPrimitive;

            //! pointer to control specialized function
            typedef bool (MgStyle::*StyleControl)( const QStyleOption*, QPainter*, const QWidget* ) const;
            StyleControl _tabBarTabShapeControl;

            //! pointer to control specialized function
            typedef bool (MgStyle::*StyleComplexControl)( const QStyleOptionComplex*, QPainter*, const QWidget* ) const;

            int _hintCounter;
            int _controlCounter;
            int _subElementCounter;
            QHash<QString, int> _styleElements;

            QStyle::ControlElement CE_CapacityBar;

            void renderSplitter(const QStyleOption*, QPainter*, const QWidget*, bool) const;

            QSize expandSize(const QSize& size, int main, int left = 0, int top = 0, int right = 0, int bottom = 0) const;
            QSize checkBoxSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
            QSize comboBoxSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
            QSize headerSectionSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
            QSize menuItemSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
            QSize pushButtonSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
            QSize tabBarTabSizeFromContents(const QStyleOption*, const QSize& size, const QWidget*) const;
            QSize toolButtonSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
            //! returns true for vertical tabs
            bool isVerticalTab(const QStyleOptionTab* option) const {
                  return isVerticalTab(option->shape);
                  }
            bool isVerticalTab(const QTabBar::Shape& shape) const;
            void polishScrollArea(QAbstractScrollArea* scrollArea) const;

            bool drawPanelTipLabelPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
            bool drawFramePrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawFrameFocusRectPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawFrameGroupBoxPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawFrameMenuPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawFrameTabBarBasePrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawFrameTabWidgetPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawFrameWindowPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawIndicatorArrowUpPrimitive(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
                  return drawIndicatorArrowPrimitive( ArrowUp, option, painter, widget );
                  }
            bool drawIndicatorArrowDownPrimitive(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
                  return drawIndicatorArrowPrimitive( ArrowDown, option, painter, widget );
                  }
            bool drawIndicatorArrowLeftPrimitive(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
                  return drawIndicatorArrowPrimitive( ArrowLeft, option, painter, widget );
                  }
            bool drawIndicatorArrowRightPrimitive(const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
                  return drawIndicatorArrowPrimitive( ArrowRight, option, painter, widget );
                  }
            bool drawIndicatorArrowPrimitive( ArrowOrientation, const QStyleOption*, QPainter*, const QWidget* ) const;

            //! dock widget separators
            /*! it uses the same painting as QSplitter, but due to Qt, the horizontal/vertical convention is inverted */
            bool drawIndicatorDockWidgetResizeHandlePrimitive(const QStyleOption* option,
                        QPainter* painter, const QWidget* widget) const {
                  renderSplitter(option, painter, widget, !(option->state & State_Horizontal));
                  return true;
                  }

            bool drawIndicatorHeaderArrowPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawPanelButtonCommandPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawPanelMenuPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawPanelButtonToolPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawPanelScrollAreaCornerPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawPanelItemViewItemPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawPanelLineEditPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawIndicatorBranchPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawIndicatorButtonDropDownPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawIndicatorCheckBoxPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawIndicatorRadioButtonPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawIndicatorTabTearPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawIndicatorToolBarHandlePrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawIndicatorToolBarSeparatorPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawWidgetPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const;

            //! checkbox state (used for checkboxes _and_ radio buttons)
            enum CheckBoxState {
                  CheckOn,
                  CheckOff,
                  CheckTriState
                  };
            void renderRadioButton(QPainter*, const QRect&, const QPalette&, StyleOptions,
                                   CheckBoxState, qreal opacity = -1, AnimationMode mode = AnimationNone) const;
            QColor slabShadowColor( QColor, StyleOptions, qreal, AnimationMode ) const;

            //! checkbox
            void renderCheckBox( QPainter*, const QRect&, const QPalette&, StyleOptions, CheckBoxState, qreal opacity = -1, AnimationMode mode = AnimationNone ) const;
            void renderScrollBarHole( QPainter*, const QRect&, const QColor&, const Qt::Orientation&, const TileSet::Tiles& = TileSet::Full ) const;
            //! scrollbar handle (non animated)
            void renderScrollBarHandle(
                  QPainter* painter, const QRect& r, const QPalette& palette,
                  const Qt::Orientation& orientation, const bool& hover) const {
                  renderScrollBarHandle( painter, r, palette, orientation, hover, -1 );
                  }

            void renderScrollBarHandle( QPainter*, const QRect&, const QPalette&, const Qt::Orientation&, const bool&, const qreal& ) const;
            void renderScrollBarArrow( QPainter*, const QRect&, const QColor&, const QColor&, ArrowOrientation ) const;
            QColor scrollBarArrowColor( const QStyleOption*, const SubControl&, const QWidget* ) const;
            void renderSliderTickmarks( QPainter*, const QStyleOptionSlider*, const QWidget* ) const;
            QPolygonF genericArrow(ArrowOrientation, ArrowSize = ArrowNormal) const;
            QRect centerRect(const QRect& in, const QSize& s ) const {
                  return centerRect( in, s.width(), s.height() );
                  }
            QRect centerRect(const QRect& in, int w, int h) const {
                  return QRect(in.x() + (in.width() - w) / 2, in.y() + (in.height() - h) / 2, w, h);
                  }
            //! adjust rect based on provided margins
            QRect insideMargin( const QRect& r, int main, int left = 0, int top = 0, int right = 0, int bottom = 0 ) const {
                  return r.adjusted( main + left, main + top, -main - right, -main - bottom );
                  }
            //! generic element
            int newStyleElement( const QString& element, const char* check, int& counter) {
                  if ( !element.contains(check) )
                        return 0;
                  int id = _styleElements.value(element, 0);
                  if ( !id ) {
                        ++counter;
                        id = counter;
                        _styleElements.insert(element, id);
                        }
                  return id;
                  }

            //! style hint
            QStyle::StyleHint newStyleHint( const QString& element ) {
                  return (StyleHint) newStyleElement( element, "SH_", _hintCounter );
                  }

            //! control element
            QStyle::ControlElement newControlElement( const QString& element ) {
                  return (ControlElement)newStyleElement( element, "CE_", _controlCounter );
                  }

            //! subElement
            QStyle::SubElement newSubElement(const QString& element ) {
                  return (SubElement)newStyleElement( element, "SE_", _subElementCounter );
                  }
            bool drawComboBoxComplexControl(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
            bool drawDialComplexControl(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
            bool drawGroupBoxComplexControl(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
            bool drawSliderComplexControl(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
            bool drawSpinBoxComplexControl(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
            bool drawTitleBarComplexControl(const QStyleOptionComplex*, QPainter*, const QWidget*) const;
            bool drawToolButtonComplexControl(const QStyleOptionComplex*, QPainter*, const QWidget*) const;

            bool eventFilterComboBoxContainer( QWidget*, QEvent* );
            bool eventFilterDockWidget( QDockWidget*, QEvent* );
            bool eventFilterMdiSubWindow( QMdiSubWindow*, QEvent* );
            bool eventFilterScrollBar( QWidget*, QEvent* );
            bool eventFilterTabBar( QWidget*, QEvent* );
            bool eventFilterToolBar( QToolBar*, QEvent* );
            bool eventFilterToolBox( QToolBox*, QEvent* );

            bool drawCapacityBarControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawComboBoxLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawDockWidgetTitleControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawHeaderEmptyAreaControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawHeaderLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawHeaderSectionControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawMenuBarItemControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawMenuItemControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawProgressBarControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawProgressBarContentsControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawProgressBarGrooveControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawProgressBarLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawPushButtonLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawRubberBandControl( const QStyleOption*, QPainter*, const QWidget* ) const;

            //! scrollbar
            bool drawScrollBarSliderControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawScrollBarAddLineControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawScrollBarSubLineControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawScrollBarAddPageControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawScrollBarSubPageControl( const QStyleOption*, QPainter*, const QWidget* ) const;

            bool drawShapedFrameControl( const QStyleOption*, QPainter*, const QWidget* ) const;

            // splitters
            bool drawSplitterControl( const QStyleOption* option, QPainter* painter, const QWidget* widget ) const {
                  renderSplitter( option, painter, widget, option->state & State_Horizontal );
                  return true;
                  }

            bool drawTabBarTabLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;

            //! tabbar tabs.
            /*! there are two methods (_Single and _Plain) implemented, to deal with tabbar appearance selected from options */
            bool drawTabBarTabShapeControl_Single( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawTabBarTabShapeControl_Plain( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawToolBarControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawToolBoxTabLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawToolBoxTabShapeControl( const QStyleOption*, QPainter*, const QWidget* ) const;
            bool drawToolButtonLabelControl( const QStyleOption*, QPainter*, const QWidget* ) const;

            QRect handleRTL(const QStyleOption* opt, const QRect& subRect) const {
                  return visualRect(opt->direction, opt->rect, subRect);
                  }

            //! right to left alignment handling
            QPoint handleRTL(const QStyleOption* opt, const QPoint& pos) const {
                  return visualPos(opt->direction, opt->rect, pos);
                  }

            //! tiles from tab orientation
            inline TileSet::Tiles tilesByShape( const QTabBar::Shape& shape) const;

            //! toolbar mask
            /*! this masks out toolbar expander buttons, when visible, from painting */
            QRegion tabBarClipRegion( const QTabBar* ) const;
            inline QStyle::SubControl scrollBarHitTest(const QRect& rect, const QPoint& point, const QStyleOption* option) const;
            inline bool preceeds( const QPoint& point, const QRect& bound, const QStyleOption* option ) const;
            TabBarData& tabBarData() const {
                  return *_tabBarData;
                  }

            QRect groupBoxSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
            QRect comboBoxSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
            QRect scrollBarSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
            QRect sliderSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;
            QRect spinBoxSubControlRect( const QStyleOptionComplex*, SubControl, const QWidget* ) const;

            //! this properly handles single/double or no scrollBar buttons
            QRect scrollBarInternalSubControlRect( const QStyleOptionComplex*, SubControl ) const;

            //! menu item background
            void renderMenuItemBackground( const QStyleOption*, QPainter*, const QWidget* ) const;

            void renderMenuItemRect( const QStyleOption* opt, const QRect& rect, const QPalette& pal,
                                     QPainter* p, qreal opacity = -1 ) const {
                  renderMenuItemRect( opt, rect, pal.color(QPalette::Window), pal, p, opacity );
                  }

            void renderMenuItemRect( const QStyleOption*, const QRect&, const QColor&, const QPalette&, QPainter* p, qreal opacity = -1 ) const;

            //! header background
            void renderHeaderBackground( const QRect&, const QPalette&, QPainter*, const QWidget*, bool horizontal, bool reverse ) const;
            void renderHeaderLines( const QRect&, const QPalette&, QPainter*, TileSet::Tiles ) const;

            //! mdi subwindow titlebar button
            void renderTitleBarButton( QPainter*, const QStyleOptionTitleBar*, const QWidget*, const SubControl& ) const;
            void renderTitleBarIcon( QPainter*, const QRectF&, const SubControl& ) const;
            void renderSpinBoxArrow( QPainter*, const QStyleOptionSpinBox*, const QWidget*, const SubControl& ) const;
            //! default implementation. Does not change anything
            QRect defaultSubElementRect( const QStyleOption* option, const QWidget* ) const {
                  return option->rect;
                  }

            //! pushbutton contents
            QRect pushButtonContentsRect( const QStyleOption* option, const QWidget* ) const {
                  return insideMargin( option->rect,
                                       PushButton_ContentsMargin,
                                       PushButton_ContentsMargin_Left,
                                       PushButton_ContentsMargin_Top,
                                       PushButton_ContentsMargin_Right,
                                       PushButton_ContentsMargin_Bottom );
                  }

            //! toolbox tab
            QRect toolBoxTabContentsRect( const QStyleOption* option, const QWidget* ) const {
                  return insideMargin( option->rect, 0, 5, 0, 5, 0 );
                  }

            QRect checkBoxContentsRect( const QStyleOption* option, const QWidget* ) const {
                  return handleRTL( option, option->rect.adjusted( CheckBox_Size + CheckBox_BoxTextSpace, 0, 0, 0 ) );
                  }

            QRect progressBarContentsRect( const QStyleOption* option, const QWidget* ) const {
                  return insideMargin( option->rect, ProgressBar_GrooveMargin );
                  }

            //! tabBar buttons
            QRect tabBarTabLeftButtonRect( const QStyleOption* option, const QWidget* widget ) const {
                  return tabBarTabButtonRect( SE_TabBarTabLeftButton, option, widget );
                  }

            QRect tabBarTabRightButtonRect( const QStyleOption* option, const QWidget* widget ) const {
                  return tabBarTabButtonRect( SE_TabBarTabRightButton, option, widget );
                  }

            QRect tabBarTabButtonRect( SubElement, const QStyleOption*, const QWidget* ) const;

            // tabbar tab text
            QRect tabBarTabTextRect( const QStyleOption* option, const QWidget* widget ) const {
                  return QCommonStyle::subElementRect( SE_TabBarTabText, option, widget ).adjusted( 6, 0, -6, 0 );
                  }

            // tab widgets
            QRect tabWidgetTabContentsRect( const QStyleOption*, const QWidget* ) const;
            QRect tabWidgetTabPaneRect( const QStyleOption*, const QWidget* ) const;

            QRect tabWidgetLeftCornerRect( const QStyleOption* option, const QWidget* widget ) const;
            QRect tabWidgetRightCornerRect( const QStyleOption* option, const QWidget* widget ) const;
            bool emptyPrimitive( const QStyleOption*, QPainter*, const QWidget* ) const {
                  return true;
                  }
            QIcon getCachedIcon(QString key) const;

      protected slots:
            virtual QIcon standardIcon(StandardPixmap standardIcon,
                        const QStyleOption* option, const QWidget* widget) const;

      public slots:
            void configurationChanged();

      public:
            MgStyle();
            virtual int pixelMetric(PixelMetric, const QStyleOption* = 0, const QWidget* = 0) const;
            virtual int styleHint(StyleHint, const QStyleOption* = 0, const QWidget* = 0, QStyleHintReturn* = 0) const;
            virtual QRect subElementRect(SubElement, const QStyleOption*, const QWidget*) const;
            virtual QRect subControlRect(ComplexControl, const QStyleOptionComplex*, SubControl, const QWidget*) const;
            QSize sizeFromContents(ContentsType, const QStyleOption*, const QSize&, const QWidget*) const;
            SubControl hitTestComplexControl(ComplexControl, const QStyleOptionComplex*, const QPoint&, const QWidget*) const;

            virtual void polish(QWidget* widget);
            void unpolish(QWidget* widget);
            void drawPrimitive(PrimitiveElement, const QStyleOption*, QPainter*, const QWidget*) const;
            void drawControl(ControlElement, const QStyleOption*, QPainter*, const QWidget*) const;
            void drawComplexControl(ComplexControl, const QStyleOptionComplex*, QPainter*, const QWidget*) const;
            virtual void drawItemText(QPainter*, const QRect&, int alignment, const QPalette&, bool enabled,
                                      const QString&, QPalette::ColorRole = QPalette::NoRole) const;

            virtual bool eventFilter(QObject*, QEvent*);

            Animations& animations() const                 {
                  return *_animations;
                  }
            Transitions& transitions() const               {
                  return *_transitions;
                  }
//      WindowManager& windowManager() const           { return *_windowManager; }
            FrameShadowFactory& frameShadowFactory() const {
                  return *_frameShadowFactory;
                  }
//      virtual QPalette standardPalette() const;
      };

//---------------------------------------------------------
//   preceeds
//---------------------------------------------------------

bool MgStyle::preceeds( const QPoint& point, const QRect& bound, const QStyleOption* option ) const {
      if (option->state & QStyle::State_Horizontal) {
            if (option->direction == Qt::LeftToRight)
                  return point.x() < bound.right();
            else
                  return point.x() > bound.x();
            }
      else
            return point.y() < bound.y();
      }

//---------------------------------------------------------
//   scrollBarHitTest
//---------------------------------------------------------

QStyle::SubControl MgStyle::scrollBarHitTest(const QRect& rect, const QPoint& point, const QStyleOption* option) const {
      if (option->state & QStyle::State_Horizontal) {
            if (option->direction == Qt::LeftToRight )
                  return point.x() < rect.center().x() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
            else
                  return point.x() > rect.center().x() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
            }
      else
            return point.y() < rect.center().y() ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine;
      }

//---------------------------------------------------------
//   tilesByShape
//---------------------------------------------------------

TileSet::Tiles MgStyle::tilesByShape( const QTabBar::Shape& shape ) const {
      switch (shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                  return TileSet::Top | TileSet::Left | TileSet::Right;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                  return TileSet::Bottom | TileSet::Left | TileSet::Right;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                  return TileSet::Right | TileSet::Top | TileSet::Bottom;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                  return TileSet::Left | TileSet::Top | TileSet::Bottom;

            default:
                  return TileSet::Ring;
            }
      }

//---------------------------------------------------------
//   adjustSlabRect
//---------------------------------------------------------

void MgStyle::adjustSlabRect(SlabRect& slab, const QRect& tabWidgetRect, bool documentMode, bool vertical) const {
      // no tabWidget found, do nothing
      if ( documentMode || !tabWidgetRect.isValid() )
            return;
      else if ( vertical ) {
            slab._r.setTop( qMax( slab._r.top(), tabWidgetRect.top() ) );
            slab._r.setBottom( qMin( slab._r.bottom(), tabWidgetRect.bottom() ) );
            }
      else {
            slab._r.setLeft( qMax( slab._r.left(), tabWidgetRect.left() ) );
            slab._r.setRight( qMin( slab._r.right(), tabWidgetRect.right() ) );
            }
      return;
      }
#endif

