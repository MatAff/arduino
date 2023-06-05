# Copy shared section of ino file
sed -n '/SHARED/,/SHARED/p' pm_snake.ino > ./cpp/srcs/Controller.cpp
sed -i 's#// UNCOMMENT FOR CPP ##' ./cpp/srcs/Controller.cpp

# Add some things we need in the cpp version
# echo -e "long micros() { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }\n$(cat ./cpp/srcs/Controller.cpp)" > ./cpp/srcs/Controller.cpp
# echo -e "#include <chrono>\n$(cat ./cpp/srcs/Controller.cpp)" > ./cpp/srcs/Controller.cpp
# echo -e "#include <cmath>\n$(cat ./cpp/srcs/Controller.cpp)" > ./cpp/srcs/Controller.cpp

cd cpp
make clean
make Sim
./Sim