#!/bin/sh
#
# Copy FFmpeg headers for MuseScore - extracts minimal needed headers.
# Run from FFmpeg source root. Output: list of header paths to copy.
#

ROOT_INCLUDES="libavcodec/avcodec.h
libavformat/avformat.h
libavutil/mathematics.h
libavutil/rational.h
libavutil/avstring.h
libavutil/opt.h
libavutil/mem.h
libswscale/swscale.h
libavutil/imgutils.h
libswresample/swresample.h"

NL='
'

# Resolve include path: if contains "/" use as-is, else prepend parent dir of $2
resolve_include() {
    inc=$(echo "$1" | tr -d ' \t')
    from="$2"
    [ -z "$inc" ] && return 1
    if echo "$inc" | grep -q '/'; then
        echo "$inc"
    else
        parent="${from%/*}"
        [ -n "$parent" ] && echo "${parent}/${inc}" || echo "$inc"
    fi
}

# Check if line is in list (newline-separated)
line_in_list() {
    line="$1"
    list="$2"
    echo "$list" | grep -qFx "$line" 2>/dev/null
}

# Collect all headers recursively
collect_headers() {
    root="$1"
    collected=""
    to_process="$ROOT_INCLUDES"

    while [ -n "$to_process" ]; do
        next_process=""
        for rel_path in $to_process; do
            if line_in_list "$rel_path" "$collected"; then
                continue
            fi
            collected="${collected}${collected:+$NL}$rel_path"
            full_path="${root}/${rel_path}"
            if [ ! -f "$full_path" ]; then
                continue
            fi
            # Extract #include "..." (local includes only)
            while IFS= read -r line; do
                inc=$(echo "$line" | sed -n 's/^[[:space:]]*#include[[:space:]]*"\([^"]*\)".*/\1/p')
                [ -z "$inc" ] && continue
                resolved=$(resolve_include "$inc" "$rel_path")
                [ -z "$resolved" ] && continue
                if ! line_in_list "$resolved" "$collected" && ! line_in_list "$resolved" "$next_process"; then
                    next_process="${next_process}${next_process:+$NL}$resolved"
                fi
            done < "$full_path"
        done
        to_process="$next_process"
    done
    echo "$collected"
}

root=$(pwd)
headers=$(collect_headers "$root")
echo "$headers" | sort -u
