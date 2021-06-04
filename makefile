all: compile

compile:
	gcc main.c -o main.o -lpthread

test: compile
	./main.o -n 4 -p 0.75 -q 5 -t 3 -b 0.05

clean:
	rm main.o