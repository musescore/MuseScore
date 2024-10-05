#!/usr/bin/env bash
#This is an imitation of a workflow to run it locally for testing and debugging purposes

TRANSIFEX_API_TOKEN="123"

github_run_id=42
GITHUB_ENV=""
DO_SETUP='true'
  
    # - name: "Configure workflow"
    #   env:
    #     pull_request_title: ${{ github.event.pull_request.title }}
    #   run: |
        bash ./buildscripts/ci/tools/make_build_mode_env.sh -e workflow_dispatch -m testing
        BUILD_MODE=$(cat ./build.artifacts/env/build_mode.env)

        DO_UPDATE_TS='false'
        if [[ "$BUILD_MODE" == "testing" || "$BUILD_MODE" == "stable" ]]; then 
            DO_UPDATE_TS='true'
            if [ -z "${TRANSIFEX_API_TOKEN}" ]; then 
              echo "warning: not set TRANSIFEX_API_TOKEN, update .ts disabled" 
              DO_UPDATE_TS='false'
            fi 
        fi

        DO_PLACEHOLDER_TRANSLATIONS='false'
        if [[ "$BUILD_MODE" == "nightly" || "$BUILD_MODE" == "devel" ]]; then
          DO_PLACEHOLDER_TRANSLATIONS='true'
        fi


#    - name: Setup environment
if [ $DO_SETUP == 'true' ]; then
      #run: |
        bash ./buildscripts/ci/linux/setup.sh
fi

#    - name: Generate _en.ts files
#      run: |
        bash ./buildscripts/ci/translation/run_lupdate.sh

#    - name: Update .ts files
#      if: env.DO_UPDATE_TS == 'true'
if [ $DO_UPDATE_TS == 'true' ]; then
#      run: |
        bash ./buildscripts/ci/translation/tx_install.sh -t ${TRANSIFEX_API_TOKEN} -s linux
        bash ./buildscripts/ci/translation/tx_pull.sh
fi        

    # - name: Generate placeholder.ts files
    #   if: env.DO_PLACEHOLDER_TRANSLATIONS == 'true'
    #   run: |
    #     python3 ./tools/translations/placeholder_translations.py
#exit 0
#    - name: Build
#      run: |
#        C_URL=${SENTRY_URL}; if [ -z "$C_URL" ]; then C_URL="''"; fi
        bash ./buildscripts/ci/linux/build.sh -n ${github_run_id}
    # - name: Generate dump symbols
    #   run: |
    #     bash ./buildscripts/ci/linux/dumpsyms.sh  
 #   - name: Package 
#      run: |
        bash ./buildscripts/ci/linux/package.sh
