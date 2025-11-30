if [ ! -d "bin" ]; then
    mkdir bin
else
	rm bin/*
fi

g++ -std=c++17 -g -O0 -I . -o bin/TaMarkingPartB PartB/TaMarking.cpp