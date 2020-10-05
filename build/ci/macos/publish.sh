#!/usr/bin/env bash

echo "Publish MuseScore"

ARTIFACTS_DIR=build.artifacts

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -a|--artifact) ARTIFACT_NAME="$2"; shift ;;
        -s|--secret) OSUOSL_SSH_ENCRYPT_SECRET="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$ARTIFACT_NAME" ]; then ARTIFACT_NAME="$(cat $ARTIFACTS_DIR/env/artifact_name.env)"; fi

if [ -z "$ARTIFACT_NAME" ]; then echo "error: not set ARTIFACT_NAME"; exit 1; fi
if [ -z "$OSUOSL_SSH_ENCRYPT_SECRET" ]; then echo "error: not set OSUOSL_SSH_ENCRYPT_SECRET"; exit 1; fi

echo "ARTIFACT_NAME: $ARTIFACT_NAME"

7z x -y build/ci/tools/osuosl/osuosl_nighlies_rsa.enc -obuild/ci/tools/osuosl/ -p${OSUOSL_SSH_ENCRYPT_SECRET}

SSH_KEY=build/ci/tools/osuosl/osuosl_nighlies_rsa
chmod 600 ${SSH_KEY}

scp -oStrictHostKeyChecking=no -C -i ${SSH_KEY} ${ARTIFACTS_DIR}/${ARTIFACT_NAME} musescore-nightlies@ftp-osl.osuosl.org:~/ftp/macosx

# Delete old files
ssh -i $SSH_KEY musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/macosx; ls MuseScoreNightly* -t | tail -n +41 | xargs rm -f"

# At the moment, the HTML page has not been updated, and the need for it is not clear. 
# Therefore, we will disable the HTML page.
ssh -i $SSH_KEY musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/macosx; mv index.html index.html_off"
