HDR_DIR = ./hdrs
SRC_DIR = ./srcs
OBJ_DIR = ./objs

IGNORE := $(SRC_DIR)/Sim.cpp

CC_FILES := $(filter-out $(IGNORE), $(wildcard $(SRC_DIR)/*.cpp))

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CC_FILES))

TARGETS = Sim
DEFAULT_TGT = Sim

CC=g++
CFLAGS=-std=c++17 
CVINC=-I/usr/local/include/opencv4/
CVLIBS= -lopencv_highgui -lopencv_videoio -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_objdetect -lopencv_calib3d
INC=-I$(HDR_DIR) $(CVINC) 

all: $(DEFAULT_TGT)

Sim: $(OBJ_FILES)
	$(CC) $(CFLAGS) $(INC) $(SRC_DIR)/$@.cpp -o $@ $^ $(CVLIBS)

$(OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(CVLIBS)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

noexec:
	rm -f $(TARGETS)
clean:
	rm -f $(TARGETS) ./$(OBJ_DIR)/*.o

