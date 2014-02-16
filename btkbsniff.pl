#!/usr/bin/perl

@keys =
("", "", "", "",
"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
"1", "2", "3", "4", "5", "6", "7", "8", "9", "0");

use IO::Handle;

open("BT", "hcidump -x |") or die("Can't start hcidump");

while($line = ){
    if($line =~ m/\s+A1/) {
        @bytes = split(/ /, $line);
        $code = hex($bytes[10]);

        if($code != 0) {
            printf("%02X\t%s\n", $code, $keys[$code]);
        }
    }
}

close("BT");
