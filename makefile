CC	= g++
CFLAGS	= -Wall -g

MASTER_SRC = main.cpp 
SRC = parser.h tree.h sem.h
SHARE_OBJ = parser.o tree.o sem.o
CP11 = -std=c++11
MASTER_OBJ = $(MASTER_SRC:.cpp=.o)
OBJ	= $(SRC:.h=.o)

TARGET	= P4

OUTPUT = $(TARGET)
all: $(OUTPUT)

%.o: %.cpp $(SRC)
	$(CC) $(CP11) $(CFLAGS) -c $< -o $@

$(TARGET): $(MASTER_OBJ) $(SHARE_OBJ)
	$(CC) $(CP11) $(CFLAGS) $(MASTER_OBJ) $(OBJ) -o $(TARGET)


clean:
	/bin/rm -f *.o $(TARGET)
	/bin/rm -f *.preorder *.postorder *.inorder
