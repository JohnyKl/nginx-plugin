## Tests Usage

### Additional dependencies

Ubuntu:
* libjansson-dev
* libcurl4-gnutls-dev
* libcunit1-dev
* libboost-all-dev

```bash
$ apt-get install libjansson-dev libcurl4-gnutls-dev libcunit1-dev libboost-all-dev
```

CentOS, RHEL, Fedora:
* boost-devel
* curl-devel
* CUnit-devel
* jansson-devel

```bash
$ yum install boost-devel curl-devel CUnit-devel jansson-devel
```

### Run tests

For building and running a nginx-plugin-tests executable, change directory to tests/, type make and wait for building an executable.
Then, run the executable, all test results will be printed to the console.

```bash
$ cd nginx-plugin/tests/
$ make
$ ./nginx-plugin-tests
```
