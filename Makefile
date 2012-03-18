CFLAGS = -I include/ `pkg-config glib-2.0 --cflags` -fPIC -DXML_NS -DXML_DTD -DXML_BYTE_ORDER=12 -O3 -ffast-math
#CFLAGS = -I include/ `pkg-config glib-2.0 --cflags` -fPIC -DXML_NS -DXML_DTD -DXML_BYTE_ORDER=12 -O0 -g
LDFLAGS = -lm `pkg-config glib-2.0 --libs` -lexpat

SRC = code.c config.c dis.c gc.c instr.c interpreter.c rand.c push.c serialize.c stack.c unserialize.c val.c
OBJ = $(SRC:%.c=%.o)
DEPENDFILE = .depend
PREFIX = /usr/local

.PHONY: all clean install dep

all: dep libpush.so test

clean:
	rm -f $(OBJ) .depend libpush.so test

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

