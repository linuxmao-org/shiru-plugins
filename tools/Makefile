# Tools

CXX = g++
CXXFLAGS = -O2 -g

all: bin/ConvertChipWaveFxb

clean:
	rm -rf bin

bin/ConvertChipWaveFxb: ConvertChipWaveFxb.cpp
	@install -d $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ -I../dpf/distrho -I../plugins/chipwave
