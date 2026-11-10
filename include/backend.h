#ifndef BACKEND_H
#define BACKEND_H

/*
 * backend.h — Gestao de Salas POLI-UPE
 *
 * Declara os tipos (struct, enum) e as assinaturas das funcoes do
 * backend. O corpo das funcoes fica em backend.c.
 *
 * Convencao de retorno das funcoes que ALTERAM dados:
 *    0 -> sucesso   (uma mensagem de confirmacao e copiada em mensagem[])
 *   -1 -> erro      (o motivo e copiado em mensagem[])
 *
 * Convencao de retorno das funcoes que LISTAM ou BUSCAM:
 *   >= 0 -> quantidade de registros copiados para o vetor de saida
 *   -1   -> erro de validacao (motivo em mensagem[])
 */

/* ---------- Limites dos vetores ---------- */
#define MAX_AMBIENTES   200
#define MAX_ALOCACOES   3000
#define MAX_RESERVAS    500

/* ---------- Tamanho das strings ---------- */
#define TAM_CODIGO      20
#define TAM_TEXTO       100
#define TAM_MENSAGEM    200

/* Faixa de horario permitida, em minutos contados desde 00:00
 * 07:10 = 7*60 + 10 = 430
 * 22:10 = 22*60 + 10 = 1330                                        */
#define HORA_MINIMA     430
#define HORA_MAXIMA     1330

/* Valor usado quando nao ha reserva a ignorar no teste de conflito  */
#define NENHUM_ID       -1

/* ---------- Enumeracoes ---------- */
enum tipo_ambiente  { SALA, LABORATORIO };
enum dia_semana     { SEGUNDA, TERCA, QUARTA, QUINTA, SEXTA, SABADO };
enum status_reserva { PENDENTE, CONFIRMADA, CANCELADA };

/* ---------- Registros ---------- */
struct Ambiente {
    int  id;
    char codigo[TAM_CODIGO];   /* ex.: "A03", "LIP07"               */
    int  tipo;                 /* enum tipo_ambiente                */
    int  ativo;                /* 1 = ativo, 0 = excluido           */
};

struct Alocacao {
    int  id;
    char turma[TAM_CODIGO];
    char disciplina[TAM_TEXTO];
    char professor[TAM_TEXTO];
    int  id_ambiente;
    int  dia_semana;           /* enum dia_semana                   */
    int  hora_inicio;          /* minutos desde 00:00               */
    int  hora_fim;             /* minutos desde 00:00               */
    int  ativo;                /* 1 = ativa, 0 = cancelada          */
};

struct Reserva {
    int  id;
    char responsavel[TAM_TEXTO];
    char finalidade[TAM_TEXTO];
    int  id_ambiente;
    int  dia_semana;
    int  hora_inicio;
    int  hora_fim;
    int  status;               /* enum status_reserva               */
};

/* ---------- Inicializacao ---------- */
/* Zera os vetores e carrega os tres arquivos de dados.
 * Arquivo inexistente -> lista vazia (nao e erro).                  */
int inicializar_backend(char mensagem[]);

/* ---------- Funcoes comuns ---------- */
int horario_valido(int inicio, int fim);
int horarios_se_sobrepoem(int inicioA, int fimA, int inicioB, int fimB);
int ambiente_ocupado(int id_ambiente, int dia, int inicio, int fim,
                     int id_reserva_ignorar);
int buscar_ambiente_por_id(int id);
void padronizar_codigo(char codigo[]);

/* ---------- Ambientes ---------- */
int listar_ambientes(struct Ambiente saida[], int maximo);
int cadastrar_ambiente(char codigo[], int tipo, char mensagem[]);
int editar_ambiente(int id, char codigo[], int tipo, char mensagem[]);
int excluir_ambiente(int id, char mensagem[]);

/* ---------- Alocacoes ---------- */
int listar_alocacoes(struct Alocacao saida[], int maximo);
int criar_alocacao(char turma[], char disciplina[], char professor[],
                   int id_ambiente, int dia, int inicio, int fim,
                   char mensagem[]);
int editar_alocacao(int id, char turma[], char disciplina[],
                    char professor[], int id_ambiente, int dia,
                    int inicio, int fim, char mensagem[]);
int cancelar_alocacao(int id, char mensagem[]);

/* ---------- Disponibilidade ---------- */
int buscar_ambientes_livres(int dia, int inicio, int fim,
                            struct Ambiente saida[], int maximo,
                            char mensagem[]);

/* ---------- Reservas ---------- */
int listar_reservas(struct Reserva saida[], int maximo);
int registrar_reserva(char responsavel[], char finalidade[],
                      int id_ambiente, int dia, int inicio, int fim,
                      char mensagem[]);
int confirmar_reserva(int id, char mensagem[]);
int cancelar_reserva(int id, char mensagem[]);

#endif
