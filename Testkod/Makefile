TARGET=testbin
OBJECTS=simple.o

all: ${TARGET}

${TARGET}: main.cpp ${OBJECTS}
	g++ ${OBJECTS} -std=c++11 main.cpp -o testbin -lSDL2

simple.o simple.h: simple.mcl
	../matsprak simple.mcl
	


