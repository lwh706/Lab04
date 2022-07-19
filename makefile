CC=gcc
SRC_DIR=src
BIN_DIR=bin
LIB_DIR=lib
INCLUDE_DIR=include
CFLAG=-Wall -m64

.PHONY: clean rebuild

all: Lab04_Requestor Lab04_Replyer

Lab04_Requestor:
	${CC} ${CFLAG} ${SRC_DIR}/Lab04_Requestor.c -o ${BIN_DIR}/Lab04_Requestor

Lab04_Replyer:
	${CC} ${CFLAG} ${SRC_DIR}/Lab04_Replyer.c -o ${BIN_DIR}/Lab04_Replyer

clean:
	-rm -rf ${BIN_DIR}/Lab04_Requestor
	-rm -rf ${BIN_DIR}/Lab04_Replyer

rebuild: clean all
