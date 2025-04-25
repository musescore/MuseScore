/****************************************************************************
** Resource object code
**
** Created by: The Resource Compiler for Qt version 6.5.3
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

  // /Users/zainazeeza/MuseScore/fonts/FreeSans.ttf
  
  
  
static const unsigned char qt_resource_data[1563260] = { 'Q', 'R', 'C', '_', 'D', 'A', 'T', 'A' };

static const unsigned char qt_resource_name[] = {
  // fonts
  0x0,0x5,
  0x0,0x6d,0x65,0xb3,
  0x0,0x66,
  0x0,0x6f,0x0,0x6e,0x0,0x74,0x0,0x73,
    // FreeSans.ttf
  0x0,0xc,
  0x1,0xe7,0x2e,0xe6,
  0x0,0x46,
  0x0,0x72,0x0,0x65,0x0,0x65,0x0,0x53,0x0,0x61,0x0,0x6e,0x0,0x73,0x0,0x2e,0x0,0x74,0x0,0x74,0x0,0x66,
  
};

static const unsigned char qt_resource_struct[] = {
  // :
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/fonts
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x2,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/fonts/FreeSans.ttf
  0x0,0x0,0x0,0x10,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,
0x0,0x0,0x1,0x96,0x66,0x28,0x8d,0x46,

};

#ifdef QT_NAMESPACE
#  define QT_RCC_PREPEND_NAMESPACE(name) ::QT_NAMESPACE::name
#  define QT_RCC_MANGLE_NAMESPACE0(x) x
#  define QT_RCC_MANGLE_NAMESPACE1(a, b) a##_##b
#  define QT_RCC_MANGLE_NAMESPACE2(a, b) QT_RCC_MANGLE_NAMESPACE1(a,b)
#  define QT_RCC_MANGLE_NAMESPACE(name) QT_RCC_MANGLE_NAMESPACE2( \
        QT_RCC_MANGLE_NAMESPACE0(name), QT_RCC_MANGLE_NAMESPACE0(QT_NAMESPACE))
#else
#   define QT_RCC_PREPEND_NAMESPACE(name) name
#   define QT_RCC_MANGLE_NAMESPACE(name) name
#endif

#ifdef QT_NAMESPACE
namespace QT_NAMESPACE {
#endif

bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
bool qUnregisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);

#ifdef QT_NAMESPACE
}
#endif

int QT_RCC_MANGLE_NAMESPACE(qInitResources_fonts_FreeSans)();
int QT_RCC_MANGLE_NAMESPACE(qInitResources_fonts_FreeSans)()
{
    int version = 3;
    QT_RCC_PREPEND_NAMESPACE(qRegisterResourceData)
        (version, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_fonts_FreeSans)();
int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_fonts_FreeSans)()
{
    int version = 3;
    QT_RCC_PREPEND_NAMESPACE(qUnregisterResourceData)
       (version, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

namespace {
   struct initializer {
       initializer() { QT_RCC_MANGLE_NAMESPACE(qInitResources_fonts_FreeSans)(); }
       ~initializer() { QT_RCC_MANGLE_NAMESPACE(qCleanupResources_fonts_FreeSans)(); }
   } dummy;
}

#ifdef __clang__
#   pragma clang diagnostic pop
#endif
