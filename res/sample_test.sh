#!/bin/sh

caws genfltr sample sample
caws gentbl sample sample
caws settbl sample sample -f ../fbs/sample.json
caws settblref sample sample sample
go
sleep 10
caws settbl sample sample -f ../fbs/sample2.json
sleep 10
stop
caws delfltr sample
caws deltbl sample
quit
