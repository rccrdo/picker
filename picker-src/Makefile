# CC = gcc -g -O2 -Wall -D_GNU_SOURCE -DNDEBUG
LIBGLIB_CFLAGS = `pkg-config --cflags glib-2.0`
LIBGLIB_LDLIBS = `pkg-config --libs glib-2.0`

PICKER_OBJ = main.o picker_append_helpers.o picker_dump.o picker_bitmap.o \
              picker_mainloop.o picker_procset.o 

EXEC_NAME = picker

.PHONY: picker
    picker:  $(PICKER_OBJ)
	$(CC) -o $(EXEC_NAME) $(LIBGLIB_LDLIBS) $(PICKER_OBJ)
	strip $(EXEC_NAME)
	
.PHONY: new
    new: clean picker


main.o: main.c
	$(CC) $(LIBGLIB_CFLAGS) -c $^ -o $@

picker_append_helpers.o: picker_append_helpers.c
	$(CC) $(LIBGLIB_CFLAGS) -c $^ -o $@

picker_bitmap.o: picker_bitmap.c
	$(CC) $(LIBGLIB_CFLAGS) -c $^ -o $@

picker_dump.o: picker_dump.c
	$(CC) $(LIBGLIB_CFLAGS) -c $^ -o $@
	
picker_mainloop.o: picker_mainloop.c
	$(CC) $(LIBGLIB_CFLAGS) -c $^ -o $@

picker_procset.o: picker_procset.c
	$(CC) $(LIBGLIB_CFLAGS) -c $^ -o $@

.PHONY: purge
    purge: 
	-rm -f *~
	-rm -f *#

.PHONY: clean
    clean: purge
	-rm -f $(PICKER_OBJ)
	-rm -f $(EXEC_NAME)

