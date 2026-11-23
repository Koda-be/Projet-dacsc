OBJS=objs
BIN=bin
DEBUG=$(BIN)/debug
RELEASE=$(BIN)/release
CMAKEBUILD=$(abspath $(OBJS)/cmake)

#C compiler
CCOMP=gcc
HEADERS=./headers
WFLAGS=-pedantic-errors -Werror -Wall -Wextra -Wconversion -Wsign-conversion
FLAGS=-I$(HEADERS) $(WFLAGS) -pthread
MYSQL_CONFIG ?= $(shell for config in mysql_config mariadb_config; do command -v $$config && break; done)
MYSQL_PKG ?= $(shell for package in mysqlclient libmariadb mariadb; do pkg-config --exists $$package && { echo $$package; break; }; done)
MYSQL_CFLAGS ?= $(if $(MYSQL_PKG),$(shell pkg-config --cflags $(MYSQL_PKG)),$(if $(MYSQL_CONFIG),$(shell $(MYSQL_CONFIG) --cflags)))
MYSQL_LIBS ?= $(if $(MYSQL_PKG),$(shell pkg-config --libs $(MYSQL_PKG)),$(if $(MYSQL_CONFIG),$(shell $(MYSQL_CONFIG) --libs),-lmariadb))
DEBUGFLAGS=-ggdb
RELEASEFLAGS=-O2 -D NDEBUG

#Main target names
CCLIENT=ClientBookEncoderQt

#Module languages specifications
CMODULE_NAMES :=sockets obep
CSERVER := $(RELEASE)/server
DEBUGCSERVER := $(DEBUG)/server
#Test
CTESTS :=$(patsubst %,$(DEBUG)/%.test,$(CMODULE_NAMES))
TESTS :=$(CTESTS)

#Main targets
DACSC:	cserver cclient tests


cserver:	$(CSERVER)
cclient:	$(RELEASE)/$(CCLIENT)

# Probably very stupid to call cmake inside a make target but oh well
$(RELEASE)/$(CCLIENT): $(wildcard $(CCLIENT)/* headers/*.h) sockets/sockets.c obep/transport.c obep/obep.c
			mkdir -p $(@D)
			cmake -S $(CCLIENT) -B $(CMAKEBUILD)/$(CCLIENT)
			+cmake --build $(CMAKEBUILD)/$(CCLIENT)
			mv $(CMAKEBUILD)/$(CCLIENT)/$(CCLIENT) $(RELEASE)

$(CSERVER):	$(OBJS)/server.o $(OBJS)/sockets.o $(OBJS)/obep.o $(OBJS)/transport.o
			mkdir -p $(@D)
			$(CCOMP) $(FLAGS) $(RELEASEFLAGS) $^ $(MYSQL_LIBS) -o $@

$(OBJS)/transport.o: obep/transport.c $(HEADERS)/obep_transport.h $(HEADERS)/obep.h $(HEADERS)/sockets.h
	mkdir -p $(@D)
	$(CCOMP) $(FLAGS) $(RELEASEFLAGS) -c $< -o $@

# Main target object files

$(OBJS)/server.o:	server/server.c $(HEADERS)/obep.h $(HEADERS)/sockets.h $(HEADERS)/entities.h $(HEADERS)/obep_transport.h
			mkdir -p $(@D)
			$(CCOMP) $(FLAGS) $(MYSQL_CFLAGS) $(RELEASEFLAGS) -c $< -o $@

# Module release object files

$(OBJS)/sockets.o:	sockets/sockets.c $(HEADERS)/sockets.h
			mkdir -p $(@D)
			$(CCOMP) $(FLAGS) $(RELEASEFLAGS) -c $< -o $@

$(OBJS)/obep.o:	obep/obep.c $(HEADERS)/obep.h $(HEADERS)/entities.h
			mkdir -p $(@D)
			$(CCOMP) $(FLAGS) $(RELEASEFLAGS) -c $< -o $@

tests: $(TESTS) $(DEBUGCSERVER).test $(DEBUG)/obep-transport.test

check: $(DEBUG)/obep-transport.test
	$<

$(DEBUG)/obep-transport.test: obep/test_obep_transport.c $(OBJS)/transport.o $(OBJS)/debug_sockets.o $(HEADERS)/obep_transport.h
	mkdir -p $(@D)
	$(CCOMP) $(FLAGS) $(DEBUGFLAGS) $(filter-out %.h,$^) -o $@


# Target for the object file of the module in release mode
.SECONDEXPANSION:
$(OBJS)/%.o:	MODNAME =$(patsubst $(OBJS)/%.o,%,$@)

$(OBJS)/%.o:	$$(MODNAME)/$$(MODNAME).c $(HEADERS)/%.h
			mkdir -p $(@D)
			$(CCOMP) $(FLAGS) $(RELEASEFLAGS) -c $< -o $@


# Target for the object file of the module in debug mode
.SECONDEXPANSION:
$(OBJS)/debug_%.o:	MODNAME =$(patsubst $(OBJS)/debug_%.o,%,$@)

$(OBJS)/debug_%.o:	$$(MODNAME)/$$(MODNAME).c $(HEADERS)/%.h
			mkdir -p $(@D)
			$(CCOMP) $(FLAGS) $(DEBUGFLAGS) -c $< -o $@


# Target for the object file of the module's testing program
.SECONDEXPANSION:
$(OBJS)/test_%.o:	MODNAME =$(patsubst $(OBJS)/test_%.o,%,$@)

$(OBJS)/test_%.o:	$$(MODNAME)/test.c $(HEADERS)/$$(MODNAME).h
					mkdir -p $(@D)
					$(CCOMP) $(FLAGS) $(DEBUGFLAGS) -c $< -o $@

# Special object files that depend on more than just the module

$(OBJS)/debug_obep.o:	obep/obep.c $(HEADERS)/obep.h $(HEADERS)/entities.h
					mkdir -p $(@D)
					$(CCOMP) $(FLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJS)/test_obep.o:	obep/test.c $(HEADERS)/obep.h $(HEADERS)/entities.h
					mkdir -p $(@D)
					$(CCOMP) $(FLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJS)/debug_server.o: server/server.c $(HEADERS)/entities.h $(HEADERS)/sockets.h $(HEADERS)/obep.h $(HEADERS)/obep_transport.h
		mkdir -p $(@D)
		$(CCOMP) $(FLAGS) $(MYSQL_CFLAGS) $(DEBUGFLAGS) -c $< -o $@

# Module testing targets

$(DEBUG)/sockets.test:	$(OBJS)/debug_sockets.o $(OBJS)/test_sockets.o
			mkdir -p $(@D)
			$(CCOMP) $(FLAGS) $(DEBUGFLAGS) $^ -o $(DEBUG)/sockets.test

$(DEBUG)/obep.test:		$(OBJS)/debug_obep.o $(OBJS)/test_obep.o
			mkdir -p $(@D)
			$(CCOMP) $(FLAGS) $(DEBUGFLAGS) $^ -o $(DEBUG)/obep.test

$(DEBUGCSERVER).test:	$(OBJS)/debug_server.o $(OBJS)/debug_obep.o $(OBJS)/debug_sockets.o $(OBJS)/transport.o
			mkdir -p $(@D)
			$(CCOMP) $(FLAGS) $(DEBUGFLAGS) $^ $(MYSQL_LIBS) -o $@

#phony targets
.PHONY: DACSC cclient cserver tests check clean clobber newmod

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
