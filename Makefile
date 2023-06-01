findeq : findeq.c
	gcc -o findeq findeq.c -pthread
	./findeq -t=5 -m=10 -o=output.txt ./

findeq2 : findeq_ver2.c
	gcc -o findeq2 findeq_ver2.c -pthread
	./findeq2 -t=5 -m=10 -o=output.txt ./

findeq3 : findeq_ver3.c
	gcc -o findeq3 findeq_ver3.c -pthread
	./findeq3 -t=5 -m=10 -o=output.txt ./

findeq4 : findeq_ver4.c
	gcc -o findeq4 findeq_ver4.c -pthread
	./findeq4 -t=5 -m=10 -o=output.txt ./

test : test.c
	gcc -o test test.c -pthread
	
clean : 
	rm -rf a.out test findeq output.txt