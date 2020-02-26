
.PHONY: build help sketch clean

build:
	cd libcscript      && make
	cd libfst          && make
	cd test-libfst     && make
	cd isbn-xml-to-fst && make
	cd cmd             && make

help:
	@echo make help
	@echo make sketch
	@echo make clean

sketch:
	tree-files --no-vcs | grep -v '/OLD/' | grep -v 'uftrace\.data' | grep -v '/tmp/' | grep -v '\.o$$'

clean:
	cd libcscript      && make clean
	cd libfst          && make clean
	cd test-libfst     && make clean
	cd isbn-xml-to-fst && make clean
	cd cmd             && make clean
