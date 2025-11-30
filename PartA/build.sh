if [ ! -d "bin" ]; then
    mkdir bin
else
	rm -f bin/*
fi

g++ -std=c++17 -g -O0 -I . -o bin/TaMarkingPartA TaMarking_101229043_101299841.cpp
