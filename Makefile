glad:
	glad --profile core --out-path ./glad --api gl=4.1 --generator c-debug

glfw/CMakeLists.txt:
	git submodule update --init --recursive --depth 1
