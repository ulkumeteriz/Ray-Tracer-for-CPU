files = image/*.cpp filemanip/*.cpp geometry/*.cpp scene/*.cpp
flags = -std=c++11 -ljpeg -O3
compiler = g++
all:
	$(compiler) $(files) main.cpp -o raytracer $(flags)

run:
	./main.out
