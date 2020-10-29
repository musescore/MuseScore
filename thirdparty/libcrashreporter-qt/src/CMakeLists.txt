add_subdirectory(libcrashreporter-handler)

if (ENABLE_CRASH_REPORTER)
    add_subdirectory(libcrashreporter-gui)
endif(ENABLE_CRASH_REPORTER)
