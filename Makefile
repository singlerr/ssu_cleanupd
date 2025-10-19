CC = gcc
CFLAGS = -g
TARGET = ssu
OBJECTS = main.o logger.o conf.o worker.o worker_group.o show.o add.o modify.o remove.o path_utils.o string_utils.o file_utils.o list.o
HEADERS = path_utils.h string_utils.h file_utils.h list.h api.h base.h worker.h worker_group.h logger.h conf.h

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

main.o : main.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

logger.o : logger.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

conf.o : conf.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

worker.o : worker.c$(HEADERS)
	$(CC) $(CLFAGS) -c $^

worker_group.o : worker_group.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

show.o : show.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

add.o : add.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

modify.o : modify.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

remove.o : remove.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

path_utils.o : path_utils.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

string_utils.o : string_utils.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

file_utils.o : file_utils.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

list.o : list.c $(HEADERS)
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f *.o $(TARGET)
