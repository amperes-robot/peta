ARDUINO?=arduino
MAIN="sw.ino"

ifneq (, $(findstring CYGWIN, $(shell uname)))
	MAIN:="$(shell cygpath -wa $(MAIN))" else
endif

.PHONY: all verify upload sim unsim test untest

all: verify

verify: unsim untest
	$(ARDUINO) --verify $(MAIN)

upload: unsim untest
	$(ARDUINO) --upload $(MAIN)

sim:
	for x in *_SIM; do cp $$x $$x.cpp; done
	g++ *.cpp -Os -g -D"IO_SIM" -o simulator -pthread --std=c++11 -Wall -Wextra -Wshadow -Wstrict-aliasing -Werror $(shell pkg-config --cflags --libs gtkmm-3.0)
# cd simulator && xbuild simulator.sln
# cp simulator/bin/Debug/simulator.exe bin/simulator

test:
	for x in *_TST; do cp $$x $$x.cpp; done
	g++ *.cpp -Os -g -D"IO_SIM" -D"IO_TST" -o test -pthread --std=c++11 -Wall -Wextra -Wshadow -Wstrict-aliasing -Werror
	./test
	rm test

untest:
	rm -f *_TST.cpp

unsim:
	rm -f *_SIM.cpp
