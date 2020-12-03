
set (BB_SRC
    ${CMAKE_CURRENT_LIST_DIR}/bb.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bb.h
    )

if (NOT MSVC AND NOT MINGW)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
endif (NOT MSVC AND NOT MINGW)
