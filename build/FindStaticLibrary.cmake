add_library(zlibstat STATIC IMPORTED)	
set_target_properties(zlibstat PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}/zlibstat${ARCH_TYPE}.lib)   
    