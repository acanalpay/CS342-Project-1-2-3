all: libsbmemlib.a app create_memory_sb destroy_memory_sb experiment1 experiment2 experiment3 experiment4

libsbmemlib.a: sbmemlib.c
	gcc -Wall -c sbmemlib.c -lpthread -lrt
	ar -cvq libsbmemlib.a sbmemlib.o
	ranlib libsbmemlib.a

app: app.c
	gcc -Wall -o app app.c -L. -lsbmemlib -lpthread -lrt

create_memory_sb: create_memory_sb.c
	gcc -Wall -o create_memory_sb create_memory_sb.c -L. -lsbmemlib -lpthread -lrt

destroy_memory_sb: destroy_memory_sb.c
	gcc -Wall -o destroy_memory_sb destroy_memory_sb.c -L. -lsbmemlib -lpthread -lrt

experiment1: first_experiment.c
	gcc -Wall -o experiment1 first_experiment.c -L. -lsbmemlib -lpthread -lrt

experiment2: second_experiment.c
	gcc -Wall -o experiment2 second_experiment.c -L. -lsbmemlib -lpthread -lrt

experiment3: third_experiment.c
	gcc -Wall -o experiment3 third_experiment.c -L. -lsbmemlib -lpthread -lrt

experiment4: fourth_experiment.c
	gcc -Wall -o experiment4 fourth_experiment.c -L. -lsbmemlib -lpthread -lrt


clean:
	rm -rf *.o *.a *~ a.out app sbmemlib.o sbmemlib.a libsbmemlib.a create_memory_sb destroy_memory_sb experiment1 experiment2 experiment3 experiment4
