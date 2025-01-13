# from the default Makefile
BENCHMARK:=bt
BENCHMARKU:=BT

# from Faros setup 
CLASS:=A

.PHONY: setup
setup: npbparams.h

npbparams.h: ../config/make.def
	@ echo make.def modified. Rebuilding npbparams.h just in case
	rm -f npbparams.h
	../sys/setparams ${BENCHMARK} ${CLASS}

.PHONY: clean
clean:
	rm -f npbparams.h

