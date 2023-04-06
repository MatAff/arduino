# Copy shared section of ino file
sed -n '/SHARED/,/SHARED/p' pm_skulls.ino > ./srcs/LightController.cpp

# make clean

make SimSkull

./SimSkull
