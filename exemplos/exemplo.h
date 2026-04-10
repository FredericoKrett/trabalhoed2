#ifndef ARQUIVO_MATRICIAL
#define ARQUIVO_MATRICIAL

#include <stdbool.h>

/*
    POR FAVOR, NOTEM QUE ESTE COMENTARIO INTRODUTORIO E' MUITO IMPORTANTE!

    Um arquivo matricial e' uma matriz de 2 dimensoes, com m linhas e n colunas,
    armazenada em um arquivo do sistema de arquivos do sistema.

    Um novo arquivo matricial pode ser criado por meio da operacao createMF().
    Esta operacao define tamanho da matriz, i.e., o numero de linhas, de colunas
    e o tamanho de cada registro. Note que um arquivo matricial e' agnostico
    em relacao ao tipo do registro.

    Um arquivo matricial pode ser aberto para consulta e/ou atualizacoes por
    meio openMF().

    Sao providas operacoes tipicas para busca, insercao, remocao e alteracao
    de registros no arquivo. Em particular, a operacao de busca searchMF()
    recebe por parametro a funcao que determina o criterio de busca, ou seja,
    se um dado registro dentro do arquivo e' o registro procurado.
*/

typedef struct StArquivoMatricial *ArquivoM;

typedef bool (*encontrou) (int ofsCampo, int tamCampo, void *valorProcurado, void *reg);
/*
    Uma funcao deste tipo retorna true caso um registro do arquivo contenha o valor apontado
    por "valorProcurado" no campo (de tamanho tamCampo) que comeca a partir da posicao (offset)
    "ofsCampo" do inicio do registro. Neste caso, copia em "reg" o conteudo do registro.
    Retorna falso, caso contrario.
*/

ArquivoM createMF(char *filePath, int nLinhas, int nColunas, int tamanhoRegistro);
/*
    Abre para escrita o arquivo nomeado em "filePath", contendo nLinhas e nColunas e
    que armazenara' registros com "tamanhoRegistro" bytes. Os valores de nLinhas, nColunas
    e tamanhoRegistro deve ser positivos diferente de zero.
    Retorna NULL, caso nao tenha conseguido criar o arquivo especificado.
*/

ArquivoM openMF(char *filePath, bool soLeitura);
/*
    Abre o arquivo "filePath". Caso "soLeitura" seja verdadeiro, operacoes que modifiquem
    o arquivo sao proibidas.
    Retorna NULL, caso não consiga abrir o arquivo.
*/

bool insertMF(ArquivoM mf, int lin, int col, void *reg);
/*
    Insere o registro apontado por "reg" na linha "lin", coluna "col" do arquivo.
    Sobreescreve o registro antigo. O tamanho do registro apontado por reg
    deve ser o mesmo tamanho informado na criacao do arquivo matricial.
    Retorna falso, caso [lin,col] estejam fora da faixa de indices validos
    (definido na criacao do arquivo), ou se arquivo foi aberto apenas para leitura.
*/

bool getMF(ArquivoM mf, int lin, int col, void *reg);
/*
     Copia para "reg" o registro armazenado em [lin,col] do arquivo.
     O tamanho do bloco apontado por reg deve ser igual ao tamanho do registro informado
     na criacao do arquivo.
     Retorna falso, caso [lin,col] estejam fora da faixa de indices validos
    (definido na criacao do arquivo), ou se nao existir nenhum registro armazenado 
    naquela posicao.
*/

bool searchMF(ArquivoM mf, encontrou fBusca, int ofsCampo, int tamCampo, void *valorProcurado, void *reg, void *lin, void *col);
/*
      Procura no arquivo um registro para o qual a funcao fBusca(ofsCampo,tamCampo,valorProcurado,reg) retorne true,
      copiando o valor do registro encontrado em "reg" e atribuindo a lin e col a posicao onde o registro foi encontrado.
      A ordem da busca nao e' definida. Isto e', percorre o arquivo em alguma sequencia e retorna o primeiro registro
      que satisfaca a busca.
      Retorna true, caso tenha encontrado um registro; falso, caso contrario.
*/

bool removeMF(ArquivoM mf, int lin, int col);
/*
    Remove o registro que esta' na posicao [lin,col]
    Retorna falso, caso [lin,col] estejam fora da faixa de indices validos
    (definido na criacao do arquivo), ou se arquivo foi aberto apenas para leitura.
*/

bool existMF(ArquivoM mf, int lin, int col);
/*
    Retorna true caso exista um registro armazenado em [lin,col]; falso, caso contrario (ou se lin e/ou col estiverem
    fora da faixa valida)
*/

void closeMF(ArquivoM mf);
/*
    Fecha o arquivo matricial mf.
*/
#endif