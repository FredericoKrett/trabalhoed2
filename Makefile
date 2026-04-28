CC = gcc
CFLAGS = -std=c99 -fstack-protector-all -Wall -Wextra -Isrc -Itst/unity

CORE_SRC = src/hashfile.c src/quadra.c src/habitante.c src/parser.c src/sig.c src/svg.c
MAIN_SRC = src/main.c
TST_HASH = tst/test_hashfile.c
TST_SVG = tst/test_svg.c
TST_HABITANTE = tst/test_habitante.c
TST_QUADRA = tst/test_quadra.c
TST_PARSER = tst/test_parser.c
TST_SIG = tst/test_sig.c
UNITY = tst/unity/unity.c

# ALVO FINAL: ted (conforme descricao do projeto)
ted: $(CORE_SRC) $(MAIN_SRC)
	$(CC) $(CFLAGS) $(CORE_SRC) $(MAIN_SRC) -o ted

# Retro-compatibilidade do MAKE
projeto: ted

run: ted
	mkdir -p saida_teste_1_base saida_teste_1_mudanca saida_teste_1_mudanca_verif
	./ted -e testes/t1 -f c1.geo -pm c1.pm -o saida_teste_1_base
	./ted -e testes/t1 -f c1.geo -pm c1.pm -q c1/mudanca-todos-moradores.qry -o saida_teste_1_mudanca
	./ted -e testes/t1 -f c1.geo -pm c1.pm -q c1/mudanca-todos-moradores-com-verif.qry -o saida_teste_1_mudanca_verif

# ===== Testes ======
test: test_hashfile test_svg test_habitante test_quadra test_parser test_sig
	./test_hashfile
	./test_svg
	./test_habitante
	./test_quadra
	./test_parser
	./test_sig

tstall: test

test_hashfile: $(CORE_SRC) $(TST_HASH) $(UNITY)
	$(CC) $(CFLAGS) $(CORE_SRC) $(TST_HASH) $(UNITY) -o test_hashfile

test_svg: $(CORE_SRC) $(TST_SVG) $(UNITY)
	$(CC) $(CFLAGS) $(CORE_SRC) $(TST_SVG) $(UNITY) -o test_svg

test_habitante: src/habitante.c $(TST_HABITANTE) $(UNITY)
	$(CC) $(CFLAGS) src/habitante.c $(TST_HABITANTE) $(UNITY) -o test_habitante

test_quadra: src/quadra.c $(TST_QUADRA) $(UNITY)
	$(CC) $(CFLAGS) src/quadra.c $(TST_QUADRA) $(UNITY) -o test_quadra

test_parser: $(CORE_SRC) $(TST_PARSER) $(UNITY)
	$(CC) $(CFLAGS) $(CORE_SRC) $(TST_PARSER) $(UNITY) -o test_parser

test_sig: $(CORE_SRC) $(TST_SIG) $(UNITY)
	$(CC) $(CFLAGS) $(CORE_SRC) $(TST_SIG) $(UNITY) -o test_sig

clean:
	rm -f *.dat *.dir *.hf *.hfc test_hashfile test_svg test_habitante test_quadra test_parser test_sig ted projeto.exe *.svg test_output.svg parser_test.geo parser_test.pm
	rm -rf saida_testes saida_teste_1 saida_teste_1_base saida_teste_1_mudanca saida_teste_1_mudanca_verif

run_all: ted
	@set -e; \
	if [ ! -d testes/testes-t1 ]; then \
		echo "Diretorio testes/testes-t1 ausente; run_all usa apenas a massa local de desenvolvimento."; \
	else \
		tmp_root=$$(mktemp -d /tmp/ted-run-all.XXXXXX); \
		trap 'rm -rf "$$tmp_root"' EXIT; \
		runner="$$tmp_root/ted"; \
		test_root="$$tmp_root/testes-t1"; \
		out_root="$$tmp_root/saida_testes"; \
		cp ./ted "$$runner"; \
		chmod +x "$$runner"; \
		cp -a testes/testes-t1 "$$test_root"; \
		mkdir -p "$$out_root"; \
		for c in c1 c2 c3; do \
			out_case="$$out_root/$${c}/_base"; \
			mkdir -p "$$out_case"; \
			"$$runner" -e "$$test_root" -f "$${c}.geo" -pm "$${c}.pm" -o "$$out_case"; \
			for qry in testes/testes-t1/$${c}/*.qry; do \
				qry_file=$$(basename "$$qry"); \
				qry_name=$${qry_file%.qry}; \
				out_case_qry="$$out_root/$${c}/_$${qry_name}"; \
				mkdir -p "$$out_case_qry"; \
				"$$runner" -e "$$test_root" -f "$${c}.geo" -pm "$${c}.pm" -q "$${c}/$$qry_file" -o "$$out_case_qry"; \
			done; \
		done; \
		rm -rf saida_testes; \
		mkdir -p saida_testes; \
		cp -a "$$out_root/." saida_testes/; \
	fi
