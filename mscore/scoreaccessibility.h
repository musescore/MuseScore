#ifndef __SCORE_ACCESSIBILITY__
#define __SCORE_ACCESSIBILITY__

#include <QStatusBar>
#include <QAccessible>
#include <QAccessibleWidget>
#include "scoreview.h"

namespace  Ms {
//---------------------------------------------------------
//   AccessibleScoreView
//---------------------------------------------------------

#ifndef Q_OS_WIN
// implement value interface so the value change event is generated
// even though it is not an expected property for static text on Linux (AT-SPI)
// on Windows, static text *does* use value, so defining this interface is unnecessary
// and it results in Narrator reading all of the range value information (min/max/current)
// macOS is more like Linux than it is like Windows...
#define SCOREVIEW_VALUEINTERFACE
#endif
//#define SCOREVIEW_IMAGEINTERFACE

#if defined(SCOREVIEW_VALUEINTERFACE)
#define SCOREVIEW_INHERIT_VALUE     ,QAccessibleValueInterface
#else
#define SCOREVIEW_INHERIT_VALUE
#endif
#if defined(SCOREVIEW_IMAGEINTERFACE)
#define SCOREVIEW_INHERIT_IMAGE     ,QAccessibleImageInterface
#else
#define SCOREVIEW_INHERIT_IMAGE
#endif

class AccessibleScoreView : public QObject, QAccessibleWidget SCOREVIEW_INHERIT_VALUE SCOREVIEW_INHERIT_IMAGE
{
    Q_OBJECT
    ScoreView * s;

public:
    AccessibleScoreView(ScoreView* c);
    int childCount() const Q_DECL_OVERRIDE;
    QAccessibleInterface* child(int /*index*/) const Q_DECL_OVERRIDE;
    QAccessibleInterface* parent() const Q_DECL_OVERRIDE;
    QRect rect() const Q_DECL_OVERRIDE;
    bool isValid() const Q_DECL_OVERRIDE;
    QAccessible::State state() const Q_DECL_OVERRIDE;
    QAccessible::Role role() const Q_DECL_OVERRIDE;
    QString text(QAccessible::Text t) const Q_DECL_OVERRIDE;
    QWindow* window() const Q_DECL_OVERRIDE;
    static QAccessibleInterface* ScoreViewFactory(const QString& classname, QObject* object);
    virtual void* interface_cast(QAccessible::InterfaceType t) Q_DECL_OVERRIDE;
#ifdef SCOREVIEW_VALUEINTERFACE
    virtual void setCurrentValue(const QVariant&) Q_DECL_OVERRIDE;
    virtual QVariant currentValue() const Q_DECL_OVERRIDE;
    virtual QVariant maximumValue() const Q_DECL_OVERRIDE;
    virtual QVariant minimumValue() const Q_DECL_OVERRIDE;
    virtual QVariant minimumStepSize() const Q_DECL_OVERRIDE;
#endif
#ifdef SCOREVIEW_IMAGEINTERFACE
    virtual QString imageDescription() const Q_DECL_OVERRIDE;
    virtual QSize imageSize() const Q_DECL_OVERRIDE;
    virtual QPoint imagePosition() const Q_DECL_OVERRIDE;
#endif
};

//---------------------------------------------------------
//   ScoreAccessibility
//---------------------------------------------------------

class ScoreAccessibility : public QObject
{
    Q_OBJECT

    static ScoreAccessibility* inst;
    QMainWindow* mainWindow;
    QLabel* statusBarLabel;
    ScoreAccessibility(QMainWindow* statusBar);
    std::pair<int, float> barbeat(Element* e);
    int _oldStaff = -1;
    int _oldBar = -1;

public:
    ~ScoreAccessibility();
    void updateAccessibilityInfo();
    void clearAccessibilityInfo();
    static void createInstance(QMainWindow* statusBar);
    static ScoreAccessibility* instance();
    void currentInfoChanged();
    static void makeReadable(QString&);

private slots:
    void updateAccessibility();
};
}

#endif
