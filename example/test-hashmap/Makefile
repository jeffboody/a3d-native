TARGET   = test-hashmap
CLASSES  = a3d/a3d_list a3d/a3d_hashmap a3d/a3d_log
SOURCE   = $(TARGET).c $(CLASSES:%=%.c)
OBJECTS  = $(TARGET).o $(CLASSES:%=%.o)
HFILES   = $(CLASSES:%=%.h)
OPT      = -O2 -Wall
CFLAGS   = $(OPT) -I.
LDFLAGS  = -L/usr/lib -lpthread -lm
CCC      = gcc

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CCC) $(OPT) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY:

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)
	rm a3d

$(OBJECTS): $(HFILES)
