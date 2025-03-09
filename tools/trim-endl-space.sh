#!/usr/bin/env bash

curr_branch=${1:-$(git branch --show-current)}
files=$(git ls-tree --name-only -r "${curr_branch}")

for f in ${files}; do
  sed -i 's/ *$//g' "${f}"
done
