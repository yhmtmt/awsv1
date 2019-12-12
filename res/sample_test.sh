#!/bin/sh

caws genfltr sample sample
caws gentbl sample sample
caws settbl sample sample -f ../fbs/sample.json
caws settblref sample sample sample
caws run sample;
sleep 3
caws settbl sample sample -f ../fbs/sample2.json
sleep 3
caws delfltr sample
sleep 3
caws stop sample
caws delfltr sample
caws deltbl sample
caws quit
