VERSION_TAG := $(shell git describe --always --tags --abbrev=0)
COMMIT_HASH := $(shell git rev-parse --short HEAD)
OS_INFO := $(shell uname -rmo)

bin/hs main.cpp:
	mkdir -p bin

	c++ main.cpp hs/parser/parser.cpp -o bin/hs -std=c++2a -g \
		-DOS_VERSION="$(OS_INFO)" \
		-DHS_VERSION="$(VERSION_TAG)" \
		-DHS_COMMIT_HASH="$(COMMIT_HASH)"
clean:
	rm -rf "bin/hs"

install:
	sudo mkdir -p /usr/include/hs/std
	sudo cp -rf std/ /usr/include/hs/
	sudo cp -rf bin/hs /usr/bin/