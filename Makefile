SRC=$(wildcard src/*.cpp)
OBJECTS=$(SRC:.cpp=.o)

TEST_SRC=$(wildcard tests/*.cpp)
TEST_BIN=$(TEST_SRC:.cpp=)
CFLAGS+=--std=c++11 -g -Iinclude

TARGET=matsprak

all: $(OBJECTS) $(TEST_BIN) $(TARGET)
	@echo Klart

$(TARGET): all main.o
	g++ main.o $(OBJECTS) -o $(TARGET)
	
src/%.o: src/%.cpp include/*.h
	g++ $< -c -o $@ $(CFLAGS)
	
main.o: main.cpp $(OBJECTS)
	g++ main.cpp -c -o main.o $(CFLAGS)
	
tests/%: tests/%.cpp src/*.cpp include/*.h
	g++ $< $(OBJECTS) -o $@ -I./include $(CFLAGS)


