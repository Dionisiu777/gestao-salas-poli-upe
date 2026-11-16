# gestao-salas-poli-upe
# Sistema de Gestão de Salas — POLI-UPE

Projeto desenvolvido na linguagem C com interface gráfica em GTK 4 para a disciplina de **Programação Imperativa (2ª Unidade)** do curso de **Engenharia da Computação (2º período)** da Escola Politécnica de Pernambuco (POLI-UPE).

## 1. Sobre o Projeto

O sistema foi construído para auxiliar o Departamento de Apoio Técnico-Pedagógico (DATP) da POLI-UPE na organização e controle de salas de aula e laboratórios, substituindo o controle manual feito em planilhas.

### Funcionalidades Principais
- **Gerenciamento de Ambientes:** Cadastro, listagem, edição e exclusão lógica de salas e laboratórios, com padronização de códigos (ex.: `A03`, `LIP07`) e bloqueio de duplicidades.
- **Gerenciamento de Alocações Regulares:** Cadastro, consulta, edição e cancelamento de aulas por turma, disciplina, professor, sala, dia da semana e intervalo de horário (`07:10` às `22:10`), impedindo conflitos de horário no mesmo ambiente.
- **Consulta de Disponibilidade:** Pesquisa rápida de todos os ambientes livres em um determinado dia da semana e faixa de horário.
- **Gerenciamento de Reservas:** Registro de solicitações de reserva com status `PENDENTE`, validação de disponibilidade para aprovação (`CONFIRMADA`) e cancelamento (`CANCELADA`).
- **Persistência em Arquivos Binários:** Gravação e recuperação automática dos registros nos arquivos `dados/ambientes.dat`, `dados/alocacoes.dat` e `dados/reservas.dat`.

## 2. Equipe

Curso: Engenharia da Computação — 2º Período (POLI-UPE)

- Integrante: João Lucas    
- Integrante: Vinicius calado.
- Integrante: Nirlon Diniz
- Integrante: Elbert Melo 
- Integrante: Matheus Gonçalves

## 3. Estrutura de Pastas e Arquivos

```text
gestao-salas-poli-upe/
│
├── src/
│   ├── main.c          # Inicialização da aplicação GTK
│   ├── interface.c     # Telas, formulários, listas e eventos da interface (GTK 4)
│   ├── backend.c       # Regras de negócio, validações e verificação de conflitos
│   └── arquivos.c      # Leitura e gravação em arquivos binários (fread / fwrite)
│
├── include/
│   ├── interface.h     # Declaração da inicialização da interface
│   ├── backend.h       # Structs, enums, constantes e protótipos do backend
│   └── arquivos.h      # Protótipos das funções de persistência
│
├── dados/              # Armazenamento dos arquivos binários (.dat)
│   └── .gitkeep
│
├── README.md
└── .gitignore

4. Pré-requisitos
Para compilar e executar o projeto no Windows, é necessário ter instalado o MSYS2 (ambiente UCRT64) com o compilador GCC, pkg-config e a biblioteca GTK 4:
5. Como Compilar e Executar
Abra o terminal MSYS2 UCRT64 na pasta raiz do projeto (gestao-salas-poli-upe).

Compile todos os módulos com o comando:
gcc src/main.c src/interface.c src/backend.c src/arquivos.c -Iinclude -o gestao-salas.exe $(pkg-config --cflags --libs gtk4)
Execute a aplicação:
./gestao-salas.exe