OUTPUT = mftp
BUILD_FLAG = -I ./include
cc = gcc

ifeq ($(target), exe)
	ifeq ($(debug), y)
		LINK_FLAG = -L ./libs -lmlog.debug
		CFLAG = -g -Wall
		OBJS_FLAGS = $(CFLAG) $(BUILD_FLAG)
		OUTPUT = mftp.debug
    else
        LINK_FLAG = -L ./libs -lmlog
        CFLAG = -o2
        OBJS_FLAGS = $(CFLAG) $(BUILD_FLAG)
    endif
    #$(join $(output), .debug)
else ifeq ($(target), a)

else ifeq ($(target), so)

endif

SRCS = $(wildcard *.c)
OBJS = $(patsubst %c, %o, $(SRCS))
OUTPUT_OBJS = mftp.o

ALL:$(OUTPUT)
    
$(OUTPUT):$(OBJS) 
	@$(cc) $(OUTPUT_OBJS) $(LINK_FLAG) -o $(OUTPUT) 

%.o:%.c
	@$(cc) $(OBJS_FLAGS) -c $< -o $@
   
.PHONY: clean
clean:
	@rm -rf *.o *.a *.so mftp mftp.debug
