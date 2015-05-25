# $@ - left side of ':'
# $^ - right side of ':'
# $< - first of dependencies

# CFG
CC = clang++
SRCDIR = src
IDIR = libs/include
ODIR = obj
BINDIR = bin
LIBS = -lm -L libs\lib -l OpenCL.lib
EXECNAME = cnn.exe

CFLAGS = -std=c++11 \
	-c \
	-stdlib=libstdc++ \
	-I "C:\programs\install\MinGW\include" \
	-I "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++" \
	-I "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++\mingw32" \
	-I$(IDIR)

_OBJ = atom_test.o Main_cl.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ)) # append ODIR to each entry


# default target
$(EXECNAME): $(OBJ)
	@echo Linking..
	$(CC) -o $(BINDIR)/$@ $^ $(CFLAGS) $(LIBS)

compile: $(OBJ)
	@echo --end--

$(ODIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

run: $(EXECNAME)
	$(BINDIR)/$<

clean:
	rm -f $(ODIR)/*.o
	rm -f $(BINDIR)/*

test:
	echo test
