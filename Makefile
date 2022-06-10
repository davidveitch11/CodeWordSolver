GCC=gcc
LINKS=-l ws2_32

all : makeData parserTest patternFinderTest readTest solver

clean :
	rm *.exe *.o
	rm makeData parserTest patternFinderTest readTest solver

#

makeData : makeData.o
	$(GCC) makeData.o -o makeData $(LINKS)

parserTest : parserTest.o puzzel.o data.o
	$(GCC) parserTest.o puzzel.o data.o -o parserTest

patternFinderTest : patternFinderTest.o data.o dataStore.o
	$(GCC) patternFinderTest.o data.o dataStore.o -o patternFinderTest $(LINKS)

readTest : readTest.o dataStore.o
	$(GCC) readTest.o dataStore.o -o readTest $(LINKS)

solver : solver.o puzzel.o data.o dataStore.o
	$(GCC) solver.o puzzel.o data.o dataStore.o -o solver $(LINKS)

#

makeData.o : makeData.c makeData.h
	$(GCC) makeData.c -c

parserTest.o : parserTest.c puzzel.h
	$(GCC) parserTest.c -c

puzzel.o : puzzel.c puzzel.h data.h dataStore.h
	$(GCC) puzzel.c -c

data.o : data.c data.h dataStore.h
	$(GCC) data.c -c

dataStore.o : dataStore.h dataStore.c
	$(GCC) dataStore.c -c

patternFinderTest.o : patternFinderTest.c data.h dataStore.h
	$(GCC) patternFinderTest.c -c

readTest.o : readTest.c dataStore.h
	$(GCC) readTest.c -c

solver.o : solver.c
	$(GCC) solver.c -c
