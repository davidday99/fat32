SRC = src
TEST = test
INC = inc
OBJ = obj
ODIR = bin
ONAME = libfat32
PROFNAME = prof_output

SRCS = $(wildcard $(SRC)/*.c)
TESTS = $(wildcard $(TEST)/*.c)
RELEASEOBJS = $(addprefix $(OBJ)/release/, $(notdir $(SRCS:.c=.o)))
DEBUGOBJS = $(addprefix $(OBJ)/debug/, $(notdir $(SRCS:.c=.o)))
TESTOBJS = $(addprefix $(OBJ)/test/, $(notdir $(TESTS:.c=.o)))
PROFOBJS = $(addprefix $(OBJ)/prof/, $(notdir $(SRCS:.c=.o)))

CC = gcc
DEBUGGER = gdb-multiarch
AR = ar
PROFILER = gprof
RM = rm -rf
MKDIR = @mkdir -p $(@D)
CFLAGS = -I$(INC) -MMD -MP -Wall
RELEASEFLAGS = -Os
ARCHIVEFLAGS = rcs
DEBUGFLAGS = -g3
PROFLAGS = $(RELEASEFLAGS) -pg

release: $(ODIR)/release/$(ONAME)

debug: $(ODIR)/debug/$(ONAME)

test: $(ODIR)/test/$(ONAME)

profile: $(ODIR)/prof/$(ONAME)
	./$(ODIR)/prof/$(ONAME)
	$(PROFILER) $(ODIR)/prof/$(ONAME) gmon.out > $(PROFNAME)
	
clean:
	-$(RM) $(OBJ)
	-$(RM) $(ODIR)

$(ODIR)/release/$(ONAME): $(RELEASEOBJS)
	$(MKDIR)
	$(AR) $(ARCHIVEFLAGS) $(ODIR)/release/$(ONAME).a $^
	ctags -R *

$(ODIR)/debug/$(ONAME): $(DEBUGOBJS) $(TESTOBJS)
	$(MKDIR)
	$(CC) -o $(ODIR)/debug/$(ONAME) $^ $(CFLAGS) $(DEBUGFLAGS) 
	ctags -R *

$(ODIR)/test/$(ONAME): $(RELEASEOBJS) $(TESTOBJS) 
	$(MKDIR)
	$(CC) -o $(ODIR)/test/$(ONAME) $^ $(CFLAGS) $(RELEASEFLAGS)
	ctags -R *

$(ODIR)/prof/$(ONAME): $(PROFOBJS) $(TESTOBJS)
	$(MKDIR)
	$(CC) -o $(ODIR)/prof/$(ONAME) $^ $(CFLAGS) $(RELEASEFLAGS) $(PROFLAGS) 
	ctags -R *

$(OBJ)/release/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS) 

$(OBJ)/debug/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(DEBUGFLAGS)

$(OBJ)/debug/%.o: $(TEST)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(DEBUGFLAGS)

$(OBJ)/test/%.o: $(TEST)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS)

$(OBJ)/prof/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS) $(PROFLAGS)
	
$(OBJ)/prof/%.o: $(TEST)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS) $(PROFLAGS)


-include $(DEBUGOBJS:.o=.d)
-include $(RELEASEOBJS:.o=.d)
-include $(TESTOBJS:.o=.d)
-include $(PROFOBJS:.o=.d)

.PHONY: clean debug

