bin/hs main.cpp:
	mkdir -p bin

	c++ main.cpp -o bin/hs -std=c++2a

clean:
	rm -rf "bin/hs"

install:
	sudo cp -rf bin/hs /usr/bin/
