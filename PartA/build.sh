if [ ! -d "bin" ]; then
    mkdir bin
else
	rm bin/*
fi

g++ -std=c++17 -g -O0 -I . -o bin/TaMarkingPartA PartA/TaMarking_101229043_101299841.cpp
