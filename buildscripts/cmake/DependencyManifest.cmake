require_dep(ogg)
require_dep(vorbis)
require_dep(flac)
require_dep(opus)
require_dep(libsndfile)

require_source_dep(liblouis)

if (MUE_BUILD_IMPEXP_MNX_MODULE)
    require_dep(nlohmann_json)
    require_dep(json_schema_validator)
    require_source_dep(mnx_w3c)
    require_source_dep(mnxdom)
endif()
