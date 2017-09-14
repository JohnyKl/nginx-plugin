## Tests Usage

### Additional dependencies

Packages are need to be installed:
* libjansson-dev
* libcurl4-dev
* libcunit1-dev
* libboost-all-dev

### Run tests

For building and running a nginx-plugin-tests executable, change directory to tests/, type make and wait for building an executable.
Then, run the executable, all test results will be printed to the console.

```bash
$ cd nginx-plugin/tests/
$ make
$ ./nginx-plugin-tests
```