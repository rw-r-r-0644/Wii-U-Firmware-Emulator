import sys

assert len(sys.argv) == 4

indump = open(sys.argv[1], "r+b")

outimage = open(sys.argv[2], "w+b")
outspare = open(sys.argv[3], "w+b")

while True:
	chunk = indump.read(0x800)
	if len(chunk) != 0x800:
		break
	outimage.write(chunk)
	chunk = indump.read(0x040)
	if len(chunk) != 0x040:
		break
	outspare.write(chunk)