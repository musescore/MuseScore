check_cxx_source_compiles("
    #ifdef __STDC_NO_VLA__
    syntax error;
    #else
    int fvla (int m, int * c)
    {
      int D[m];
      return D[0] == c[0];
    }

    int main(int, char * []) { return 0; }
    #endif"
    HAVE_CXX_VARARRAYS)

add_library(FLAC++
    metadata.cpp
    stream_decoder.cpp
    stream_encoder.cpp)
target_compile_definitions(FLAC++
    PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:FLACPP_API_EXPORTS>
    PUBLIC $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:FLAC__NO_DLL>)
if(NOT WIN32)
    target_compile_definitions(FLAC++ PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:FLAC__USE_VISIBILITY_ATTR>)
endif()
target_include_directories(FLAC++ INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(FLAC++ PUBLIC FLAC)
if(BUILD_SHARED_LIBS)
    set_target_properties(FLAC++ PROPERTIES
        VERSION 6.3.0
        SOVERSION 6)
    if(NOT WIN32)
        set_target_properties(FLAC++ PROPERTIES CXX_VISIBILITY_PRESET hidden)
    endif()
endif()

add_library(FLAC::FLAC++ ALIAS FLAC++)

install(TARGETS FLAC++ EXPORT targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/")

if(INSTALL_PKGCONFIG_MODULES)
    set(prefix "${CMAKE_INSTALL_PREFIX}")
    set(exec_prefix "${CMAKE_INSTALL_PREFIX}")
    set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
    set(includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
    configure_file(flac++.pc.in flac++.pc @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/flac++.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()
