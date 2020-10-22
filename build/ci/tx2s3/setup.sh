#!/bin/bash

TRANSIFEX_USER=""
TRANSIFEX_PASSWORD=""
S3_KEY=""
S3_SECRET=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --tx_user) TRANSIFEX_USER="$2"; shift ;;
        --tx_password) TRANSIFEX_PASSWORD="$2"; shift ;;
        --s3_key) S3_KEY="$2"; shift ;;
        --s3_secret) S3_SECRET="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ -z "$TRANSIFEX_USER" ]; then echo "error: not set TRANSIFEX_USER"; exit 1; fi
if [ -z "$TRANSIFEX_PASSWORD" ]; then echo "error: not set TRANSIFEX_PASSWORD"; exit 1; fi
if [ -z "$S3_KEY" ]; then echo "error: not set S3_KEY"; exit 1; fi
if [ -z "$S3_SECRET" ]; then echo "error: not set S3_SECRET"; exit 1; fi

ENV_FILE=./../musescore_tx2s3_environment.sh
rm -f ${ENV_FILE}

echo "Install Qt (lrelease)"
qt_version="598"
QT_PATH="$HOME/Qt/${qt_version}"
mkdir -p "${QT_PATH}"
wget -q --show-progress -O qt5.zip "https://s3.amazonaws.com/utils.musescore.org/qt${qt_version}.zip"
7z x -y qt5.zip -o"${QT_PATH}"

export PATH=${QT_PATH}/bin:$PATH
echo export PATH="${QT_PATH}/bin:\${PATH}" >> ${ENV_FILE}

lrelease -version

echo "Install transifex-client" 
apt install python3-setuptools
pip3 install transifex-client

cat >~/.transifexrc <<EOL
[https://www.transifex.com]
hostname = https://www.transifex.com
password = ${TRANSIFEX_PASSWORD}
token =
username = ${TRANSIFEX_USER}
EOL

echo "tx version: $(tx --version)"

echo "Install s3cmd"
pip3 install s3cmd

cat >~/.s3cfg <<EOL
[default]
access_key = ${S3_KEY}
bucket_location = US
cloudfront_host = cloudfront.amazonaws.com
cloudfront_resource = /2010-07-15/distribution
default_mime_type = binary/octet-stream
delete_removed = False
dry_run = False
encoding = UTF-8
encrypt = False
follow_symlinks = False
force = False
get_continue = False
gpg_command = None
gpg_decrypt = %(gpg_command)s -d --verbose --no-use-agent --batch --yes --passphrase-fd %(passphrase_fd)s -o %(output_file)s %(input_file)s
gpg_encrypt = %(gpg_command)s -c --verbose --no-use-agent --batch --yes --passphrase-fd %(passphrase_fd)s -o %(output_file)s %(input_file)s
gpg_passphrase =
guess_mime_type = True
host_base = s3.amazonaws.com
host_bucket = %(bucket)s.s3.amazonaws.com
human_readable_sizes = False
list_md5 = False
log_target_prefix =
preserve_attrs = True
progress_meter = True
proxy_host =
proxy_port = 0
recursive = False
recv_chunk = 4096
reduced_redundancy = False
secret_key = ${S3_SECRET}
send_chunk = 4096
simpledb_host = sdb.amazonaws.com
skip_existing = False
socket_timeout = 300
urlencoding_mode = normal
use_https = False
verbosity = WARNING
EOL


echo "s3cmd version: $(s3cmd --version)"

echo "Setup done"