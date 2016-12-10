CC = g++
CFLAGS = -std=c++11
LFLAGS = -lsfml-window -lsfml-graphics -lsfml-system -L/usr/local/lib/ -lboost_filesystem
NAME = solar
OBJ = Main.o Galaxy.o

compile: ${OBJ} EXE

runcompile: ${OBJ} EXE clean
	./${NAME}

Main.o: Main.cpp
	${CC} -c Main.cpp ${CFLAGS}

Galaxy.o: Galaxy.cpp
	${CC} -c Galaxy.cpp ${CFLAGS}

EXE: ${OBJ}
	${CC} -o ${NAME} ${OBJ} ${LFLAGS}

clean:
	rm *.o
