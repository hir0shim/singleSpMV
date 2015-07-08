all: visualizer generator 
visualizer: visualizer.cpp
	icpc -std=c++11 visualizer.cpp -o visualizer
generator: generator.cpp
	icpc -std=c++11 generator.cpp -o generator

clean: 
	rm visualizer generator 
.PHONY: all clean
