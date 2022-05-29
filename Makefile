all: bfcompile bfinter

bfcompile: bfcompile.c
	$(CC) -o $@ $<

bfinter: bfinter.c
	$(CC) -o $@ $<

clean:
	-rm bfcompile bfinter

.PHONY: all clean
