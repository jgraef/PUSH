CFLAGS = -I../include/ `pkg-config glib-2.0 gthread-2.0 --cflags` -O3 -ffast-math
#CFLAGS = -I../include/ `pkg-config glib-2.0 gthread-2.0 --cflags` -O0 -g
LDFLAGS = -L../ -lpush

DEPENDFILE = .depend

.PHONY: all clean install dep libpolecart

all: dep polecart

libpolecart:
	make -C libpolecart/

polecart: polecart.c libpolecart
	$(CC) $(CFLAGS) -I libpolecart/include/ -o $@ $< $(LDFLAGS) -L libpolecart/

clean:
	rm -f $(BIN) $(DEPENDFILE)

dep: polecart.c
	$(CC) -MM $(CFLAGS) $^ > $(DEPENDFILE)

-include $(DEPENDFILE)

