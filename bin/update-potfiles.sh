#!/usr/bin/env bash

# update-potfiles
#   Generates a list of files which might contain translatable strings.
#
# Usage:
#   bin/update-potfiles > po/POTFILES.in

cd "$(dirname "$0")"/..

SRCDIRS=(
	client/Game2
	compilers/MazeCompiler
	compilers/ResourceCompiler
	engine
	)

echo '# List of source files which contain translatable strings.'
for srcdir in "${SRCDIRS[@]}"; do
	find "$srcdir" -type f -name '*.cpp' \
		-not -name 'StdAfx.*'
done | sort

