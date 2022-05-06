
#This is a template to build your own project with the e-puck2_main-processor folder as a library.
#Simply adapt the lines below to be able to compile

# Define project name here
PROJECT = Mini_Projet

#Define path to the e-puck2_main-processor folder
GLOBAL_PATH = C:\Users\duval\Documents\EPFL\BA6\Robotique\Projet\lib\e-puck2_main-processor

#Source files to include
CSRC += ./main.c \
		./motor_control.c \
		./process_image.c \

#Header folders to include
INCDIR +=

#Jump to the main Makefile
include $(GLOBAL_PATH)/Makefile
