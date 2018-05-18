include_directories(
      ${PROJECT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_SOURCE_DIR}/thirdparty/freetype/include
      )

add_executable(
      difftest
      ${ui_headers}
      ${mocs}
      difftest.cpp
      )

target_link_libraries(
      difftest
      ${QT_LIBRARIES}
      testResources
      libmscore
      synthesizer
      midi
      qzip
      z
      )

if (OMR)
      target_link_libraries(difftest omr poppler)
      if (OCR)
            target_link_libraries(difftest tesseract_api)
      endif (OCR)
endif (OMR)

target_link_libraries(
      difftest
      ${QT_LIBRARIES}
      mscore_freetype
      z
      )

if (NOT MINGW AND NOT APPLE)
   target_link_libraries(difftest
      dl
      pthread)
endif (NOT MINGW AND NOT APPLE)

if (APPLE)
target_link_libraries(difftest ${OsxFrameworks})
target_link_libraries(difftest
      dl
      pthread
      )
set_target_properties (
      difftest
      PROPERTIES
      AUTOMOC true
      COMPILE_FLAGS "-include all.h -D QT_GUI_LIB -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"
      LINK_FLAGS    "-g -stdlib=libc++"
      )
else(APPLE)
set_target_properties (
      difftest
      PROPERTIES
      AUTOMOC true
      COMPILE_FLAGS "-include all.h -D QT_GUI_LIB -D TESTROOT=\\\"${PROJECT_SOURCE_DIR}\\\" -g -Wall -Wextra"
      LINK_FLAGS    "-g"
      )
endif(APPLE)

if (APPLE AND (CMAKE_VERSION VERSION_LESS "3.5.0"))
set_target_properties (
      difftest
      PROPERTIES
      COMPILE_FLAGS "-include all.h -D QT_GUI_LIB -D TESTROOT=\\\\\"${PROJECT_SOURCE_DIR}\\\\\" -g -Wall -Wextra"
      )
endif (APPLE AND (CMAKE_VERSION VERSION_LESS "3.5.0"))
