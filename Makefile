CC = gcc
CFLAGS = -std=c99 -fstack-protector-all -Wall -Wextra -Isrc -Itst/unity

CORE_SRC = src/hashfile.c src/quadra.c src/habitante.c src/parser.c src/sig.c
MAIN_SRC = src/main.c
TST = tst/test_hashfile.c tst/unity/unity.c

# ALVO FINAL: projeto
projeto: $(CORE_SRC) $(MAIN_SRC)
	$(CC) $(CFLAGS) $(CORE_SRC) $(MAIN_SRC) -o projeto.exe

run: projeto
	.\projeto.exe -e testes\t1 -f c1.geo -pm c1.pm -o saida_teste_1

test: test_hashfile
	./test_hashfile.exe
	git add src/ tst/ Makefile
	git commit -m "test: successful tests execution for Phase 1 Hashfile" || echo "No changes to commit"

test_hashfile: $(CORE_SRC) $(TST)
	$(CC) $(CFLAGS) $(CORE_SRC) $(TST) -o test_hashfile.exe

clean:
	rm -f *.dat *.dir test_hashfile test_hashfile.exe projeto projeto.exe