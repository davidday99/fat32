SRC = src
TEST = test
REL = release
DEBUG = debug
PROF = prof
BACKEND = backend
INC = inc
OBJ = obj
ODIR = build
ONAME = libfat32
ANAME = libfat32.a
PROFNAME = prof_output

SRCS = $(wildcard $(SRC)/*.c)
TESTS = $(wildcard $(TEST)/*.c)
DEVS = $(wildcard $(BACKEND)/*.c)
RELEASEOBJS = $(addprefix $(OBJ)/$(REL)/, $(notdir $(SRCS:.c=.o)))
DEBUGOBJS = $(addprefix $(OBJ)/$(DEBUG)/, $(notdir $(SRCS:.c=.o)))
TESTOBJS = $(addprefix $(OBJ)/$(TEST)/, $(notdir $(TESTS:.c=.o)))
PROFOBJS = $(addprefix $(OBJ)/$(PROF)/, $(notdir $(SRCS:.c=.o)))
DEVOBJS = $(addprefix $(OBJ)/$(BACKEND)/, $(notdir $(DEVS:.c=.o)))

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
TESTFLAGS = -O0 -g3
PROFLAGS = $(RELEASEFLAGS) -pg

release: $(ODIR)/$(REL)/$(ANAME)

debug: $(ODIR)/$(DEBUG)/$(ONAME)

test: $(ODIR)/$(TEST)/$(ONAME)

profile: $(ODIR)/$(PROF)/$(ONAME)
	./$(ODIR)/$(PROF)/$(ONAME)
	$(PROFILER) $(ODIR)/$(PROF)/$(ONAME) gmon.out > $(PROFNAME)

backend: $(ODIR)/$(BACKEND)/$(ONAME) 

clean:
	-$(RM) $(OBJ)
	-$(RM) $(ODIR)

$(ODIR)/$(REL)/$(ANAME): $(RELEASEOBJS)
	$(MKDIR)
	$(AR) $(ARCHIVEFLAGS) $(ODIR)/$(REL)/$(ANAME) $^
	ctags -R *

$(ODIR)/$(DEBUG)/$(ONAME): $(DEBUGOBJS) $(TESTOBJS)
	$(MKDIR)
	$(CC) -o $(ODIR)/$(DEBUG)/$(ONAME) $^ $(CFLAGS) $(DEBUGFLAGS) 
	ctags -R *

$(ODIR)/$(TEST)/$(ONAME): $(RELEASEOBJS) $(TESTOBJS) 
	$(MKDIR)
	$(CC) -o $(ODIR)/$(TEST)/$(ONAME) $^ $(CFLAGS) $(TESTFLAGS)
	ctags -R *

$(ODIR)/$(PROF)/$(ONAME): $(PROFOBJS) $(TESTOBJS)
	$(MKDIR)
	$(CC) -o $(ODIR)/$(PROF)/$(ONAME) $^ $(CFLAGS) $(RELEASEFLAGS) $(PROFLAGS) 
	ctags -R *

$(ODIR)/$(BACKEND)/$(ONAME): $(DEVOBJS) $(ODIR)/$(REL)/$(ANAME)
	$(MKDIR)
	$(CC) -o $(ODIR)/$(BACKEND)/$(ONAME) $^ $(CFLAGS) $(RELEASEFLAGS)

$(OBJ)/$(REL)/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS) 

$(OBJ)/$(DEBUG)/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(DEBUGFLAGS)

$(OBJ)/$(DEBUG)/%.o: $(TEST)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(DEBUGFLAGS)

$(OBJ)/$(TEST)/%.o: $(TEST)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(TESTFLAGS)

$(OBJ)/$(PROF)/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS) $(PROFLAGS)
	
$(OBJ)/$(PROF)/%.o: $(TEST)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS) $(PROFLAGS)

$(OBJ)/$(BACKEND)/%.o: $(BACKEND)/%.c
	$(MKDIR)
	$(CC) -o $@ $< -c $(CFLAGS) $(DEBUGFLAGS)

-include $(DEBUGOBJS:.o=.d)
-include $(RELEASEOBJS:.o=.d)
-include $(TESTOBJS:.o=.d)
-include $(PROFOBJS:.o=.d)

.PHONY: clean 

