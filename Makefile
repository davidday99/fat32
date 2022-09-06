SRC = src
INC = inc
OBJ = obj
ODIR = bin
ONAME = fat32
PROFNAME = prof_output

SRCS = $(wildcard $(SRC)/*.c)
DEBUGOBJS = $(addprefix $(OBJ)/debug/, $(notdir $(SRCS:.c=.o)))
RELEASEOBJS = $(addprefix $(OBJ)/release/, $(notdir $(SRCS:.c=.o)))
PROFOBJS = $(addprefix $(OBJ)/prof/, $(notdir $(SRCS:.c=.o)))

CC = gcc
DEBUGGER = gdb-multiarch
PROFILER = gprof
RM = rm -rf
MKDIR = @mkdir -p $(@D)
CFLAGS = -I$(INC) -MMD -MP -Wall
DEBUGFLAGS = -g3
RELEASEFLAGS = -Os
PROFLAGS = -pg

release: $(ODIR)/release/$(ONAME)

debug: $(ODIR)/debug/$(ONAME)

profile: $(ODIR)/prof/$(ONAME)
	./$(ODIR)/prof/$(ONAME)
	$(PROFILER) $(ODIR)/prof/$(ONAME) gmon.out > $(PROFNAME)
	
clean:
	-$(RM) $(OBJ)
	-$(RM) $(ODIR)

$(ODIR)/debug/$(ONAME): $(DEBUGOBJS)
	$(MKDIR)
	$(CC) -o $(ODIR)/debug/$(ONAME) $^ $(CFLAGS) $(DEBUGFLAGS) 
	ctags -R *

$(OBJ)/debug/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(DEBUGFLAGS)

$(ODIR)/release/$(ONAME): $(RELEASEOBJS)
	$(MKDIR)
	$(CC) -o $(ODIR)/release/$(ONAME) $^ $(CFLAGS) $(RELEASEFLAGS) 
	ctags -R *

$(OBJ)/release/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS) 

$(ODIR)/prof/$(ONAME): $(PROFOBJS)
	$(MKDIR)
	$(CC) -o $(ODIR)/prof/$(ONAME) $^ $(CFLAGS) $(RELEASEFLAGS) $(PROFLAGS) 
	ctags -R *

$(OBJ)/prof/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS) $(PROFLAGS)

-include $(DEBUGOBJS:.o=.d)
-include $(RELEASEOBJS:.o=.d)


.PHONY: clean debug

