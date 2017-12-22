cfiles=$(shell ls *.c)
efiles=$(patsubst %.c,%.bin,${cfiles})

all:$(efiles)

%.bin:%.c
	gcc $^ -o $@ -lpthread

clean:
	rm *.bin
