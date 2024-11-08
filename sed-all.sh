#!/bin/sh

# this thing took me 2 hours to get all the regex working correctly my goodness

oldname="myproj"

# change this.
# and if you want to change another time, change newname to the current oldname,
# then change newname to the name you want.
newname="tinydb"

# read .gitignore,
# filter to include only ignored directories,
# then modify them into regex strings
invalid_paths=$(cat .gitignore | grep -E ".*\*" \
    | sed -E 's/\//\\\//g' | sed -E 's/\*/\.\*/g' | sed -E 's/^/.*\\\//g')

invalid_cmd=(-and -not -regex ".\/\.git\/.*" -and -not -regex "\.\/\.git\/.*")

# modify the invalid paths into regex to be passed into `find`
for pth in ${invalid_paths[@]}; do
    invalid_cmd+=(-and -not -regex "$pth")
done

# echo ${invalid_paths[@]}

for cmake_file in `find . -type f -regextype egrep -regex \
    ".*\/(.*\.cmake$|CMakeLists.txt$)" \
    ${invalid_cmd[@]}`; do
    # echo $cmake_file
    sed -i "s/$oldname/$newname/g" $cmake_file
done

# add more file extensions according to what you use.
# currently configured:
# .c
# .C
# .cc
# .cpp
# .cxx
# .c++
# .ccm
# .cppm
# .cxxm
# .c++m
# .h
# .H
# .hh
# .hpp
# .hxx
# .h++
# .ixx
# .ixxm
for source_file in `find . -type f -regextype egrep -regex \
    ".*\.(ixx(m)?|c((c|xx|pp|\+\+)m?)?|h(h|pp|xx|\+\+)?|C|H)$" \
     ${invalid_cmd[@]}`; do
    # echo $source_file
    sed -i "s/${oldname^^}/${newname^^}/g" $source_file
done

cat << EOF
All file contents have had "$oldname" replaced by "$newname".
EOF
