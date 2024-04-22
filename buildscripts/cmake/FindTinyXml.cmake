
if (MUE_COMPILE_USE_SYSTEM_TINYXML)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(tinyxml2 REQUIRED IMPORTED_TARGET tinyxml2)
    set(TINYXML_MODULE_LINK PkgConfig::tinyxml2)
    set(TINYXML_MODULE_DEF "SYSTEM_TINYXML")
else()
    set(TINYXML_MODULE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyxml/tinyxml2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyxml/tinyxml2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyxml/mu_patch.h
    )
endif()
set(TinyXml_FOUND ON)
