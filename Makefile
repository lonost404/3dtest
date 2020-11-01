PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJS = 3dtest.o
LDFLAGS = -lGLEW -lGL -lglfw  -std=c++17

ifeq ($(BUILD_MODE),debug)
	CFLAGS += -g
else ifeq ($(BUILD_MODE),run)
	CFLAGS += -O2 
else
	#$(error Build mode $(BUILD_MODE) not supported by this Makefile)
endif

all:	3dtest

3dtest:	$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o:	$(PROJECT_ROOT)/src/%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)/src/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr 3dtest $(OBJS)
