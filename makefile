# $@ - left side of ':'
# $^ - right side of ':'
# $< - first of dependencies

# CFG
CC = clang++
SRCDIR = src
IDIR = libs/include
ODIR = obj
BINDIR = bin
LIBS = -lm -L libs/lib -l OpenCL
EXECNAME = cnn.exe

CFLAGS = -std=c++11 \
	-c \
	-Wall \
	-Wextra \
	-stdlib=libstdc++ \
	-isystem "C:\programs\install\MinGW\include" \
	-isystem "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++" \
	-isystem "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++\mingw32" \
	-I$(IDIR)

LFLAGS = -std=c++11 \
	-l "stdc++" \
	-I$(IDIR)

_OBJ = Main_cl.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ)) # append ODIR to each entry


# default target
$(EXECNAME): $(OBJ)
	@echo Linking..
	g++ -o $(BINDIR)/$@ $^ $(LFLAGS) $(LIBS)

compile: $(OBJ)
	@echo --end--

link: $(EXECNAME)
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
