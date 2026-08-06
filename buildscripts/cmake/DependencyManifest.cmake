require_dep(ogg)
require_dep(vorbis)
require_dep(flac)
require_dep(opus)
require_dep(libsndfile)

require_source_dep(liblouis)

# mnxdom's dependencies. mnxdom itself is fetched by src/importexport/mnx,
# which points it at these prefixes instead of letting it fetch its own.
if (MUE_BUILD_IMPEXP_MNX_MODULE AND NOT MSS_USE_SYSTEM_MNXDOM)
    require_dep(nlohmann_json)
    require_dep(json_schema_validator)
endif()
