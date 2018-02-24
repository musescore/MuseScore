#!/bin/bash
#
# bintray-tidy.sh - delete old versions from a package hosted on Bintray.
#
# Note: There is no official quota for open source projects on Bintray, but
# they will suspend the account if they think you are taking advantage.

set -e # exit on error - don't want to accidentally delete the wrong version!
#set -x

function showHelp() {
cat <<EOF
                            ~~~ bintray-tidy.sh ~~~

bintray-tidy.sh - delete older versions from packages on Bintray to save space.

USAGE:    ./bintray-tidy.sh [OPTIONS] STRATEGY [[OWNER/]REPO/]PACKAGE

STRATEGIES:
  max-versions <num>    Keep <num> newest versions and delete the rest.
  max-days <num>        Delete all versions created more than <num> days ago.
  archive               Keep some older versions for archival purposes.

OPTIONS:
  -h, -?, --help        Show this help.
  -s, --simulate        Show what would be deleted without actually deleting it.

ENVIRONMENT:
  * BINTRAY_USER, BINTRAY_API_KEY - required
  * BINTRAY_REPO, BINTRAY_REPO_OWNER - optional

EXAMPLES:

 1) Keep the 10 most recent versions and delete everything else:

    $  ./bintray-tidy.sh  max-versions  10  MyRepo/CoolPackage

 2) Simulate running the archive strategy to see what would be deleted:

    $  ./bintray-tidy.sh  -s  archive  DreamTeam/TeamRepo/SharedPackage

                            ~~~ bintray-tidy.sh ~~~
EOF
}

main() {
  initialize_variables

  echo "$0 - delete old versions from a Bintray packages."
  echo "Package: '${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${package}'."

  get_remote_versions

  if [ "$arg_s" ]; then
    echo "SIMULATION: No versions will actually be deleted from the server."
  fi

  case "${strategy}" in
    max-versions )
      max_versions
      ;;
    max-days )
      max_days
      ;;
    archive )
      archive
      ;;
    * )
      fatal_error "invalid strategy '${strategy}'"
      exit 1
      ;;
  esac

  echo "Versions before: ${#REMOTE_VERSIONS[@]},\
 after: $((${#REMOTE_VERSIONS[@]}-$deleted_counter)),\
 deleted: $deleted_counter."

  [ "$arg_s" ] && echo "SIMULATION: Nothing was deleted.\
 Rerun without '-s' to actually delete versions." || true
}

function initialize_variables() {
  [ "$(which curl)" ] || { echo "$0: \`curl\` is not installed!" ; exit 1 ;}
  # Constants
  API="https://api.bintray.com"

  # Required environment variables
  BINTRAY_USER="${BINTRAY_USER:?Environment variable missing/empty!}" # env
  BINTRAY_API_KEY="${BINTRAY_API_KEY:?Environment variable missing/empty!}" # env

  # Optional environment variables
  BINTRAY_REPO_OWNER="${BINTRAY_REPO_OWNER:-$BINTRAY_USER}" # env, or use BINTRAY_USER
  BINTRAY_REPO="${BINTRAY_REPO:?Not specified and not set in environment.}" # env or CLI

  # Variables
  PCK_URL="${API}/packages/${BINTRAY_REPO_OWNER}/${BINTRAY_REPO}/${package}"
  CURL="curl -u${BINTRAY_USER}:${BINTRAY_API_KEY} \
         -H Content-Type:application/json \
         -H Accept:application/json"
  deleted_counter="0"
}

function get_remote_versions() {
  readarray -t REMOTE_VERSIONS < <(
    curl -X GET "${PCK_URL}" \
      | sed -nr 's|.*"versions":\["([^]]*)"\].*|\1|p' \
      | sed 's|","|\n|g' )
  echo "$0: ${#REMOTE_VERSIONS[@]} versions found on the remote server."
  [ "${#REMOTE_VERSIONS[@]}" != "0" ] || exit 1
}

function max_versions() {
  # Enforce upper limit on the number of versions:
  for ((i=0;i<${#REMOTE_VERSIONS[@]};i++)); do
    if [ "$i" -lt "$max_versions" ]; then
      print_action "Keeping" "${REMOTE_VERSIONS[$i]}"
    else
      delete_version "${REMOTE_VERSIONS[$i]}"
    fi
  done
}

function max_days() {
  # Delete versions that are more than ${max_days} days old:
  cuttoff_date_YMD="$(date --utc -d "$max_days days ago" +%Y%m%d)" # YYYYMMDD
  for ((i=0;i<${#REMOTE_VERSIONS[@]};i++)); do
    get_version_date "${REMOTE_VERSIONS[$i]}" || continue # skip version

    if [ "$version_date_YMD" -ge "$cuttoff_date_YMD" ]; then
      print_action "Keeping" "${REMOTE_VERSIONS[$i]}" "$created"
    else
      delete_version "${REMOTE_VERSIONS[$i]}" "$created"
    fi
  done
}

function archive() {
  # Delete old versions, but keep a few for archival purposes.
  # The algorithm that decides what to keep is hard-coded to prevent the stategy
  # being run with contradictory arguments that could have undesirable results.
  algorithm_last_changed="2016-01-01 00:00:00 UTC" # TODO: Keep this up-to-date!
  # No versions will be deleted that were created before an algorithm change.
  previous_version_date_YMD="" previous_version_date_Ywk="" # initialize values.
  for ((i=0;i<${#REMOTE_VERSIONS[@]};i++)); do
    get_version_date "${REMOTE_VERSIONS[$i]}" || continue # skip version

    [ "$version_date_YMD" -gt "$(date --utc -d "$algorithm_last_changed" +%Y%m%d)" ] || continue

    if [ "$version_date_YMD" -ge "$(date --utc -d "7 days ago" +%Y%m%d)" ]; then
      # Keep every version from last 7 days
      print_action "7D: Keeping" "${REMOTE_VERSIONS[$i]}" "$created"
    elif [ "$version_date_YMD" -ge "$(date --utc -d "1 month ago" +%Y%m%d)" ]; then
      # Keep maximum one version per day for past month
      if [ "$version_date_YMD" != "$previous_version_date_YMD" ]; then
        print_action "1pD: Keeping" "${REMOTE_VERSIONS[$i]}" "$created"
      else
        delete_version "${REMOTE_VERSIONS[$i]}" "$created"
      fi
    elif [ "$version_date_YMD" -ge "$(date --utc -d "3 months ago" +%Y%m%d)" ]; then
      # Keep maximum one version per week for past 3 months
      if [ "$version_date_Ywk" != "$previous_version_date_Ywk" ]; then
        print_action "1pW: Keeping" "${REMOTE_VERSIONS[$i]}" "$created"
      else
        delete_version "${REMOTE_VERSIONS[$i]}" "$created"
      fi
    else
      if [ "${version_date_YMD:0:6}" != "${previous_version_date_YMD:0:6}" ]; then
        # Maximum one version per month...
        if [ "$version_date_YMD" -ge "$(date --utc -d "1 year ago" +%Y%m%d)" ]; then
          # Keep it if less than a year old
          print_action "1pM: Keeping" "${REMOTE_VERSIONS[$i]}" "$created"
        elif [ "$((${version_date_YMD:4:2}%3))" == "0" ]; then
          # Otherwise only keep it every 3rd month. (Keep one every 3 months.)
          print_action "1p3M: Keeping" "${REMOTE_VERSIONS[$i]}" "$created"
        else
          delete_version "${REMOTE_VERSIONS[$i]}" "$created"
        fi
      else
        delete_version "${REMOTE_VERSIONS[$i]}" "$created"
      fi
    fi
    previous_version_date_YMD="$version_date_YMD"
    previous_version_date_Ywk="$version_date_Ywk"
  done
}

function get_version_date() {
  created="$(${CURL} -X GET "${PCK_URL}/versions/$1" 2>/dev/null | sed -nr \
's|.*"created":"([0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]{3}Z)".*|\1|p' )"
  [ "$created" ] || return 1 # failed to fetch "created" field
  version_date_YMD="$(date --utc -d "$created" +%Y%m%d)" # YYYYMMDD
  version_date_Ywk="$(date --utc -d "$created" +%Y%V)" # YYYY<weeknum>
  [ "$version_date_YMD" ] || return 1 # "created" field was wrong format
}

function print_action() {
  if [ "$3" ]; then
    echo "$1 version $2 ($(date --utc -d "$3" "+%c"))"
  else
    echo "$1 version $2"
  fi
}

function delete_version() {
  print_action "Deleting" "$1" "$2"
  deleted_counter="$(($deleted_counter+1))"
  [ "$arg_s" ] && return 0 || true
  ${CURL} -X DELETE "${PCK_URL}/versions/$1" &>/dev/null \
    || fatal_error "failed to delete $1"
}

function fatal_error() {
  echo "$0: Error: $1. Try '-h' for help." >&2
  exit 1
}

# Get arguments & parameters that were passed in on the command line
while [ "${1:0:1}" == "-" ]; do
  arg="${1:1}" # Could be passed in like: -i -e, -ie, or --argument-name
  while [ "${arg}" ]; do
    case "-${arg}" in
      -h*|-\?*|--help )
        showHelp && exit 0
        ;;
      -s*|--simulate|--just-print|--dry-run|--recon|--no-act )
        arg_s="true"
        ;;
      * )
        [ "-${arg:0:1}" == "--" ] || arg="${arg:0:1}"
        echo fatal_error "unknown option '-${arg}'"
        ;;
    esac
    [ "-${arg:0:1}" == "--" ] && arg="" || arg="${arg:1}" # pop 1st character
  done
  shift
done

# Get strategy (passed in on the command line)
strategy="$1" && shift || true
case "${strategy}" in
  max-versions )
    [ "$1" -eq "$1" ] 2>/dev/null || fatal_error "max-versions <int> requires integer"
    max_versions="$1" && shift
    ;;
  max-days )
    [ "$1" -eq "$1" ] 2>/dev/null || fatal_error "max-days <int> requires integer"
    max_days="$1" && shift
    ;;
  archive )
    ;;
  '' )
    fatal_error "missing strategy"
    ;;
  * )
    fatal_error "invalid strategy '${strategy}'"
    ;;
esac

# Get package (passed in on the command line)
[ "$1" ] || fatal_error "missing package"
package="$(sed -nr 's|^([^/]*/){0,2}([^/]*)$|\2|p' <<< "$1")" # required
repo="$(sed -nr 's|^([^/]*/){0,1}([^/]*)/[^/]*$|\2|p' <<< "$1")" # optional
owner="$(sed -nr 's|^([^/]*)/[^/]*/[^/]*$|\1|p' <<< "$1")" # optional
[ "${package}" ] || fatal_error "package '$1' badly formed"
[ "${repo}" ] && BINTRAY_REPO="${repo}" || true # override env
[ "${owner}" ] && BINTRAY_REPO_OWNER="${owner}" || true # override env

main "$@"
