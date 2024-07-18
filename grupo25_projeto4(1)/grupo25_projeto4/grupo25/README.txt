Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208


Usamos os ficheiros do professor (data.o, entry.o, list.o e table.o) pois não obtivemos uma nota perfeita (20) no primeiro projeto o que o professor referenciou poderia ser a razão para uma das partes 
do segundo projeto (quando ainda estavamos a utilizar o nosso código do projeto anterior) não estar a funcionar.

Como não está especificado no enunciado, o tempo medido das operações, no nosso trabalho, não leva em conta o tempo que estas esperam para ser feitas.

O makefile potencialmente pode apresentar um aviso ao correr, onde diz que foi detetado um desvio do relógio e que a build pode estar incompleta. Isto acontece pois por vezes quando cria o object e o 
executável podem ter tempos diferentes.

Vários readers lêem ao mesmo tempo, no entanto estes atualizam a quantidade de readers individualmente.

No zip, na pasta object, mantivemos os ficheiros do professor (data.o, entry.o, list.o e table.o) pois não são eliminados pelo Makefile nem criados pelo mesmo. É necessário para correr o programa fazer 
make ou make all de forma a que crie os objects o e os binários, e além disso os objectos sdmessage.pb-c.c e sdmessage.pb-c.h.

Ocurreu um pequeno problema na fase de testes em um dos pcs, na altura em que é preciso reabrir o server que tinha sido fechado no cliente B, onde dizia que o server não podia ser aberto, no entanto,
na faculdade e nos pcs dos outros colegas não houve qualquer problema logo consideramos que era um problema particular daquele pc especificamente.

Este trabalho não foi tão auxiliado pelo valgrind como os 3 prévios devido ao professor ter mencionado que potenciais losses de bytes não eram consideradas para a nota neste trabalho.