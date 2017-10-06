# nginx-plugin

[![Build Status](https://travis-ci.org/JohnyKl/nginx-plugin.svg?branch=feature%2Funit-tests)](https://travis-ci.org/JohnyKl/nginx-plugin)

## Usage

### Build

Execute `build.sh` script from the directory containing the plugin. Pass your target nginx version as a parameter to the script.

```bash
$ ./build.sh 1.11.1
```

Build script produces shared library "ngx_http_templarbit_csp_module.so" that needs to be added to nginx modules folder

### Deploy

Shared library produced by the ```build.sh``` script is compatible with nginx 1.11.1+ versions.
In order to integrate it next steps have to be followed:

1. Add instruction to load shared library to the top of your nginx.conf
```
load_module "modules/ngx_http_templarbit_csp_module.so";
```

2. Configure server instance and locations

templarbit_token, templarbit_property_id and templarbit_fetch_interval parameters are server instance level only.
In order to enable headers injection, "templarbit_csp_enable on" directive must be added to the corresponding location(s).

Example is shown below

```
server {
   listen       80;
   server_name  example.com;

   templarbit_token return_valid;
   templarbit_property_id 571f4f43-ad7a-415d-894b-1a1f234899db;
   templarbit_fetch_interval 5;
   templarbit_api_url https://api.tb-stag-01.net/v1/csp;

   location / {
      templarbit_csp_enable on;
      root   html;
      index  index.html index.htm;
    }
}
```
