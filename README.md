# FilosofosFamintos
Resolução do problema dos Filósofos Famintos utilizando paralelismo em threads sem starvation e sem deadlock.
Trabalho solicitado para a disciplina Fundamentos de Programação Paralela e Distribuída (2023).

## Especificação do trabalho
Resolva o problema dos Filósofos Famintos para um número N de filósofos (passado como o primeiro parâmetro na linha de comando) de forma que todos eles comam o máximo em paralelo possível, sem starvation, sem deadlock e com completude. Faça com que os filósofos parem de comer assim que qualquer um deles atinja uma quantidade de vezes que comeu igual ao segundo parâmetro passado como argumento na linha de comando.
  
Os únicos prints na tela que devem ser feitos no código final devem seguir o exemplo padronizado a seguir: "Filosofo X esta comendo ao mesmo tempo que os filosofos Y,Z,W." Filósofo comendo sozinho não imprime nada.
  
IMPORTANTE : Essa impressão deve ser em ordem. Supondo X descobrir que Y, Z e W estejam comendo também e supondo que X = 20 , Y = 1 , W = 50 e Z = 14, ele escreveria: "Filosofo 20 esta comendo ao mesmo tempo que os filósofos 1,14,50."

O código deve respeitar a linha de comando "./filosofos.exe <N_filosofos> <M_vezes_comeu>."

Ao final do código, deve ser impresso EM ORDEM CRESCENTE dos filósofos quantas vezes cada filosofo comeu seguindo o padrão:
Filosofo 1 comeu 50 vezes.
Filosofo 2 comeu 70 vezes.

## Regras a serem seguidas:
Não pode haver: SLEEP, WARNING, ERROR, VARIÁVEL GLOBAL, BUSY-WAIT, STARVATION, DEADLOCK.
