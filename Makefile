FILE_NUM   = $(i)
COMMIT_MSG = ${m}
CPP_FILES  = ${wildcard *.cpp}
HDR_FILES  = ${wildcard include/*.hpp}
LAB        = Lab4
STUDENT_ID = 311581017

CXX_FLAGS  = -O3


main: ${CPP_FILES} ${HDR_FILES} routing.o structure.o
	@g++ ${CXX_FLAGS} main.cpp routing.o structure.o -o ${LAB}

routing.o: routing.cpp ${HDR_FILES}
	@g++ ${CXX_FLAGS} -c routing.cpp

structure.o: structure.cpp ${HDR_FILES}
	@g++ ${CXX_FLAGS} -c structure.cpp

clean:
	@rm *.o ${LAB}

test:
	@./${LAB} data/input${FILE_NUM}.in data/output${FILE_NUM}.out
	make plot

check:
	@./SolutionChecker data/input${FILE_NUM}.in data/output${FILE_NUM}.out

plot:
	python3 routing.py data/input${FILE_NUM}.in data/output${FILE_NUM}.out ${FILE_NUM}

cm:
	git add .
	git commit -m "${COMMIT_MSG}"

push:
	git push origin master

gen:
	@python3 generator.py