#!/usr/bin/env sh
xmlstarlet sel --template --value-of "//translation[not(@type)]" "$1"
echo ""
