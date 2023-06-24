# Copy shared section of ino file
sed -n '/SHARED/,/SHARED/p' *.ino > ./srcs/Controller.cpp
sed -i 's/max(/std::max(/g' ./srcs/Controller.cpp

make clean

make Sim

./Sim
