all:lenet2_all_result_get.c cnnfunc.c
	gcc -lm lenet2_all_result_get.c cnnfunc.c 
timer:lenet_cpu_time.c cnnfunc.c
	gcc -O3 -lm cnnfunc.c lenet_cpu_time.c 
clean:
	rm a.out
