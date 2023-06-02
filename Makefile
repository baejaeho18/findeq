findeq : findeq.c isEqual.c
	gcc -o findeq findeq.c isEqual.c -pthread
	./findeq -t=10 -m=1 -o=output.txt target

findeq2 : findeq_ver2.c
	gcc -o findeq2 findeq_ver2.c -pthread
	./findeq2 -t=5 -m=10 -o=output.txt target

findeq3 : findeq_ver3.c
	gcc -o findeq3 findeq_ver3.c -pthread
	./findeq3 -t=5 -m=10 -o=output.txt target

findeq4 : findeq_ver4.c
	gcc -o findeq4 findeq_ver4.c -pthread
	./findeq4 -t=5 -m=10 -o=output.txt target

test : test.c
	gcc -o test test.c -pthread
	
clean : 
	rm -rf a.out test findeq findeq2 findeq3 findeq4 output.txt