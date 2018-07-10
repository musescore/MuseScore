add_library(zlibstat STATIC IMPORTED)	
        set_target_properties(zlibstat PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/dependencies/zlib/x86/static/debug/zlibstat.lib)   
    