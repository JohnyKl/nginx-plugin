#!/bin/bash 
OUT_DIR=../
NGINX_VER=$1
NGINX_DWL_URL=http://nginx.org/download/$NGINX_VER.tar.gz
OS_COMMAND=yum

if [ "$(. /etc/os-release; echo $NAME)" = "Ubuntu" ]; then
	OS_COMMAND=apt-get
	sudo $OS_COMMAND install wget
	sudo $OS_COMMAND install libboost-all-dev
	sudo $OS_COMMAND install libcurl4-gnutls-dev
	sudo $OS_COMMAND install libcunit1-dev
	sudo $OS_COMMAND install libpcre3 libpcre3-dev

else
	$OS_COMMAND install -y httpd-devel pcre perl pcre-devel zlib zlib-devel GeoIP GeoIP-devel

	sudo $OS_COMMAND install boost-devel

	$OS_COMMAND install curl-devel

	sudo $OS_COMMAND install wget

	wget http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-10.noarch.rpm -P ./
	rpm -Uvh epel-release*rpm
	$OS_COMMAND install CUnit-devel
fi
	sudo $OS_COMMAND install libtool
	sudo $OS_COMMAND install autoconf
	sudo $OS_COMMAND install automake
	sudo $OS_COMMAND install autoheader
	sudo $OS_COMMAND install unzip
	mkdir vendor/
	wget https://github.com/templarbit/jansson/archive/master.zip -O ./vendor/jansson_src.zip
	cd vendor/
	unzip ./jansson_src.zip
	mv ./jansson-master ./jansson
	rm -f jansson_src.zip
	cd jansson/
	autoconf
	libtoolize --force
	aclocal
	autoheader
	automake --force-missing --add-missing
	autoconf
	./configure
	make
	make install
	cd ../../	

mkdir -p $OUT_DIR
wget $NGINX_DWL_URL -P $OUT_DIR
tar -xzf $OUT_DIR$NGINX_VER.tar.gz -C $OUT_DIR
rm $OUT_DIR$NGINX_VER.tar.gz
cd $OUT_DIR$NGINX_VER/
./configure --add-dynamic-module=../nginx-plugin --prefix=/opt/nginx --with-debug
make
