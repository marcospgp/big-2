EXEC=Main

$(EXEC):
	gcc View.c Logic.c Main.c -Wall -Wextra -pedantic -ansi -O2 -o cartas

limpar:
	rm $(EXEC) *.o

install: $(EXEC)
	sudo cp style.css /var/www/html/big-2
	sudo cp style.css /var/www/html
	sudo cp card-images/ -r /var/www/html/big-2
	sudo cp cartas /usr/lib/cgi-bin
