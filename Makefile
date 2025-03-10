.PHONY: all clean

all: glew glew/src/glew.c glad glfw build
	cmake --build build --target run

build:
	mkdir -p build
	cmake -B build .

glad:
	python -m glad --profile core --out-path ./glad --api gl=4.1 --generator c-debug

glfw/CMakeLists.txt:
	git submodule update --init --recursive --depth 1 --remote

glew:
	git submodule update --init --recursive --depth 1 --remote

glew/src/glew.c:
	cd glew/auto && make

clean:
	rm -rf build