SRCS = ${shell ls *.cpp}
HEADERS = ${shell ls *.hpp}
OBJS = $(SRCS:.cpp=.o)
CXXFLAGS = -Wall -g -std=c++0x

.PHONY:all clean

all:demo

clean:
	rm -rf *.o demo

demo:$(OBJS) $(HEADERS)
	$(CXX) $(OBJS) -o $@
