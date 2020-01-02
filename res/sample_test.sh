#!/bin/sh

caws genfltr sample sample
caws gentbl sample sample
caws settbl sample sample -f ../fbs/sample.json
caws settblref sample sample sample
caws lstfltrs
caws run sample;
sleep 2
caws settbl sample sample -f ../fbs/sample2.json
sleep 2
caws delfltr sample
sleep 2
caws stop sample
caws delfltr sample
caws lstfltrs
caws deltbl sample
caws quit
