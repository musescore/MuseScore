#!/bin/python
import fileinput
import sys

inFile = 'mscore/loginmanager.cpp'


def encode(inString):
    result = ""
    for i, c in enumerate(inString):
        # add a musescore tab at the beginning of the line
        if i % 4 == 0:
            result += "      "
        result += 'ba[' + str(i) + '] = 0x' + format(ord(c), '02x') + ';'
        if (i+1) % 4 == 0:
            if i != (len(inString) - 1):
                result += "\n"
        else:
            result += " "
    return result


def processFile(consumerKey, consumerSecret):
    doneKey = False
    doneConsumer = False
    omit = 0
    for line in fileinput.input(inFile, inplace=1):
        if line.strip().startswith("ba.resize(32)"):
            if not doneKey:
                print line.rstrip("\n")
                print encode(consumerKey)
                doneKey = True
                omit = 9

        if line.strip().startswith("_consumerKey"):
            if not doneConsumer:
                print line.rstrip("\n")
                print encode(consumerSecret)
                doneConsumer = True
                omit = 9

        if omit <= 0:
            print line.rstrip("\n")
        omit = omit - 1
    if doneKey and doneConsumer:
        print "Keys successfully added"


if __name__ == '__main__':
    if len(sys.argv) != 3:
        exit()
    consumerKey = sys.argv[1]
    consumerSecret = sys.argv[2]
    if len(consumerKey) != 32:
        print "Wrong size of consumerKey"
        exit()
    if len(consumerSecret) != 32:
        print "Wrong size of consumerSecret"
        exit()
    processFile(consumerKey, consumerSecret)

