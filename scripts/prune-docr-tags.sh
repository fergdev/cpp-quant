#!/usr/bin/env bash
set -euo pipefail

KEEP=5

list_sorted() {
  doctl registry repository list-tags cpp-quant \
    --format Tag,UpdatedAt --no-header |
    sort -k2 -r |
    awk '{print $1}'
}

echo "All tags (newest first):"
list_sorted | cat

BACKEND_TAGS=$(list_sorted | grep -E '^[0-9a-f]{7,40}$' || true)
UI_TAGS=$(list_sorted | grep -E '^ui-[0-9a-f]{7,40}$' || true)

prune_list() {
  local KEEP="$1"
  shift
  local i=0
  while read -r TAG; do
    [ -z "${TAG}" ] && continue
    i=$((i + 1))
    if [ "$i" -le "$KEEP" ]; then
      echo "Keeping: $TAG"
    else
      echo "Deleting: $TAG"
      doctl registry repository delete-tag cpp-quant "$TAG" --force
    fi
  done
}

echo "--- Pruning backend tags (keep $KEEP) ---"
echo "${BACKEND_TAGS}" | prune_list "$KEEP"

echo "--- Pruning UI tags (keep $KEEP) ---"
echo "${UI_TAGS}" | prune_list "$KEEP"
