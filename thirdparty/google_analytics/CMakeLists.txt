get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message("Configuring " ${DIR_NAME})

find_package(Qt5 COMPONENTS Core Network REQUIRED CONFIG)
add_definitions(
    ${Qt5Core_DEFINITIONS}
    ${Qt5Network_DEFINITIONS}
)
include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(LIBRARY_TYPE STATIC)
if (DYNAMIC_LINKING)
    set(LIBRARY_TYPE SHARED)
endif ()

file(GLOB_RECURSE google_analytics_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.cpp)
file(GLOB_RECURSE google_analytics_HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.h)

add_library(google_analytics ${LIBRARY_TYPE} ${google_analytics_SOURCES} ${google_analytics_HEADERS})
target_link_libraries(google_analytics Qt5::Core Qt5::Network)

if (MSVC)
  set_target_properties( google_analytics PROPERTIES
    COMPILE_FLAGS "/wd4127 /wd4458 /wd4996"
  )
else (MSVC)
    if (MINGW)
        set_target_properties( google_analytics PROPERTIES
          COMPILE_FLAGS "-Wno-deprecated-declarations"
        )
    endif (MINGW)

endif (MSVC)
