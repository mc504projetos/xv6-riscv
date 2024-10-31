CC = gcc
CFLAGS = -Wall -g

all: run_experiment

run_experiment: run_experiment.o metrics.o writer.o djisktra.o
	$(CC) $(CFLAGS) -o run_experiment run_experiment.o metrics.o writer.o djisktra.o

run_experiment.o: run_experiment.c metrics.h
	$(CC) $(CFLAGS) -c run_experiment.c

metrics.o: metrics.c metrics.h
	$(CC) $(CFLAGS) -c metrics.c

writer.o: writer.c
	$(CC) $(CFLAGS) -c writer.c

djisktra.o: djisktra.c
	$(CC) $(CFLAGS) -c djisktra.c

clean:
	rm -f *.o run_experiment
