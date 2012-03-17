CFLAGS = -I. `pkg-config glib-2.0 --cflags` -fPIC -DXML_NS -DXML_DTD -DXML_BYTE_ORDER=12 -O3 -ffast-math
#CFLAGS = -I. `pkg-config glib-2.0 --cflags` -fPIC -DXML_NS -DXML_DTD -DXML_BYTE_ORDER=12 -O0 -g
LDFLAGS = -lm `pkg-config glib-2.0 --libs` -lexpat

SRC = code.c config.c dis.c gc.c instr.c interpreter.c rand.c serialize.c stack.c unserialize.c val.c

.PHONY: all clean install run_test

all: libpush.so test

clean:
	rm -f libpush.so

install: all
	cp libpush.so /usr/local/lib
	cp push.h /usr/local/include

libpush.so: $(SRC)
	$(CC) -shared -Wl,-soname,$@ $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: test.c libpush.so
	$(CC) -o $@ $(CFLAGS) $< -L. -lpush

libpush.so: push.h
