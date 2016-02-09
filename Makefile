all:
	gcc fs.c shell.c -o fs

test:
	./fs

clean:
	@rm -f fs
	@rm -f fs.img

