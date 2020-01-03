#!/bin/sh
caws genfltr sample fltr1
caws genfltr sample fltr2
caws gentbl sample tbl1
caws gentbl sample tbl2
caws settbl sample tbl1 -f ../fbs/sample.json
caws settbl sample tbl2 -f ../fbs/sample2.json
caws gench sample ch1
caws gench sample ch2
caws settblref tbl1 fltr1 tbl
caws settblref tbl2 fltr2 tbl
caws setfltrpar fltr1 ch ch1
caws setfltrpar fltr2 ch ch2
caws getfltrpar fltr1 ch
caws getfltrpar fltr2 ch
caws lstfltrs
caws lsttbls
caws lstchs
caws run fltr1;
caws run fltr2;
sleep 2
caws settbl sample tbl1 -f ../fbs/sample2.json
caws settbl sample tbl2 -f ../fbs/sample.json
sleep 2
caws delfltr fltr1
caws delfltr fltr2
sleep 2
caws stop fltr1
caws stop fltr2
caws delfltr fltr1
caws delfltr fltr2
caws lstfltrs
caws lsttbls
caws lstchs
caws deltbl tbl1
caws deltbl tbl2
caws quit
