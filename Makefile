build:
	gcc Segregated_Free_Lists.c -O3 -lm -Wall -Wextra -std=c99 -o sfl
run_sfl:
	./sfl
clean:
	rm -f sfl
