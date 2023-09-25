exe_00: kthreads.c test_00.c 
	gcc -std=gnu99 -O0 -g kthreads.c test_00.c -lpthread -o $@

exe_09: kthreads.c test_09.c 
	gcc -std=gnu99 -O0 -g kthreads.c test_09.c -lpthread -o $@

exe_wami: kthreads.c test_wami.c 
	gcc -std=gnu99 -O0 -g kthreads.c test_wami.c -lpthread -o $@


clean:
	rm -f exe_*
	rm -f *.dot

