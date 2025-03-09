#!/usr/bin/env bash

# simple script to change the project name

change_files=$(find . -type f -name 'CMakeLists.txt' -or -name '*.cmake' -or \
  -name '*.json' -or -name '*.cpp' -or -name '*.hpp')
# change the name to that of your new project
newname="tinydb"

for file in ${change_files}; do
  sed -i "s/template/${newname}/g" "${file}"
  sed -i "s/TEMPLATE/${newname^^}/g" "${file}"
done
