#!/usr/bin/env bash

HERE="$(dirname ${BASH_SOURCE[0]})"
ROOT=${HERE}/../..
SRC_DIR=${ROOT}/src
APIDOC_STATIC_DIR=${ROOT}/docs/apidocs_static

GEN_APIDOCSRC_DIR=${HERE}/gen_apidocsrc
GEN_APIDOCS_DIR=${HERE}/gen_apidoc

# Prepare docs
echo "Cleaning ${GEN_APIDOCSRC_DIR}"
rm -rf ${GEN_APIDOCSRC_DIR}
mkdir -p ${GEN_APIDOCSRC_DIR}

echo "Extracting docs..."
node ${HERE}/jsdoc_extractor.js -d ${SRC_DIR} -e .cpp -o ${GEN_APIDOCSRC_DIR}

echo "Copying static..."
cp -r ${APIDOC_STATIC_DIR}/* ${GEN_APIDOCSRC_DIR}/

echo "Generating docs..."
rm -rf ${GEN_APIDOCS_DIR}
mkdir -p ${GEN_APIDOCS_DIR}
jsdoc ${GEN_APIDOCSRC_DIR} ${GEN_APIDOCSRC_DIR}/index.md -r -u ${GEN_APIDOCSRC_DIR}/tutorials -c ${HERE}/conf.json -d ${GEN_APIDOCS_DIR}
echo "Finished generate docs: file://$(cd "$GEN_APIDOCS_DIR" && pwd)/index.html"
