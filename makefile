# $@ - left side of ':'
# $^ - right side of ':'
# $< - first of dependencies

CC = clang++
VPATH = src/opencl src
IDIR = libs/include
ODIR = obj
BINDIR = bin
LIBS = -lm -L libs/lib -l OpenCL
EXECNAME = cnn.exe

CFLAGS = -std=c++11 \
	-c \
	-g \
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

_OBJ = Main_cl.o Context.o UtilsOpenCL.o Kernel.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ)) # append ODIR to each entry



# default target
build: $(EXECNAME)

compile: $(OBJ)

run: $(EXECNAME)
	@echo -----------------------
	@$(BINDIR)/$<

clean:
	rm -f $(ODIR)/*.o
	rm -f $(BINDIR)/*


$(EXECNAME): $(OBJ)
	@echo Linking..
	g++ -o $(BINDIR)/$@ $^ $(LFLAGS) $(LIBS)

$(ODIR)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)
