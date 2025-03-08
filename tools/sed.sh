#!/usr/bin/env bash

# simple script to change the project name

cmake_files=$(find . -type f -name 'CMakeLists.txt' -or -name '*.cmake' -or -name '*.json')
# change the name to that of your new project
newname="myproj"

for file in ${cmake_files}; do
  sed -i "s/template/${newname}/g" "${file}"
  sed -i "s/TEMPLATE/${newname^^}/g" "${file}"
done
