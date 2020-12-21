
##
## freetype2 >= 2.5.2
##

if (USE_SYSTEM_FREETYPE)
      if (OS_IS_MAC)
            PKGCONFIG1 (freetype2 2.5.2 FREETYPE_INCLUDE_DIRS FREETYPE_LIBDIR FREETYPE_LIBRARIES FREETYPE_CPP)
            if (FREETYPE_INCLUDE_DIRS)
                  STRING(REGEX REPLACE  "\"" "" FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
                  STRING(REGEX REPLACE  "\"" "" FREETYPE_LIBDIR ${FREETYPE_LIBDIR})
                  message("freetype2 detected ${FREETYPE_INCLUDE_DIRS} ${FREETYPE_LIBDIR} ${FREETYPE_LIBRARIES}")
            else (FREETYPE_INCLUDE_DIRS)
                  message(FATAL_ERROR "freetype >= 2.5.2 is required\n")
            endif (FREETYPE_INCLUDE_DIRS)
      else (OS_IS_MAC)
            find_package(Freetype REQUIRED)
      endif (OS_IS_MAC)
endif (USE_SYSTEM_FREETYPE)


if (USE_SYSTEM_FREETYPE)
      include_directories(${FREETYPE_INCLUDE_DIRS})
else (USE_SYSTEM_FREETYPE)
      include_directories(${PROJECT_SOURCE_DIR}/thirdparty/freetype/include)
endif (USE_SYSTEM_FREETYPE)

if (NOT USE_SYSTEM_FREETYPE)
      add_subdirectory(thirdparty/freetype)
endif (NOT USE_SYSTEM_FREETYPE)
