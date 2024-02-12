OBJS= poller.o pollermodl.o stats.o
EXEC= poller
HEADER= poller.h stats.h

$(EXEC): $(OBJS) $(HEADER)
	gcc $(OBJS) -o $(EXEC) -lpthread


OBJS2= pollerSwayer.o pollermodl.o
EXEC2= pollerSwayer
HEADER2= poller.h

$(EXEC2): $(OBJS2) $(HEADER2)
	gcc $(OBJS2) -o $(EXEC2) -lpthread



cleanS:
	rm -f $(OBJS) $(EXEC)

cleanC:
	rm -f $(OBJS2) $(EXEC2)

clean:
	make cleanS
	make cleanC