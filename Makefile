.PHONY: all clean

all: glew glew/src/glew.c glad glfw build venv/lib/glad
	cmake --build build --target run

build:
	mkdir -p build
	cmake -B build .

glad:
	venv/bin/python -m glad --profile core --out-path ./glad --api gl=4.1 --generator c-debug

glfw/CMakeLists.txt:
	git submodule update --init --recursive --depth 1 --remote

glew: venv/lib/glad
	git submodule update --init --recursive --depth 1 --remote

glew/src/glew.c: glew/auto
	cd glew/auto && make

glew/auto:
	git submodule sync --recursive
	git submodule update --init --recursive --depth 1 --remote --force

venv:
	python -m venv venv

clean:
	rm -rf build

venv/lib/glad : venv
	venv/bin/python -m pip install glad