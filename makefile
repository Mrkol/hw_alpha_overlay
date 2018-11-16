CXX = clang-6.0
CXXFLAGS = -m32 -msse4.2 -Wall -Wextra -Werror -ggdb -std=c11

SRC_DIR = ./src
BIN_DIR = ./bin

REP_NAME = https://github.com/Mrkol/hw_alpha_overlay
AUTOGEN_MSG = // You are NOT supposed to look at this automatically generated file!!!\n// Please, refer to this repository instead: $(REP_NAME).

SOURCES = $(SRC_DIR)/bmp.c

main: init $(SRC_DIR)/main.c $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/main.c $(SOURCES) -o $(BIN_DIR)/main

singlefile: init
	echo "$(AUTOGEN_MSG)" > $(BIN_DIR)/main.c
	cat $(SRC_DIR)/bmp.h $(SRC_DIR)/bmp.c\
		$(SRC_DIR)/main.c\
		>> $(BIN_DIR)/main.c

	sed -i '/#include "/d' $(BIN_DIR)/main.c

init:
	mkdir -p $(BIN_DIR)
