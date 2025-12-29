# ⏱ NoMore — Gerenciador de Tempo em Linha de Comando

Aplicação de linha de comando escrita em **C** para gerenciamento de múltiplos cronômetros simultâneos, com suporte a contagem crescente, regressiva, pausa, retomada, persistência automática em disco e atualização contínua.

O projeto foi desenvolvido com foco em:
- Clareza de código
- Estabilidade
- Boas práticas em C
- Organização típica de projetos open source

---

## 📌 Informações Gerais

- **Nome do Projeto:** Cronômetros em C
- **Versão:** 1.0
- **Autor:** Bandeirinha
- **Linguagem:** C (padrão POSIX)
- **Licença:** GNU General Public License v3.0
- **Plataforma alvo:** Linux / Unix-like

---

## 🎯 Objetivo do Projeto

Fornecer um sistema simples, porém robusto, para controle de tempo via terminal, permitindo que o usuário:

- Crie múltiplos cronômetros independentes
- Acompanhe eventos de curto ou longo prazo
- Pause e retome contagens sem perda de precisão
- Persista dados automaticamente entre execuções

Além do uso prático, o projeto também serve como **estudo aplicado** de:

- Manipulação de tempo (`time_t`, `struct tm`)
- Persistência binária em C
- Sincronização com mutex (`pthread`)
- Entrada segura de dados no terminal
- Estruturação de software em C

---

## ✨ Funcionalidades

### Cronômetros
- Até **50 cronômetros simultâneos**
- Nome personalizado para cada cronômetro
- Dois modos de contagem:
  - **Crescente** (tempo decorrido desde a criação)
  - **Regressivo** (tempo restante até uma data/hora alvo)

### Controle
- Pausar cronômetros individualmente
- Retomar cronômetros pausados
- Remover cronômetros específicos
- Limpar **todos** os cronômetros com confirmação

### Interface
- Interface 100% em terminal
- Atualização manual ou automática
- Menu simples e direto
- Feedback claro ao usuário

### Persistência
- Salvamento automático em arquivo binário
- Restauração completa ao reiniciar o programa

---

## 🛠 Compilação

### Requisitos

- GCC

- Sistema Unix-like

- Biblioteca POSIX Threads (pthread)

### Comando recomendado

    gcc nomore.c -o nomore -Wall -Wextra -pthread

### Por que usar -pthread?

- Ativa suporte completo a threads

- Ajusta flags internas do compilador

- Garante linkagem correta

### ▶ Execução
    
    ./nomore
