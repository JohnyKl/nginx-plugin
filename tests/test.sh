#!/bin/bash 

# operating system type
OS=$(. /etc/os-release; echo "$ID $ID_LIKE")

# compilation and assembly configuration
CXX="g++"
OPTIMIZATION="-g -O0 -fpermissive -D_TESTS=1"
STD_MODE="-std=c++0x"
LDFLAGS="-Wl,-rpath,/usr/local/lib/gcc5/"
SOURCES="test_main.cpp \
        server/HttpMockServer.cpp \
        ../templarbit/core.c \
        ../templarbit/handler.c \
        ../templarbit/http.c \
        ../templarbit/list.c"
LIBS="-lpthread -lboost_system -lboost_filesystem -lcunit -lcurl -ljansson"
LINKER="-L/usr/local/lib/"
EXECUTABLE="templarbit-plugin-tests"
        
# colors definition
red() { tput setaf 1; }
green() { tput setaf 2; }
res() { tput sgr0; }

echo
echo "* Current OS group is: $OS"

echo "`green`* Setting up build dependencies `res`"
if [[ $OS == *"debian"* ]]; then
   apt-get update
   apt-get install libcurl4-gnutls-dev libcunit1-dev libboost-all-dev
elif [[ $OS == *"rhel"* ]]; then
   rpm -Uvh http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-10.noarch.rpm
   yum install boost-devel curl-devel CUnit-devel jansson-devel
else
   echo "`red`* Error`res`: current OS is not supported"
   exit 1
fi

# checking dependencies
if [[ -z "`ldconfig -p | grep libjansson`" ]]; then
   echo "`red`* Error`res`: Before running tests, you must build plugin first by running ../build.sh"
   exit 2
fi

# cleaning up
echo
echo "`green`* Cleaning up from previous run `res`"
rm -f $EXECUTABLE

# building tests
echo
echo "`green`* Building test suite `res`"
$CXX $OPTIMIZATION $STD_MODE $LINKER $LDFLAGS $SOURCES $LIBS -o $EXECUTABLE

if [[ ! -e "$EXECUTABLE" ]]; then
   echo "`red`* Test suite has not been built. Check build errors above `res`"
fi

echo "`green`* Tests suite has been built successfully."

# running tests
echo "`green`* Running test suite `res`"
./$EXECUTABLE

