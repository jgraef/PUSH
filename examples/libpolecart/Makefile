CFLAGS = -Iinclude/ `pkg-config glib-2.0 --cflags` -fPIC -O3 -ffast-math
LDFLAGS = `pkg-config glib-2.0 --libs`
DEPENDFILE = .depend
LIB = libpolecart.so
SRC = polecart.c
OBJ = $(SRC:%.c=%.o)




OBJ = $(SRC:%.c=%.o)

.PHONY: all dep clean

all: dep $(LIB)

dep: $(SRC)
	$(CC) -MM $(CFLAGS) $(SRC) > $(DEPENDFILE)

$(LIB): $(OBJ)
	$(CC) -shared -Wl,-soname,$@ -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(LIB) $(OBJ) $(DEPENDFILE)
	rm -f 

-include $(DEPENDFILE)

