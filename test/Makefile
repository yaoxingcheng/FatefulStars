CC = g++
CFLAGS = -Wall -std=c++11 -D_DEFAULT_SOURCE -Wno-missing-braces -s -O1 -D_DEFAULT_SOURCE -I/usr/local/include -I. -I/home/game/raylib/src -I/home/game/raylib/src/external -L. -L/usr/local/lib -L/home/game/raylib/src -L/home/game/raylib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -DPLATFORM_DESKTOP

game: game.cc init.cc update.cc render.cc
	$(CC) -o $@ $^ $(CFLAGS)
clean:
	rm -f $(OBJS) game
