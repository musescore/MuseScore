
set(REMOTE_ROOT_URL https://raw.githubusercontent.com/musescore/framework_tmp/main/buildscripts/ci/checkcodestyle)

function(download_uncrustify local_path os_prefix version)
   
    # bin
    file(DOWNLOAD 
        ${REMOTE_ROOT_URL}/tools/${os_prefix}/uncrustify_${version}
        ${local_path}/tools/${os_prefix}/uncrustify_${version}
        SHOW_PROGRESS
    )

    file(CHMOD ${local_path}/tools/${os_prefix}/uncrustify_${version} 
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    # config
    file(DOWNLOAD 
        ${REMOTE_ROOT_URL}/uncrustify_muse.cfg
        ${local_path}/uncrustify_muse.cfg
        SHOW_PROGRESS
    )

endfunction()