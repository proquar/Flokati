
all: flokati-bridge flokati-db

flokati-bridge: flokati-bridge.c
	$(CC) $(CFLAGS) -Isocket -o flokati-bridge flokati-bridge.c

flokati-db: flokati-db.c
	$(CC) $(CFLAGS) -o flokati-db flokati-db.c -Isocket -ljson

# flokati-bridge.o: flokati-bridge.c
# 	$(CC) $(CFLAGS) -c flokati-bridge.c

# remove object files and executable when user executes "make clean"
clean:
	rm flokati-bridge
	rm flokati-db