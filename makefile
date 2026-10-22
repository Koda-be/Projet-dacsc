OBJS=objs
BIN=bin
DEBUG=$(BIN)/debug
RELEASE=$(BIN)/release
CMAKEBUILD=/tmp/cmakebuild

#C compiler
CCOMP=gcc
HEADERS=./headers
WFLAGS=-pedantic-errors -Werror -Wall -Wextra -Wconversion -Wsign-conversion
FLAGS=-I$(HEADERS) $(WFLAGS)
DEBUGFLAGS=-ggdb
RELEASEFLAGS=-O2 -D NDEBUG

#Main target names
CSERVER=cserver
CCLIENT=ClientBookEncoderQt

#Module languages specifications
CMODULE_NAMES :=sockets
CMODULES := $(patsubst %,$(RELEASE)/%,$(CMODULE_NAMES))

#Test
CTESTS :=$(patsubst %,$(DEBUG)/%.test,$(CMODULE_NAMES))
TESTS :=$(CTESTS)

#Main and generic targets
DACSC:	$(RELEASE)/$(CSERVER) $(RELEASE)/$(CCLIENT) tests


$(RELEASE)/$(CSERVER):	

# Probably very stupid to call cmake inside a make target but oh well
$(RELEASE)/$(CCLIENT):	$(CCLIENT)/* $(OBJS)/sockets.o
			cmake -S $(CCLIENT) -B $(CMAKEBUILD)/$(CCLIENT)
			cmake --build $(CMAKEBUILD)/$(CCLIENT)
			mv $(CMAKEBUILD)/$(CCLIENT)/$(CCLIENT) $(RELEASE)


$(CMODULES):	$(OBJS)/$@.o
		$(CCOMP) $(FLAGS) $(DEBUGFLAGS) $^ -o $@.out


tests:	$(TESTS)


# Target for the object file of the module in release mode
.SECONDEXPANSION:
$(OBJS)/%.o:	MODNAME =$(patsubst $(OBJS)/%.o,%,$@)

$(OBJS)/%.o:	$$(MODNAME)/$$(MODNAME).c $(HEADERS)/%.h
			$(CCOMP) $(FLAGS) $(RELEASEFLAGS) -c $< -o $@


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



# Module testing targets

$(DEBUG)/sockets.test:	$(OBJS)/debug_sockets.o $(OBJS)/test_sockets.o
			$(CCOMP) $(FLAGS) $(DEBUGFLAGS) $^ -o $(DEBUG)/sockets.test


#phony targets
.PHONY:	clean clobber newmod

clean:
		rm -f $(OBJS)/*.o
		rm -rf $(CMAKEBUILD)/*

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
