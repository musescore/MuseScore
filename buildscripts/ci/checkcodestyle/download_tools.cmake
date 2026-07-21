if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
    message(FATAL_ERROR "This file is a script")
endif()

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

set(MF_URL "https://raw.githubusercontent.com/musescore/muse_framework/96f753f12a8bafc364728420b038a8c99830ca68")
set(MD_URL "https://raw.githubusercontent.com/musescore/muse_deps/718bdec68c5a6231c99e05a925b4998d34358a42")

function(download_file url dest)
    message(STATUS "Downloading ${dest}")
    file(DOWNLOAD "${url}" "${dest}" STATUS status TLS_VERIFY ON)
    list(GET status 0 code)
    if(NOT code EQUAL 0)
        message(FATAL_ERROR "Failed to download ${url}: ${status}")
    endif()
endfunction()

# muse_framework: codestyle tools
set(MF_FILES
    format_dir.cmake
    format_file.cmake
    run_scan.sh
    uncrustify.cmake
    uncrustify_muse.cfg
)
foreach(file ${MF_FILES})
    download_file("${MF_URL}/tools/codestyle/${file}" "${REPO_ROOT}/muse/tools/codestyle/${file}")
endforeach()

# muse_deps: files needed by tools/codestyle/uncrustify.cmake to resolve the uncrustify tool
set(MD_FILES
    buildtools/manifest.cmake
    buildtools/resolve.cmake
    buildtools/build_dependency.cmake
    recipes/uncrustify/meta.cmake
    recipes/uncrustify/spec.cmake
    prebuilt.lock
    prebuilt_url.txt
)
foreach(file ${MD_FILES})
    download_file("${MD_URL}/${file}" "${REPO_ROOT}/muse_deps/${file}")
endforeach()
