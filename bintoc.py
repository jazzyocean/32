import sys

with open(sys.argv[1], "rb") as inf:
    with open("emu/bootrom.c", "w") as outf:
        outt = "unsigned char bootrom[512] = {"
        for b in inf.read():
            outt += str(b) + ","
        outt += "};"
        outf.write(outt)
