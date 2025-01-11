glad:
	python -m glad --profile core --out-path ./glad --api gl=4.1 --generator c-debug

glfw/CMakeLists.txt:
	git submodule update --init --recursive --depth 1 --remote

.PHONY: all

all: glew glew/src/glew.c glad glfw

glew:
	git submodule update --init --recursive --depth 1 --remote

glew/src/glew.c:
	cd glew/auto && make