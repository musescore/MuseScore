/****************************************************************************
** Resource object code
**
** Created by: The Resource Compiler for Qt version 6.5.3
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

  // /Users/zainazeeza/MuseScore/fonts/campania/Campania.otf
  
  
  
static const unsigned char qt_resource_data[121176] = { 'Q', 'R', 'C', '_', 'D', 'A', 'T', 'A' };

static const unsigned char qt_resource_name[] = {
  // fonts
  0x0,0x5,
  0x0,0x6d,0x65,0xb3,
  0x0,0x66,
  0x0,0x6f,0x0,0x6e,0x0,0x74,0x0,0x73,
    // campania
  0x0,0x8,
  0x8,0x46,0x89,0xd1,
  0x0,0x63,
  0x0,0x61,0x0,0x6d,0x0,0x70,0x0,0x61,0x0,0x6e,0x0,0x69,0x0,0x61,
    // Campania.otf
  0x0,0xc,
  0xd,0xc4,0xdb,0xa6,
  0x0,0x43,
  0x0,0x61,0x0,0x6d,0x0,0x70,0x0,0x61,0x0,0x6e,0x0,0x69,0x0,0x61,0x0,0x2e,0x0,0x6f,0x0,0x74,0x0,0x66,
  
};

static const unsigned char qt_resource_struct[] = {
  // :
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/fonts
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x2,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/fonts/campania
  0x0,0x0,0x0,0x10,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x3,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/fonts/campania/Campania.otf
  0x0,0x0,0x0,0x26,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,
0x0,0x0,0x1,0x96,0x66,0x28,0x8d,0xc2,

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

int QT_RCC_MANGLE_NAMESPACE(qInitResources_fonts_Campania)();
int QT_RCC_MANGLE_NAMESPACE(qInitResources_fonts_Campania)()
{
    int version = 3;
    QT_RCC_PREPEND_NAMESPACE(qRegisterResourceData)
        (version, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_fonts_Campania)();
int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_fonts_Campania)()
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
       initializer() { QT_RCC_MANGLE_NAMESPACE(qInitResources_fonts_Campania)(); }
       ~initializer() { QT_RCC_MANGLE_NAMESPACE(qCleanupResources_fonts_Campania)(); }
   } dummy;
}

#ifdef __clang__
#   pragma clang diagnostic pop
#endif
