# Regras do Jogo de Tabuleiro Banco Imobiliário

## 1. Objetivo do Jogo

O Banco Imobiliário é um jogo de tabuleiro onde os jogadores assumem o papel de investidores imobiliários. Compram propriedades, cobram aluguéis, negociam entre si e tentam acumular o maior patrimônio possível.

**Objetivo:** ser o último jogador com dinheiro e propriedades, ou ter a maior pontuação ao final.

---

## 2. Componentes

- Tabuleiro com 40 casas (circuito fechado)
- 2 dados
- Peões (carrinhos) — um por jogador
- Cartas de propriedade
- Cartas de Sorte e Reves (baralho)
- Dinheiro fictício (gerenciado pela maquininha RFID)
- Maquininha de cartões Arduino com cartão RFID por jogador
- Casas e hotéis de construção

---

## 3. Preparação do Jogo

1. O jogo é jogado por **2 a 6 jogadores**.
2. Cada jogador começa com **R$ 400,00** na maquininha (configurável no menu com **#**).
3. Para determinar a ordem, cada jogador lança um dado. Quem tirar o maior número começa.
4. Cada jogador escolhe seu peão e o coloca na **Posição de Partida**.
5. Na maquininha, inicie um **novo jogo**, selecione a quantidade de jogadores e cadastre os cartões RFID.

---

## 4. Uso da Maquininha de Cartões

A maquininha gerencia todo o dinheiro do jogo. Não utilize dinheiro físico.

### Cadastro

1. Menu inicial → **1** (Novo jogo)
2. Selecione a quantidade de jogadores (2–6)
3. Cada jogador aproxima seu cartão RFID

### Operações durante o jogo

| Ação no jogo | Operação na maquininha |
|---|---|
| Comprar propriedade do banco | **−** (retirar) → cartão do comprador |
| Pagar aluguel | **C** (transferir) → cartão do pagador, depois do recebedor |
| Receber de carta Sorte/Reves | **+** (adicionar) → cartão do jogador |
| Pagar ao banco (imposto, multa) | **−** (retirar) → cartão do jogador |
| Passar pelo Ponto de Partida | **#** (salário) → cartão do jogador |
| Cair no Ponto de Partida | **#** (salário) → cartão do jogador |
| Leilão (pagamento) | **−** ou **C** conforme o caso |
| Negociação entre jogadores | **C** (transferir) |
| Falência (eliminação) | Saldo zerado com **−** |

### Atalhos do teclado

| Tecla | Função |
|---|---|
| + | Adicionar dinheiro |
| − | Retirar dinheiro |
| C | Transferir entre jogadores |
| * | Ver saldo de todos |
| # | Salário (passagem/chegada ao início) |
| D | Desfazer última operação |

### Continuar partida

- Menu inicial → **2** (Continuar jogo salvo)
- Os saldos são mantidos na memória da maquininha entre sessões

---

## 5. Desenvolvimento do Jogo

### 5.1 Turno do jogador

1. Lança os dados e move o peão no sentido **horário**.
2. Executa a ação da casa em que parou (ver seção 6).
3. Pode negociar propriedades e dinheiro com outros jogadores a qualquer momento.
4. Passa a vez ao próximo jogador.

### 5.2 Passagem e chegada ao Ponto de Partida

- **Passar** pelo Ponto de Partida (sem parar): recebe **R$ 200,00** (salário).
- **Cair** exatamente no Ponto de Partida: recebe **R$ 200,00**.
- Registre o crédito na maquininha com **#** (salário).

### 5.3 Compra de propriedades

1. Ao cair em propriedade **sem dono**, o jogador pode comprá-la pelo preço indicado.
2. Se **recusar**, a propriedade vai a **leilão** (seção 7).
3. Registre a compra na maquininha com **−** (retirar).

### 5.4 Aluguel

1. Ao cair em propriedade **com dono**, paga o aluguel indicado na carta.
2. O aluguel aumenta conforme casas e hotéis construídos (tabela na carta).
3. Registre na maquininha com **C** (transferir): pagador → recebedor.
4. Propriedades **hipotecadas** não cobram aluguel.

### 5.5 Construção

1. Para construir, o jogador deve possuir **todas as propriedades de uma cor** (monopólio).
2. Construção deve ser **uniforme**: não pode ter 2 casas em uma propriedade e 0 em outra da mesma cor.
3. Máximo de **4 casas** por propriedade; a 5ª construção é substituída por **1 hotel**.
4. Máximo de **32 casas** e **12 hotéis** no jogo total.
5. Registre o custo de construção na maquininha com **−** (retirar).

### 5.6 Hipoteca

1. Jogador sem dinheiro pode hipotecar propriedades ao banco.
2. Valor da hipoteca = **metade do preço de compra** (indicado na carta).
3. Registre o crédito na maquininha com **+** (adicionar).
4. Para **deshipotecar**, pague o valor da hipoteca + **10% de juros**.
5. Registre o pagamento com **−** (retirar).
6. Não é possível construir em propriedade hipotecada.

### 5.7 Falência

1. Se o jogador não puder pagar uma dívida (mesmo vendendo casas e hipotecando):
   - Deve declarar **falência**.
   - Suas propriedades retornam ao banco ou vão a leilão.
   - Seu saldo na maquininha é zerado com **−**.
   - O jogador é **eliminado**.

---

## 6. Casas Especiais

### 6.1 Ponto de Partida

Recebe R$ 200,00 ao passar ou cair. Registre com **#** na maquininha.

### 6.2 Sorte e Reves

Ao cair nesta casa, o jogador compra uma carta do baralho e executa a instrução. Exemplos:

- Avançar ou recuar casas
- Receber ou pagar dinheiro ao banco
- Receber de todos os jogadores (Aniversário)
- Ir para a Cadeia
- Carta "Saia da Cadeia Livre" (guardar para uso)

Registre ganhos com **+** e pagamentos com **−** ou **C** conforme a carta.

### 6.3 Cadeia (Prisão)

- **Visitar:** movimento normal, sem penalidade.
- **Ir preso:** carta ou casa "Vá para a Cadeia" — peão vai para a Cadeia sem passar pelo início.
- **Sair da Cadeia** (na próxima vez que jogar):
  1. Pagar **R$ 50,00** de fiança, ou
  2. Usar carta "Saia da Cadeia Livre", ou
  3. Tirar **dados iguais** (duplos) — até 3 tentativas; se falhar, paga fiança.
- Após sair, joga normalmente mas **não joga de novo** (exceto se saiu com duplos antes de ir preso).

### 6.4 Estacionamento

Casa livre. Nenhuma ação ou pagamento.

### 6.5 Impostos

- **Imposto de Renda:** pague **R$ 200,00** ou **10%** do patrimônio total (jogador escolhe o menor).
- **Imposto sobre Luxo:** pague **R$ 100,00**.
- Registre com **−** na maquininha.

### 6.6 Ferrovias e Utilidades

- **Ferrovias:** aluguel depende de quantas ferrovias o dono possui (1=25%, 2=50%, 3=75%, 4=100% do valor base).
- **Utilidades (Água/Luz):** lance os dados e pague valor base × resultado do dado (1 utilidade) ou × 10 (2 utilidades).

---

## 7. Leilão

Quando um jogador recusa comprar uma propriedade:

1. O banco leiloa a propriedade entre todos os jogadores.
2. **Lance mínimo:** R$ 1,00 (ou metade do preço, a critério dos jogadores).
3. Lances em ordem horária; cada jogador pode desistir.
4. Sem lances: a propriedade permanece com o banco.
5. Vencedor paga na maquininha com **−** (retirar).

---

## 8. Negociação

- Jogadores podem trocar propriedades, dinheiro e cartas "Saia da Cadeia" a qualquer momento.
- Propriedades com casas/hotéis: remova construções antes de negociar (metade do custo de volta).
- Registre transferências de dinheiro na maquininha com **C**.

---

## 9. Fim do Jogo

### 9.1 Vitória do único sobrevivente

Se todos os outros jogadores declararem falência, o último restante vence.

### 9.2 Contagem de pontos (fim por acordo)

Se os jogadores concordarem em encerrar:

**Pontuação = dinheiro em caixa + valor de propriedades + valor de casas/hotéis**

| Item | Valor |
|---|---|
| Propriedade sem construção | Preço de compra |
| Cada casa | Valor de construção |
| Cada hotel | Valor de construção + 4 casas |
| Propriedade hipotecada | Metade do preço de compra |
| Cartas "Saia da Cadeia" | R$ 50,00 cada |

Maior pontuação vence.

### 9.3 Variante: meta de patrimônio

Opcional: primeiro jogador a atingir **R$ 5.000,00** em patrimônio total vence.

### 9.4 Variante: limite de tempo

Opcional: após **90 minutos**, encerra-se com contagem de pontos.

---

## 10. Variantes Opcionais

| Variante | Regra |
|---|---|
| **Estacionamento Jackpot** | Multas pagas vão ao Estacionamento; quem cair lá recebe |
| **Leilão obrigatório** | Toda propriedade recusada vai a leilão (sem exceção) |
| **Construção acelerada** | Permite construir com 2 de 3 propriedades de uma cor |
| **Free Parking bônus** | R$ 100 ao cair no Estacionamento |

---

## 11. Resumo de Valores

| Item | Valor |
|---|---|
| Dinheiro inicial | R$ 400,00 |
| Salário (Ponto de Partida) | R$ 200,00 |
| Fiança (Cadeia) | R$ 50,00 |
| Imposto de Renda | R$ 200,00 ou 10% |
| Imposto de Luxo | R$ 100,00 |
| Juros de deshipoteca | 10% |
| Meta de patrimônio (variante) | R$ 5.000,00 |

---

## Conclusão

O Banco Imobiliário combina estratégia, negociação e administração financeira. A maquininha RFID torna as transações rápidas e precisas, permitindo foco na diversão e nas negociações. Boa sorte e que o melhor investidor vença!
