CXX = g++
CXXFLAGS = -Wall -std=c++11 -D_DEFAULT_SOURCE -Wno-missing-braces -s -O2 -D_DEFAULT_SOURCE -I/usr/local/include -I./include -L. -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -DPLATFORM_DESKTOP
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

.PHONY: clean

game: $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

%.o: %.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) -c

clean:
	rm -f $(OBJS) game
