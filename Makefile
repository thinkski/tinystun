tinystun: tinystun.c
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -f tinystun
