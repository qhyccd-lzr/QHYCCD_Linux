# @source      Makefile
# @description makefile for the application

# call with "make DEBUG=1" to build with debug

ifdef DEBUG
FLAG_DEBUG=-g
endif

# sources directories
srcdir=.
libdir=/usr/lib
incdir=/usr/include/libqhyccd/
vpath %.c $(srcdir)
vpath %.h $(srcdir) : $(incdir) 

main=IC8300
bin=$(main)

includes= common.h

objects= $(main).o

CFLAGS += -Wall -Wextra -fpack-struct -Wno-main
# -Wall: emits many many warnings
# -Wextra: emits even more warnings
# -Werror: treats warnings as errors
# -Wno-main: allows "void main"

# Opencv support
LDFLAGS+= `pkg-config --libs --cflags opencv`

# rule for compilation of c files
%.o: %.c $(includes)
	@echo
	@echo "* compiling $<"
	$(CC) -I$(incdir) -c $(FLAG_DEBUG) $(CFLAGS) $(CPPFLAGS) $<
	@echo

# creation of executable binary
$(bin): $(objects)
	@echo
	@echo "* linking $@"
	$(CC) -o $@ $^ $(LDFLAGS) -lqhyccd -lusb-1.0
	@echo

# removing files to force rebuilding
.PHONY: clean
clean:
	@echo
	@echo "* cleaning files"
	@-rm $(bin) $(objects) > /dev/null 2>&1
	@echo

# running the application
.PHONY: run
run:
	@echo
	@echo "* running application"
	@LD_LIBRARY_PATH=$(libdir):$LD_LIBRARY_PATH ./$(bin)
	@echo "* end of application"

# the end
