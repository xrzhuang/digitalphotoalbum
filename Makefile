CFLAGS = -g -std=c99 -Wall

album: album.c input_prompt.c
	gcc $(CFLAGS) -o album album.c input_prompt.c

clean:
	rm -f album  med* thumb* index.html *.o
