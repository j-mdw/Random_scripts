#!/bin/bash

#Coplien Cannonical

if [ $# -eq 0 ]  # "$#" is the number of arguments supplied 
then
	echo "Please provide class name as first argument"
	exit
fi

### Coplien Canonical class: header file

hppfile=$1".hpp"

COP="#ifndef CANON_H\n
# define CANON_H\n
\n
class Canon {\n
\n
public:\n
\n
    \tCanon(void);\n
    \tCanon(Canon const & src);\n
    \t~Canon(void);\n
\n
    \tCanon &  operator=(Canon const & src);\n
\n
private:\n
\n
};\n
\n
#endif"

echo -e $COP > $hppfile # -e flag interprets escape sequences

sed -i '' s/Canon/$1/g $hppfile

UPPER=$(echo $1 | tr a-z A-Z)

sed -i '' s/CANON/$UPPER/g $hppfile

### Coplien Canonical class: cpp file

cppfile=$1".cpp"

COP='#include "Canon.hpp"\n
\n
Canon::Canon(void) {}\n\n
Canon::Canon(Canon const & src)\n
{\n
    \t(void)src;\n
}\n
\n
Canon::~Canon(void) {}\n
\n
Canon &\n
Canon::operator=(Canon const & src)\n
{\n
    \treturn (*this);\n
}\n'

echo -e $COP > $cppfile

sed -i '' s/Canon/$1/g $cppfile
