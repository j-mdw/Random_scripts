#!/bin/bash

## Has your password been pwned? Find out the secure way

## How to use?
## bash ispwned.sh <password>
## Requirements: shasum, curl, bash

HASH=$(echo -n $1 | shasum)

hash_5="${HASH:0:5}"
hash_35="${HASH:5:35}"

if ! ping -c1 api.pwnedpasswords.com &>/dev/null
then
	echo connection to pwnedpasswords failed, please try again later
	exit 1
fi

pwned=$(curl -s https://api.pwnedpasswords.com/range/$hash_5 | grep -i $hash_35)

if [ -z $pwned ]
then
	echo "You're safe"
else
	echo $pwned
	echo "You've been pwned, change password immediatly"
fi
