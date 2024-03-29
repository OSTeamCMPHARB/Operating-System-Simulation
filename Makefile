build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c -o scheduler.out
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out
	gcc memory.c -o memory.out

clean:
	rm -f *.out  

all: clean build

run: build
	./process_generator.out processes.txt -sch 3 -mem 2
