texture:texture.cc main.cc
	clang++ -std=c++17 -g -lglfw -framework OpenGL -Iglad/include main.cc texture.cc test.cc glad/src/glad.c -o build/texture
run:texture
	./build/texture
clear:
	rm build/texture