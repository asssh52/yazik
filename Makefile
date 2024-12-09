CXX_FLAGS = -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations \
   -Wmissing-declarations -Wcast-qual -Wchar-subscripts                             \
   -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal           \
   -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline                   \
   -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked                     \
   -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo           \
   -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn                         \
   -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default               \
   -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast                    \
   -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing            \
   -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation             \
   -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192                  \
   -Wstack-usage=8192 -fsanitize=address -fsanitize=undefined -fPIE -Werror=vla -Wno-format

CXX = clang++


run:      ./bin/main.o ./bin/front.o ./bin/dump.o
	$(CXX) ./bin/main.o ./bin/front.o ./bin/dump.o $(CXXFLAGS) -o main

./bin/main.o:  src/main.cpp
	$(CXX) -c ./src/main.cpp $(CXXFLAGS) -o ./bin/main.o

./bin/front.o: src/front.cpp hpp/front.hpp
	$(CXX) -c ./src/front.cpp $(CXXFLAGS) -o ./bin/front.o

./bin/dump.o: src/dump.cpp hpp/dump.hpp src/front.cpp hpp/front.hpp
	$(CXX) -c ./src/dump.cpp $(CXXFLAGS) -o ./bin/dump.o
clean:
	rm -f main ./bin/*.o ./bin/png/*.png
