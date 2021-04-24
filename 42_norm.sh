#!/bin/bash


# 42 NORM:

# Requirements:
## vim
## 42 header
## vimrc tabstop=4 (if you want tabs to be 4 spaces)

# changes 4 consecutive spaces for tabs
# adds 42 header (if installed) on all '.c' files

for file in *.c; do
	[ -f "$file" ] || continue
	vim $file -c 'retab!' -c 'Stdheader' -c 'wq'
done

