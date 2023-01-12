sshell: sshell.o
	gcc -Wall -Wextra -Werror -Wno-unused-value -o sshell sshell.o

sshell.o: sshell.c 
	gcc -Wall -Wextra -Werror -Wno-unused-value -c -o sshell.o sshell.c

clean:
	rm -f sshell sshell.o