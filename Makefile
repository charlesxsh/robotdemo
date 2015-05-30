All:testrv

testrv:recordvoice.o testrv.o
	gcc recordvoice.o testrv.o -o testrv -lasound

recordvoice.o:recordvoice.c recordvoice.h
	gcc -c recordvoice.c recordvoice.h

testrv.o:testrv.c
	gcc -c testrv.c

clean:
	rm -rf testrv.o recordvoice.o