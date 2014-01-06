SRC=src/mainloop.cpp src/timer.cpp
OBJS=$(SRC:.cpp=.o)
NAME=libeventcore
TARGET=$(NAME).a

all: $(TARGET)

$(TARGET): $(OBJS)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Wall -W -Iinclude --std=c++0x -c -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
