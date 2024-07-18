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

A nivel de testes não encontramos nenhum erro por resolver, e o valgrind não demonstra nenhuns leaks, exceto na situação em que fechamos o server antes de fecharmos os clients (com ctrl-c) mas de 
acordo com a resposta (12/11, 16:55) ao mail que enviamos ao professor não é um problema, pois devemos considerar que os clients são fechados antes do server.