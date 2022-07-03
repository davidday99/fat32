SRC = src
INC = inc
OBJ = obj
ODIR = bin
ONAME = fat32

SRCS = $(wildcard $(SRC)/*.c)
DEBUGOBJS = $(addprefix $(OBJ)/debug/, $(notdir $(SRCS:.c=.o)))
RELEASEOBJS = $(addprefix $(OBJ)/release/, $(notdir $(SRCS:.c=.o)))


CC = gcc
DEBUGGER = gdb-multiarch
RM = rm -rf
MKDIR = @mkdir -p $(@D)
CFLAGS = -I$(INC) -MMD -MP -Wall
DEBUGFLAGS = -g3
RELEASEFLAGS = -Os

release: $(ODIR)/release/$(ONAME)
 

debug: $(ODIR)/debug/$(ONAME)
	

clean:
	-$(RM) $(OBJ)
	-$(RM) $(ODIR)

$(ODIR)/debug/$(ONAME): $(DEBUGOBJS)
	$(MKDIR)
	$(CC) -o $(ODIR)/debug/$(ONAME) $^ $(DEBUGFLAGS) 

$(OBJ)/debug/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(DEBUGFLAGS)

$(ODIR)/release/$(ONAME): $(RELEASEOBJS)
	$(MKDIR)
	$(CC) -o $(ODIR)/release/$(ONAME) $^ $(RELEASEFLAGS) 

$(OBJ)/release/%.o: $(SRC)/%.c
	$(MKDIR)   
	$(CC) -o $@ $< -c $(CFLAGS) $(RELEASEFLAGS)

-include $(OBJS:.o=.d)

.PHONY: clean debug

