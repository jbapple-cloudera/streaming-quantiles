DEBUG_FLAGS = -O0 -ggdb3
RELEASE_FLAGS = -O3 -DNDEBUG

.PHONY: all
CPPS = $(wildcard *.cpp)
all: $(subst cpp,debug.exe,$(CPPS)) $(subst cpp,release.exe,$(CPPS))

%.debug.exe: %.cpp *.hpp Makefile
	$(CXX) $(DEBUG_FLAGS) --std=c++14 -o $@ $<

%.release.exe: %.cpp *.hpp Makefile
	$(CXX) $(RELEASE_FLAGS) --std=c++14 -o $@ $<
