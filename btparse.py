import sys

def parse():    
    keys = ["", "", "", "", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", 
"L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", 
         "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"];


    for line in sys.stdin:
        if 'A1' in line:
            byte = line.split(" ")
            #print byte, keys[hex(byte)]
            index = int(byte[10], 16)
            if index < len(keys) and index != 0: 
                print byte[10], keys[index]

if __name__ == '__main__':
    parse()
