#!/bin/bash

#Coplien Cannonical

if [ $# -eq 0 ]  # "$#" is the number of arguments supplied 
then
	echo "Please provide class name as first argument"
	exit
fi

xIFS=$IFS	# Saving internal field separator


### Coplien Canonical class: header file

hppfile=$1".hpp"

COP="#ifndef CANON_H
# define CANON_H

class Canon
{
	public:

    	Canon(void);
    	Canon(Canon const & src);
    	~Canon(void);
		
		Canon &  operator=(Canon const & src);

	private:

};

#endif"


IFS=''		# Setting IFS to empty

echo -e $COP > $hppfile # -e flag interprets escape sequences

IFS=$xIFS  # Resetting IFS


sed -i '' s/Canon/$1/g $hppfile

UPPER=$(echo $1 | tr a-z A-Z)

sed -i '' s/CANON/$UPPER/g $hppfile

### Coplien Canonical class: cpp file

cppfile=$1".cpp"

COP='#include "Canon.hpp"

Canon::Canon(void) {}

Canon::Canon(Canon const & src)
{
    (void)src;
}

Canon::~Canon(void) {}

Canon &
Canon::operator=(Canon const & src)
{
    return (*this);
}'

IFS=''

echo -e $COP > $cppfile

IFS=$xIFS

sed -i '' s/Canon/$1/g $cppfile
