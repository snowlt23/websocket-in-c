SRCS = utils.c sha1.c base64.c response.c http.c websocket.c server.c

server: server.h $(SRCS) server_test.c
	gcc -o server $(SRCS) server_test.c
