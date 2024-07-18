Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208


Usamos os ficheiros do professor (data.o, entry.o, list.o e table.o) pois não obtivemos uma nota perfeita (20) no último trabalho o que o professor referenciou poderia ser a razão para uma das partes 
deste projeto (quando ainda estavamos a utilizar o nosso código do projeto anterior) não estar a funcionar.

Tal como discutido previamente com o professor por email (embora tenhamos reduzido o número de avisos sobre conditional jumps e unitialised value(s) substancialmente) ainda nos aparece um aviso após o 
cliend dar "quit":
    Server ready, waiting for connections 
    Client connection established
    ==7913== Conditional jump or move depends on uninitialised value(s)
    ==7913==    at 0x4851EDE: protobuf_c_message_unpack (in /usr/lib/x86_64-linux-gnu/libprotobuf-c.so.1.0.0)
    ==7913==    by 0x10BC6A: message_t__unpack (sdmessage.pb-c.c:88)
    ==7913==    by 0x10A90E: network_receive (network_server.c:147)
    ==7913==    by 0x10A799: network_main_loop (network_server.c:86)
    ==7913==    by 0x10A61E: main (table_server.c:48)
    ==7913== 
    Client connection closed

O nosso Makefile não cria sdmessage.pb-c.c e sdmessage.pb-c.h pois consideramos que o utilizador já deve ter esses ficheiros nas pastas corretas. A maneira de criar esses ficheiros é fazendo 
protoc-c sdmessage.proto --c_out=. e depois movendo os ficheiros creados para as pastas correspondentes (sdmessage.pb-c.c para source e sdmessage.pb-c.h para include). No zip entregue já temos criado 
o sdmessage.pb-c.h e sdmessage.pb-c.c dentro das respetivas pastas.

Quando damos "quit" ao server (com CTRL-C), um número de bytes em um número de blocos ainda ficam alcançáveis, mas tal acontece no server do professor também.

No zip, na pasta object, mantivemos os ficheiros do professor (data.o, entry.o, list.o e table.o) pois não são eliminados pelo Makefile nem criados pelo mesmo.

A nivel de testes não encontramos nenhum erro por resolver.