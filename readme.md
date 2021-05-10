# Fateful Stars - HackPKU 2021 Game
![](resources/FatefulStars.jpg)
This repository provides the source code of *Fateful Stars*, a simple yet interesting game based on [raylib](https://github.com/raysan5/raylib), which wins the first place of [HackPKU2021](https://www.hackpku.com/).
## Compile and Run

### For Linux System
Please install [raylib](https://github.com/raysan5/raylib)  first based on the instruction of official Wiki.

To build the project, simply run:

```bash
cp Makefile_linux.example Makefile
make
```

which yields the `game` binary in the root directory of the project. Enjoy it!

### For MacOS

First build raylib from source to get a static library, and copy that library to this project root directory
```bash
git clone https://github.com/raysan5/raylib
cd raylib/src/
make
cp libraylib.a fatefulstars_dir
```
Then build the projects
```bash
cp Makefile_macos.example Makefile
make
```
which also yields the `game` binary in the root directory of the project. Enjoy it!
