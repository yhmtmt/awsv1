#!/bin/sh

filter sample sample
caws gentbl sample sample
caws settbl sample sample -f ../fbs/sample.json
caws settblref sample sample sample
go
