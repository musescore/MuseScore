def pad(s; len): s | until(length >= len; . + " ");
to_entries | (map(.key | length) | max + 2) as $maxlen | .[] | (pad(.key; $maxlen) + .value.description)
