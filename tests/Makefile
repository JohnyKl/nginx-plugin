CXX=g++
OPTIMIZATION=-g -O0 -fpermissive -D_TESTS=1
STD_MODE=-std=c++0x
LDFLAGS=-Wl,-rpath,/usr/local/lib/gcc5/
SOURCES=test_main.cpp server/HttpMockServer.cpp ../templarbit/core.c \
../templarbit/handler.c ../templarbit/http.c ../templarbit/list.c
LIBS=-lpthread -lboost_system -lboost_filesystem -lcunit -lcurl -ljansson  
INCLUDES=
LINKER=-L/usr/local/lib/
EXECUTABLE=nginx-plugin-tests

all:
	${CXX} ${OPTIMIZATION} ${STD_MODE} ${INCLUDES} ${LINKER} ${LDFLAGS} \
	${SOURCES} \
	${LIBS} \
	-o ${EXECUTABLE}
	
clean:
	rm -rf ${EXECUTABLE}
