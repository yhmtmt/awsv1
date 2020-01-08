#!/bin/sh
caws genfltr sample fltr1
caws genfltr sample fltr2
caws gentbl sample tbl1
caws gentbl sample tbl2
caws settbl sample tbl1 -f ../fbs/sample.json
caws settbl sample tbl2 -f ../fbs/sample2.json
caws gettbl tbl1
caws gettbl tbl2
caws gench sample ch1 
caws gench sample ch2
caws settblref tbl1 fltr1 tbl
caws settblref tbl2 fltr2 tbl
caws setfltrpar fltr1 ch ch1 f64 1.0 u64 2 s64 3 f32 4.0 u32 5 s32 6 s16 7 u16 8 s8 9 u8 10 b n str hello e Bar
caws setfltrpar fltr2 ch ch2 f64 11.0 u64 12 s64 13 f32 14.0 u32 15 s32 16 s16 17 u16 18 s8 19 u8 110 b y str world e Bar
caws getfltrpar fltr1
caws getfltrpar fltr2
caws getfltrpar fltr1 ch f64 u64 s64 f32 u32 s32 s16 u16 s8 u8 b str e
caws getfltrpar fltr2 ch f64 u64 s64 f32 u32 s32 s16 u16 s8 u8 b str e
caws setfltrinchs fltr1 ch1 ch2
caws setfltroutchs fltr1 ch2 ch1
caws getfltrinchs fltr1
caws getfltroutchs fltr1
caws lstfltrs
caws lsttbls
caws lstchs
caws run fltr1;
caws run fltr2;
sleep 2
caws settbl sample tbl1 -f ../fbs/sample2.json
caws settbl sample tbl2 -f ../fbs/sample.json
caws gettbl tbl1
caws gettbl tbl2
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
