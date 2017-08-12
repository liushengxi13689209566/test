cc = gcc
OBJ1 = server
OBJ2 = client 
all:
		$(cc) -o  $(OBJ1) ./server.c -g   -L/usr/lib64/mysql -lmysqlclient  -lpthread
		$(cc) -o    $(OBJ2) ./client.c -g  -L/usr/lib64/mysql -lmysqlclient  -lpthread

