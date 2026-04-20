CC = gcc
CFLAGS = -std=c99 -fstack-protector-all -Wall -Wextra -Isrc -Itst/unity

CORE_SRC = src/hashfile.c src/quadra.c src/habitante.c src/parser.c src/sig.c src/svg.c
MAIN_SRC = src/main.c
TST_HASH = tst/test_hashfile.c
TST_SVG = tst/test_svg.c
UNITY = tst/unity/unity.c

# ALVO FINAL: ted (conforme descricao do projeto)
ted: $(CORE_SRC) $(MAIN_SRC)
	$(CC) $(CFLAGS) $(CORE_SRC) $(MAIN_SRC) -o ted

# Retro-compatibilidade do MAKE
projeto: ted

run: ted
	./ted -e testes/t1 -f c1.geo -pm c1.pm -o saida_teste_1

# ===== Testes ======
test: test_hashfile test_svg
	./test_hashfile
	./test_svg

tstall: test

test_hashfile: $(CORE_SRC) $(TST_HASH) $(UNITY)
	$(CC) $(CFLAGS) $(CORE_SRC) $(TST_HASH) $(UNITY) -o test_hashfile

test_svg: $(CORE_SRC) $(TST_SVG) $(UNITY)
	$(CC) $(CFLAGS) $(CORE_SRC) $(TST_SVG) $(UNITY) -o test_svg

clean:
	rm -f *.dat *.dir test_hashfile test_svg ted projeto.exe *.svg