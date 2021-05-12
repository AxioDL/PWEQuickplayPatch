# PWE Quickplay Patch
This is a hack for both Metroid Prime 1 and 2 that integrates with Prime World Editor and Dolphin to enable a quickplay feature in the editor. It can also inject extra debug functionality into the game to assist mod developers.

## Dependencies
* Install CMake.
* Install LLVM 12, with at least clang and lld.
* Install [cargo](https://doc.rust-lang.org/cargo/getting-started/installation.html).
* Install [devkitPro](https://devkitpro.org/wiki/Getting_Started) with devkitPPC

## Building
1. Place `prime1.dol` and/or `echoes.dol` in the repository root.
2. Run `cmake`, setting `DLLVM_DIR` to the path to where LLVM 12 is installed.
    * Optionally, set QUICKPLAY_PRIME1 or QUICKPLAY_PRIME2 to control if patches for these games are created.
    * For example: `mkdir build && cd build && cmake -DLLVM_DIR=/usr/lib/llvm-12 -G Ninja ..`
3. Build! The outputs are:
    * quickplay-prime*-default-mod.dol: The dol, modified to load Mod.rel on start
    * quickplay-prime*-Mod.rel: The quickplay patch itself
    * quickplay-prime2.map: The symbols for the quickplay Rel

Note: Building this patch has only been tested under Linux. If you run Windows, you can take a look at [WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10).