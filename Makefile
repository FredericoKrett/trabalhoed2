CC = gcc
CFLAGS = -std=c99 -fstack-protector-all -Wall -Wextra -Isrc -Itst/unity

SRC = src/hashfile.c
TST = tst/test_hashfile.c tst/unity/unity.c

# ALVO FINAL: ted
ted: $(SRC)
	@echo "Alvo 'ted' configurado. Ainda não temos o main.c da CLI."
	@echo "Quando adicionado, a compilação será: $(CC) $(CFLAGS) $(SRC) src/main.c -o ted.exe"

test: test_hashfile
	./test_hashfile.exe
	git add src/ tst/ Makefile
	git commit -m "test: successful tests execution for Phase 1 Hashfile" || echo "No changes to commit"

test_hashfile: $(SRC) $(TST)
	$(CC) $(CFLAGS) $(SRC) $(TST) -o test_hashfile.exe

clean:
	rm -f *.dat *.dir test_hashfile test_hashfile.exe ted ted.exe
