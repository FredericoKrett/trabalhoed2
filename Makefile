CC = gcc
CFLAGS = -std=c99 -fstack-protector-all -Wall -Wextra -Isrc -Itst/unity

SRC = src/hash.c
TST = tst/test_hash.c tst/unity/unity.c

# ALVO FINAL: ted
ted: $(SRC)
	@echo "Alvo 'ted' configurado. Ainda não temos o main.c da CLI."
	@echo "Quando adicionado, a compilação será: $(CC) $(CFLAGS) $(SRC) src/main.c -o ted.exe"

test: test_hash
	./test_hash.exe

test_hash: $(SRC) $(TST)
	$(CC) $(CFLAGS) $(SRC) $(TST) -o test_hash.exe

clean:
	rm -f *.dat *.dir test_hash test_hash.exe ted ted.exe
