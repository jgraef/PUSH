CFLAGS = -I include/ `pkg-config glib-2.0 gthread-2.0 --cflags` -fPIC -O3 -ffast-math
#CFLAGS = -I include/ `pkg-config glib-2.0 gthread-2.0 --cflags` -fPIC -O0 -g
LDFLAGS = -lm `pkg-config glib-2.0 gthread-2.0 --libs`

SRC = code.c dis.c gc.c instr.c interpreter.c rand.c push.c serialize.c stack.c unserialize.c val.c vm.c
OBJ = $(SRC:%.c=%.o)
DEPENDFILE = .depend
PREFIX = /usr/local

.PHONY: all clean install dep

all: dep libpush.so test

clean:
	rm -f $(OBJ) $(DEPENDFILE) libpush.so test

install: all
	cp libpush.so $(PREFIX)/lib
	cp -R include/* $(PREFIX)/include

dep: $(SRC)
	$(CC) -MM $(CFLAGS) $^ > $(DEPENDFILE)

libpush.so: $(OBJ)
	$(CC) -shared -Wl,-soname,$@ -o $@ $^ $(LDFLAGS)

test: test.c libpush.so
	$(CC) -o $@ $(CFLAGS) $< -L. -lpush

-include $(DEPENDFILE)

