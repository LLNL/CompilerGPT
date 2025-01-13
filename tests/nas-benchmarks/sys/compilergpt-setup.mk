.PHONY: setup
setup: setparams 

# setparams creates an npbparam.h file for each benchmark 
# configuration. npbparams.h also contains info about how a benchmark
# was compiled and linked

setparams: setparams.c ../config/make.def
	$(CC) -o setparams setparams.c

.PHONY: clean
clean:
	rm -f setparams
