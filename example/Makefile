TARGET   = example
CLASSES  = 
SOURCE   = $(TARGET).c $(CLASSES:%=%.c)
OBJECTS  = $(TARGET).o $(CLASSES:%=%.o)
HFILES   = $(CLASSES:%=%.h)
OPT      = -O2 -Wall
CFLAGS   = $(OPT) -I.
LDFLAGS  = -L/usr/lib -La3d -la3d -Lloax -lloax -Lnet -lnet -lpthread
CCC      = gcc

all: $(TARGET)

$(TARGET): $(OBJECTS) a3d net loax
	$(CCC) $(OPT) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY: a3d net loax

a3d:
	$(MAKE) -C a3d -f Makefile.loax

net:
	$(MAKE) -C net

loax:
	$(MAKE) -C loax

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)
	$(MAKE) -C a3d -f Makefile.loax clean
	$(MAKE) -C net clean
	$(MAKE) -C loax clean
	rm a3d net loax

$(OBJECTS): $(HFILES)
