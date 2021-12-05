all: chat_server chat

chat_server: server.c
	gcc -pthread server.c -o chat_server -Wall

chat: chat.c
	gcc -pthread chat.c -o chat -Wall