SRCS = utils.c sha1.c base64.c response.c server.c

server: $(SRCS)
	gcc -o server $(SRCS)
