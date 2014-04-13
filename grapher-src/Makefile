LIBCUTIL_CFLAGS = # make this point to cutil include dir

PICKER_CFLAGS = -I../picker/src

LIBCAIRO_CFLAGS = -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng12
LIBCAIRO_LDFLAGS = -lcairo

CC = gcc -g -O2 -D_GNU_SOURCE -Wall -fno-omit-frame-pointer

GRAPHER_OBJ = grapher_cairo.o grapher_filters.o grapher_process.o \
	       grapher_statstore.o grapher_sys.o grapher_track.o  \
	       main.o

EXEC_NAME = grapher


.PHONY: grapher
    grapher:  $(GRAPHER_OBJ)
	$(CC) -o $(EXEC_NAME) $(LIBCAIRO_LDFLAGS) $(GRAPHER_OBJ)
	
.PHONY: new
    new: clean grapher

grapher_cairo.o: grapher_cairo.c
	$(CC) $(LIBCAIRO_CFLAGS) $(LIBCUTIL_CFLAGS) -c $^ -o $@

grapher_filters.o: grapher_filters.c
	$(CC) $(LIBCUTIL_CFLAGS) -c $^ -o $@

grapher_process.o: grapher_process.c
	$(CC) $(LIBCUTIL_CFLAGS) -c $^ -o $@

grapher_statstore.o: grapher_statstore.c
	$(CC) $(LIBCUTIL_CFLAGS) -c $^ -o $@

grapher_sys.o: grapher_sys.c
	$(CC) $(LIBCUTIL_CFLAGS) -c $^ -o $@

grapher_track.o: grapher_track.c
	$(CC) $(LIBCUTIL_CFLAGS) -c $^ -o $@

main.o: main.c
	$(CC) $(LIBCUTIL_CFLAGS) $(LIBCAIRO_CFLAGS) $(PICKER_CFLAGS) -c $^ -o $@
	
	
.PHONY: clean
    clean:
	-rm -f *~
	-rm -f *#
	-rm -f $(GRAPHER_OBJ)
	-rm -f $(EXEC_NAME)


