# Copy shared section of ino file
sed -n '/SHARED/,/SHARED/p' *.ino > ./srcs/Controller.cpp

# make clean

make Sim

./Sim
