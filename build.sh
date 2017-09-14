#!/bin/bash 

NGINX_VER=$1

# nginx download URL
NGINX_URL=http://nginx.org/download/nginx-$NGINX_VER.tar.gz

# jansson donwload URL
JANSSON_URL=https://github.com/templarbit/jansson/archive/master.zip

# operating system type
OS=$(. /etc/os-release; echo "$ID $ID_LIKE")

# shared library location after build is completed
SO="vendor/nginx/objs/ngx_http_templarbit_csp_module.so"

# colors definition
red() { tput setaf 1; }
green() { tput setaf 2; }
res() { tput sgr0; }

if [[ -z "$NGINX_VER" ]]; then
   echo "`red`Error`res`: Version must be specified"
   echo 
   echo "Syntax: `basename $0` nginx-version"
   echo "Example: ./`basename $0` 1.11.1"
   echo 
   exit 1
fi

echo
echo "* Current OS group is: $OS"

echo
echo "`green`* Setting up build dependencies `res`"
if [[ $OS == *"debian"* ]]; then
        apt-get update
	apt-get install wget \
                        libboost-all-dev \
                        libcurl4-gnutls-dev \
                        libcunit1-dev \
                        libpcre3 \
                        libpcre3-dev \
                        libtool \
                        autoconf \
                        automake \
                        autoheader \
                        unzip
elif [[ $OS == *"rhel"* ]]; then
	rpm -Uvh http://dl.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-10.noarch.rpm
	yum install httpd-devel \
                    pcre \
                    perl \
                    pcre-devel \
                    zlib \
                    zlib-devel \
                    GeoIP \
                    GeoIP-devel \
                    boost-devel \
                    CUnit-devel \
                    curl-devel \
                    wget \
                    libtool \
                    autoconf \
                    automake \
                    autoheader \
                    unzip
else
        echo "`red`* Error$NC: current OS is not supported`res`"
        exit
fi

# cleaning up
echo
echo "`green`* Cleaning up from previous run `res`"
rm -Rf vendor/{jansson,nginx}

# getting jansson
echo
echo "`green`* Retrieving jansson sources `res`"
mkdir -p vendor/jansson
wget $JANSSON_URL -O ./vendor/jansson_src.zip
( \
  cd vendor/; \
  unzip ./jansson_src.zip; \
  mv ./jansson-master/* ./jansson/; \
  rm -Rf ./jansson-master jansson_src.zip \
)

# building jansson
echo
echo "`green`* Building jansson`res`"
( \
  cd vendor/jansson/; \
  autoreconf -i; \
  ./configure; \
  make; \
  make install \
)

# getting nginx
echo
echo "`green`* Retrieving nginx sources`res`"
mkdir -p vendor/nginx
wget $NGINX_URL -O vendor/nginx-$NGINX_VER.tar.gz
( \
  cd vendor/; \
  tar -zxf ./nginx-$NGINX_VER.tar.gz -C nginx/ --strip-components=1; \
  rm -f $NGINX_VER.tar.gz \
)

# building nginx
echo
echo "`green`* Building nginx with the plugin`res`"
( \
  cd vendor/nginx; \
  ./configure --add-dynamic-module=../.. --prefix=/opt/nginx --with-debug; \
  make \
)

echo
if [[ -e "$SO" ]]; then
   echo "`green`* Plugin has been built successfully. Location: $SO `res`"
   file $SO
else
   echo "`red`* Plugin has not been built. Check build errors above `res`"
fi

echo

