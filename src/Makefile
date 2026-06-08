CC = gcc
CFLAGS = -std=c99 -fstack-protector-all -Wall -Wextra

SRC = hashfile.c quadra.c habitante.c parser.c sig.c svg.c main.c

ted: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o ted

projeto: ted

clean:
	rm -f ted *.o *.dat *.dir *.hf *.hfc *.svg projeto.exe
