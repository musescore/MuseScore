add_library(ms_pch STATIC build/pch/pch.cpp)
target_precompile_headers(ms_pch PUBLIC build/pch/pch.h)

function(target_use_pch target_name)
    target_link_libraries(${target_name} ms_pch)
    target_precompile_headers(${target_name} REUSE_FROM ms_pch)
endfunction()
