OBJS=objs
BIN=bin
DEBUG=$(BIN)/debug
RELEASE=$(BIN)/release

#C compiler
CCOMP=gcc
HEADERS=./headers
WFLAGS=-pedantic-errors -Werror -Wall -Wextra -Wconversion -Wsign-conversion
FLAGS=-I$(HEADERS) $(WFLAGS) -pthread
DEBUGFLAGS=-ggdb
RELEASEFLAGS=-O2 -D NDEBUG

#Module languages specifications
CMODULE_NAMES :=sockets obep
CSERVER := $(RELEASE)/server
DEBUGCSERVER := $(DEBUG)/server
#Test
CTESTS :=$(patsubst %,$(DEBUG)/%.test,$(CMODULE_NAMES))
TESTS :=$(CTESTS)

#Main targets
DACSC:	cserver cclient tests


cserver:	$(CSERVER)/server


cclient:

# Main target object files

$(CSERVER)/server:	$(OBJS)/server.o $(OBJS)/socket.o $(OBJS)/obep.o
			$(CCOMP) $(FLAGS) $(RELEASEFLAGS) -o $@

# Module release object files

$(OBJS)/sockets.o:	sockets/sockets.c $(HEADERS)/sockets.h
			$(CCOMP) $(FLAGS) $(RELEASEFLAGS) -c $< -o $@

$(OBJS)/obep.o:	obep/obep.c $(HEADERS)/obep.h $(HEADERS)/entities
			$(CCOMP) $(FLAGS) $(RELEASEFLAGS) -c $< -o $@

tests:	$(TESTS) $(DEBUGCSERVER)/server.test


# Target for the object file of the module in debug mode
.SECONDEXPANSION:
$(OBJS)/debug_%.o:	MODNAME =$(patsubst $(OBJS)/debug_%.o,%,$@)

$(OBJS)/debug_%.o:	$$(MODNAME)/$$(MODNAME).c $(HEADERS)/%.h
			$(CCOMP) $(FLAGS) $(DEBUGFLAGS) -c $< -o $@


# Target for the object file of the module's testing program
.SECONDEXPANSION:
$(OBJS)/test_%.o:	MODNAME =$(patsubst $(OBJS)/test_%.o,%,$@)

$(OBJS)/test_%.o:	$$(MODNAME)/test.c $(HEADERS)/$$(MODNAME).h
					$(CCOMP) $(FLAGS) $(DEBUGFLAGS) -c $< -o $@

# Special object files that depend on more than just the module

$(OBJS)/debug_obep.o:	obep/obep.c $(HEADERS)/obep.h $(HEADERS)/entities.h
					$(CCOMP) $(FLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJS)/test_obep.o:	obep/test.c $(HEADERS)/obep.h $(HEADERS)/entities.h
					$(CCOMP) $(FLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJS)/debug_server.o:	server/server.c $(HEADERS)/entities.h $(HEADERS)/sockets.h $(HEADERS)/obep.h
					$(CCOMP) $(FLAGS) $(DEBUGFLAGS) -c $< -o $@

# Module testing targets

$(DEBUG)/sockets.test:	$(OBJS)/debug_sockets.o $(OBJS)/test_sockets.o
			$(CCOMP) $(FLAGS) $(DEBUGFLAGS) $^ -o $(DEBUG)/sockets.test

$(DEBUG)/obep.test:		$(OBJS)/debug_obep.o $(OBJS)/test_obep.o
			$(CCOMP) $(FLAGS) $(DEBUGFLAGS) $^ -o $(DEBUG)/obep.test

$(DEBUGCSERVER)/server.test:	$(OBJS)/debug_server.o $(OBJS)/debug_obep.o $(OBJS)/debug_sockets.o
			$(CCOMP) $(FLAGS) $(DEBUGFLAGS) $^ -o $@

#phony targets
.PHONY:	clean clobber newmod

clean:
		rm -f $(OBJS)/*.o

clobber: clean
		rm -f $(RELEASE)/*
		rm -f $(DEBUG)/*



newmod:	NAME_UP = $(shell echo $(NAME) | tr  '[:lower:]' '[:upper:]')

newmod:	
ifndef NAME
	echo "No name specified"
else
ifndef LANG
	echo "No language specified"
else
ifeq ($(LANG), C)
				cp -r CModuleTemplate $(NAME)
				touch $(HEADERS)/$(NAME).h
				echo $(NAME_UP)
				echo -e "#ifndef DACSC__$(NAME_UP)_H_\n#define DACSC__$(NAME_UP)_H_\n\n\n#endif\n" > $(HEADERS)/$(NAME).h
				mv $(NAME)/main.c $(NAME)/$(NAME).c
				echo -e "#include \"$(NAME).h\"\n\n" > $(NAME)/$(NAME).c
				echo -e "#include <stdio.h>\n#include <stdlib.h>\n#include \"$(NAME).h\"\n\nint main(int argc, char* argv[])\n{\n\n}\n" >> $(NAME)/test.c
else
				echo "Language unsupported"
endif
endif
endif
