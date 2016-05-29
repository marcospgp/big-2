CFLAGS= Main.c Logic.c View.c -Wall -Wextra -pedantic -Wno-long-long -O2
OBJS=$(patsubst %.c, %.o, $(wildcard *.c))
EXEC=cartas

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

limpar:
	rm $(EXEC) *.o

install: $(EXEC)
	sudo cp style.css /var/www/html
	sudo cp cartas /usr/lib/cgi-bin
