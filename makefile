# $@ - left side of ':'
# $^ - right side of ':'
# $< - first of dependencies

# CFG
CC = g++
SRCDIR = src
IDIR = lib
ODIR = obj
BINDIR = bin
LIBS = -lm
EXECNAME = cnn.exe

CFLAGS = -std=c++11 \
	-I$(IDIR)

_OBJ = atom_test.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ)) # append ODIR to each entry


# default target
$(EXECNAME): $(OBJ)
	@echo Linking..
	$(CC) -o $(BINDIR)/$@ $^ $(CFLAGS) $(LIBS)

$(ODIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

run: $(EXECNAME)
	$(BINDIR)/$<

clean:
	rm -f $(ODIR)/*.o
	rm -f $(BINDIR)/*

test:
	echo test
