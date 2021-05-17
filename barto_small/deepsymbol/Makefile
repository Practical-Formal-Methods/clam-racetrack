all:pipe deepsymbol middleware

FLAGS = -std=c++11 -U__STRICT_ANSI__ -UNDEBUG -O3 -g -fPIC \
        -Wcast-qual -Wswitch -Wall -Wextra -Wundef -Wcast-align -Wno-unused -Wno-unused-parameter -Wno-unused-function \
        -lgmp -lgmpxx -lmpfr -lm -lboost_program_options\
        -I/apron/install/include \
        -L/apron/install/lib \
        -lapron -lapronxx -lboxD\
        -D HAS_APRON

middleware: middleware.cpp
	g++  $^  $(FLAGS) -D BDOMAIN -D ABST=Abst_symV -D NUM_D -o $@

pipe: piping.cpp
	g++ -std=c++11 $^ -o $@

absBoxD: main.cpp
	g++  $^ $(FLAGS) -D BDOMAIN -D ABST=Abst_apron -D NUM_D -o $@
deepsymbol: main.cpp
	g++  $^  $(FLAGS) -D BDOMAIN -D ABST=Abst_symV -D NUM_D -o $@

absEZonoD: main.cpp
	g++  $^  $(FLAGS) -lzonotope -D EZDOMAIN -D ABST=Abst_apron -D NUM_D -o $@
absEZonoDSym: main.cpp
	g++  $^  $(FLAGS) -lzonotope -D EZDOMAIN -D ABST=Abst_symV -D NUM_D -o $@

absT1pD: main.cpp
	g++  $^  $(FLAGS) -lt1pD -D TDOMAIN -D ABST=Abst_apron -D NUM_D -o $@
absT1pDSym: main.cpp
	g++  $^  $(FLAGS) -lt1pD -D TDOMAIN -D ABST=Abst_symV -D NUM_D -o $@

clean:
	rm pipe deepsymbol middleware

