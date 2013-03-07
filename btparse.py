import sys

def parse():    
    keys = ["", "", "", "", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", 
"L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", 
         "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"];


    for line in sys.stdin:
        if 'A1' in line:
            byte = str.replace("^A1|01|00|\s", "", line)
            print byte, keys[int(byte)]


if __name__ == '__main__':
    parse()
