OBJECTS:=c_timers.o c_wtime.o c_print_results.o c_randdp.o

.PHONY: setup
setup: $(OBJECTS)

c_wtime.o: wtime.c
	$(CC) -Wall -Wextra -O3 $< -c -o $@

%.o: %.c
	$(CC) -Wall -Wextra -O3 $< -c -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS)


