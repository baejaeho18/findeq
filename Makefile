findeq : findeq.c
	gcc -o findeq findeq.c -pthread

findeq2 : findeq_ver2.c
	gcc -o findeq2 findeq_ver2.c -pthread

findeq3 : findeq_ver3.c
	gcc -o findeq3 findeq_ver3.c -pthread

test : test.c
	gcc -o test test.c -pthread
	
clean : 
	rm -rf a.out test findeq output.txt