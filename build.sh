gcc $1 main.c nanovg/build/libnanovg.a	\
	-Inanovg/src	\
	-lm -lX11 -lXcomposite -lGL -lGLEW	\
	-o Ellington
