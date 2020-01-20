#!/bin/bash

function assert()
{
    if [ $1 != 0 ]; then
	echo "$2"
	exit $1
    fi
}

function nassert()
{
    if [ $1 = 0 ]; then
	exit $1
    fi
}
