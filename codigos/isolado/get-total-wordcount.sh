#!/bin/bash

set -euxo pipefail

ls s* | parallel -j 4 'curl --silent --data-binary "@{}" -o "{}.json" http://localhost:8080'

jq -r '.word_counts | to_entries[] | [.key, .value] | @tsv' s*.json \
        | awk '{ s[$1] += $2 } END { for (k in s) printf "%s: %d\n", k, s[k] }' \
        | sort \
        | tee result

rm s*.json
