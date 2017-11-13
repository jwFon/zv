CC = g++
CXXFLAGS = -std=c++11 -Wall -O2 -g
SOURCE_FILES = zv.o util.o event.o client.o cmd.o t_string.o

kv_server : $(SOURCE_FILES)
	$(CC) -o kv_server $(SOURCE_FILES) $(CXXFLAGS)

%.o: %.cpp %.h
	$(CXX) -c $(CXXFLAGS) -MMD -o $@ $<

clean:
	rm *.d
	rm *.o
	rm kv_server
