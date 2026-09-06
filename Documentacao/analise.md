Melhorias no código Arduino
Bugs e problemas críticos
1. Modo simulador ativo por padrão


banco.ino
Lines 11-11
boolean teste = true; //TESTE EM SIMULADOR (ativa/desativa)
Com teste = true, o RFID real é ignorado. Em produção, deve ser false.

2. Variáveis globais não resetadas em novo jogo

aux, auxTeste e jogadorCont não são zerados ao iniciar um novo jogo. Isso pode causar cadastro incompleto ou cartões duplicados na segunda partida.

3. Transferência incompleta perde dinheiro

Em operacaoTransferir(), o débito do primeiro jogador ocorre antes da leitura do segundo cartão. Se o segundo cartão nunca for lido, o valor some sem dadosDesfazer completo — não há como desfazer.

4. Recursão em operacaoTransferir()

A função chama a si mesma (operacaoTransferir() na linha 536). Em Arduino, isso consome stack e pode causar crash. Melhor usar um estado (jogadorCont) no loop principal.

5. Atribuição inválida char key = ""

Em vários pontos (calculadora, menuDeInicio), key = "" atribui string vazia a um char. O correto é key = '\0' ou simplesmente não reatribuir.

6. #include <SPI.h> duplicado

Linhas 5 e 7 — remover a duplicata.

7. Serial.println sem Serial.begin()

Usado na linha 201, mas setup() não inicializa a Serial.

Arquitetura e qualidade
8. Usar long em vez de float para dinheiro


banco.ino
Lines 64-67
struct contaJogadores {
  int id;
  float saldoConta;
};
Valores monetários em centavos (long saldoCentavos) evitam erros de arredondamento (ex.: R$ 0,01).

9. Reduzir uso de String

valorTela, tagID, textoEmTela etc. fragmentam a heap do Arduino. Preferir char[] com tamanho fixo.

10. Substituir delay() por temporizador não bloqueante

delay(tempoDeTela) bloqueia teclado e RFID durante 1 segundo. Usar millis() permite leitura contínua e melhor UX.

11. EEPROM mais robusta

Problema atual	Melhoria
Sem checksum/versão
Header com magic number + versão
dinheiroInicial não persistido
Salvar junto com os jogadores
Sem opção de apagar partida
Tecla longa ou combinação para reset
lendoEPRROM() sempre chama salvaNaEEPROM()
Separar leitura e escrita claramente
12. Cadastro dinâmico de cartões RFID

Hoje só 5 cartões hardcoded em nomeCartao[]. Cartões desconhecidos retornam -1 sem feedback. Sugestão: na primeira leitura, permitir nomear o cartão e gravar o UID na EEPROM.

13. Feedback quando cartão não é reconhecido

Em operacaoAdicionar, operacaoRetirar e operacaoTransferir, se cartao == -1 ou posicaoJogador == -1, a tela fica parada sem mensagem. Exibir "Cartão inválido" ou "Jogador não cadastrado".

14. Transferência para o mesmo jogador

Se o segundo cartão for o mesmo do primeiro, nada acontece e o dinheiro já foi debitado. Validar e exibir erro.

15. Limite de dígitos inconsistente

Em calculadora(), dígitos são adicionados sem checar tamanho; a validação > 8 só roda no else if final e pode não pegar todos os casos. Validar antes de concatenar.

16. valorTela.toInt() vs long value2

toInt() retorna int (16 bits no AVR = máx. 32.767). Para valores maiores, usar atol() ou parsing manual para long.

Funcionalidades sugeridas
Funcionalidade	Descrição
Confirmação de operação
Antes de executar, mostrar valor + operação e pedir confirmação (#)
Histórico de transações
Ring buffer na EEPROM com últimas N operações
Atalho "Passou pelo início"
Tecla rápida para creditar salário (R$ 100)
Modo leilão
Dividir valor entre jogadores presentes
Som/buzzer
Feedback sonoro em sucesso/erro
Timeout de inatividade
Voltar ao menu após X segundos sem uso
Suporte a 6 jogadores
Alinhar com as regras do jogo (qtdMaximaDeJogadores = 6)
Validação ao continuar jogo
Checar integridade da EEPROM antes de carregar
Organização do código
Codigo/
├── banco.ino           # setup + loop
├── config.h            # pinos, constantes, qtdMaximaDeJogadores
├── cartoes.h           # struct nomeCartoes, cadastro RFID
├── eeprom_manager.h    # leitura/escrita/validação
├── display_utils.h     # exibeLcd, telaDeCarregamento
├── operacoes.h         # adicionar, retirar, transferir, desfazer
└── menus.h             # menuDeInicio, qtdJogadores, calculadora
Isso facilita manutenção e testes no Wokwi.

Inconsistências doc × código
Item	README/Regras	Código
Jogadores máx.
6
4
Dinheiro inicial
R$ 400
R$ 100 (default)
Biblioteca LCD
LiquidCrystal.h
LiquidCrystal_I2C.h
Pinos teclado (doc)
linhas 4,3,2,5
linhas 2,3,4,5
Teclas (PNG)
A=add, B=retirar
+ e -
Licença MIT
mencionada
arquivo LICENSE ausente
Vale atualizar README e documentação.md para refletir o código real.

Melhorias nas regras do jogo
Inconsistências internas
1. Dinheiro inicial vs. salário

Início: R$ 400
Passar pelo ponto de partida: R$ 100
Proporção baixa (25% do capital inicial). No Monopoly clássico, salário ≈ 13% do capital. Sugestões:

Aumentar salário para R$ 200, ou
Reduzir capital inicial para R$ 300
2. Integração com a maquininha

As regras não mencionam o dispositivo Arduino. Adicionar seção:

Uso da Maquininha de Cartões

Cada jogador possui um cartão RFID vinculado à sua conta
Compras, aluguéis e salários são registrados na maquininha
Passar pelo início: jogador aproxima o cartão e pressiona "+" com valor 100
Falência: saldo zerado na maquininha; jogador eliminado
3. Limite de jogadores

Regras dizem 2–6; código suporta 2–4. Decidir: ampliar hardware ou ajustar regras para 2–4.

Regras ausentes (comuns em jogos desse tipo)
O PDF Baralho.pdf existe, mas as regras não citam cartas de sorte/reves. Sugestões:

Tema	O que falta
Cartas de baralho
Sorte/Reves: efeitos (ganhar/perder dinheiro, mover peão, etc.)
Cadeia/prisão
Ir preso, pagar fiança, carta "Saia da cadeia"
Estacionamento
Regra do estacionamento (livre ou jackpot?)
Impostos
Casas de imposto no tabuleiro
Leilão
Procedimento: lance mínimo, ordem, tempo
Construção
Máximo de casas no tabuleiro, construção uniforme
Hipoteca
Valor, juros para deshipotecar, receber aluguel hipotecado?
Negociação
Troca de propriedades/dinheiro entre jogadores
Aluguel escalonado
Tabela por número de casas/hotéis
Falência
Passo a passo: vender casas → hipotecar → transferir propriedades → eliminação
Duplo salário
Cair exatamente no início vs. apenas passar
Utilities/ferrovias
Regras específicas de aluguel
Sugestões de balanceamento
4. Leilão quando jogador recusa compra

Detalhar:

Lance mínimo: metade do preço da propriedade
Sem lances: propriedade permanece com o banco
Tempo limite por rodada de lances
5. Condição de vitória alternativa

Além de "último sobrevivente", incluir:

Limite de tempo (ex.: 90 min) → contagem de pontos
Meta de patrimônio (ex.: R$ 5.000) → vitória imediata
6. Regra anti-monopólio lento

Se um jogador dominar muitas propriedades sem construir por X voltas, incentivo (taxa de manutenção ou leilão forçado).

7. Cartão "Aniversário"

No baralho: todos pagam R$ X ao jogador — registrar na maquininha como múltiplas transferências.

Estrutura sugerida para o documento de regras
1. Objetivo
2. Componentes (tabuleiro, cartas, maquininha, peões, dados)
3. Preparação (dinheiro inicial, ordem, cadastro RFID)
4. Turno do jogador (dado → movimento → ação)
5. Propriedades (compra, leilão, aluguel, construção)
6. Hipoteca e falência
7. Casas especiais (início, prisão, imposto, sorte/reves)
8. Uso da maquininha (operações e atalhos)
9. Fim de jogo e contagem de pontos
10. Variantes opcionais
Priorização sugerida
Código (impacto imediato)
teste = false em produção
Reset de aux, auxTeste, jogadorCont em novo jogo
Corrigir transferência (estado + rollback)
Feedback para cartão inválido
float → long para saldos
Alinhar 6 jogadores e R$ 400 com as regras
Regras (impacto na jogabilidade)
Seção de uso da maquininha
Alinhar salário e capital inicial
Regras do baralho (Sorte/Reves)
Procedimento de leilão e falência
Regras de construção e hipoteca
Se quiser, posso implementar alguma dessas melhorias no código ou reescrever o documento de regras com a estrutura proposta.