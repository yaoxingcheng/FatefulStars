# PKU Hack 2021 Game
## Compile and Run
Simple way to compile and run the demo:
- Compile the raylib source and get the static library
```bash
git clone https://github.com/raysan5/raylib.git
cd raylib/src
make
```
- copy the static library to your project dir
```bash
cp libraylib.a target_dir
```
- compile and run the demo
```bash
g++ -Wall -std=c++11 -O2 -D_DEFAULT_SOURCE -I/usr/local/include -I./include -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL libraylib.a demo/physics.cc -o game
./game
```