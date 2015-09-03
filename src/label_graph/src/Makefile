CC= g++
CCFLAGS=  -g -O3 -std=c++11 -Wall -pedantic
LDFLAGS= -lboost_program_options -lpq
INCFLAGS= -I /usr/include/postgresql/9.4/server/ -I /usr/include/postgresql/
SOURCES= labelGraph_using_importFile_with_minimul_code_4.cpp

OBJECTS= $(SOURCES:.cpp=.o)
TARGET= labelGraph_using_importFile_with_minimul_code_4

all: $(OBJECTS) $(TARGET)

$(TARGET): labelGraph_using_importFile_with_minimul_code_4.o
	$(CC) -o $@ $^ $(LDFLAGS) 

$(TARGET).o: $(TARGET).cpp
	$(CC)  $(CCFLAGS) $(INCFLAGS) -c $<

%.o: %.cpp %.h
	$(CC)  $(CCFLAGS) $(INCFLAGS) -c $<

%.o: %.cpp %.hpp
	$(CC)  $(CCFLAGS) $(INCFLAGS) -c $<

valgrind: labelGraph_using_importFile_with_minimul_code_4
	valgrind -v --track-origins=yes --undef-value-errors=yes --leak-check=full --show-leak-kinds=all ./mainP &> ./valgrindlog.txt
clean:
	rm -f *.o $(TARGET)