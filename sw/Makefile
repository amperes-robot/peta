ARDUINO?=arduino
MAIN="sw.ino"

ifneq (, $(findstring CYGWIN, $(shell uname)))
	MAIN:="$(shell cygpath -wa $(MAIN))"
else
endif

.PHONY: all verify upload sim unsim

all: verify

verify: unsim
	$(ARDUINO) --verify $(MAIN)

upload: unsim
	$(ARDUINO) --upload $(MAIN)

sim:
	mkdir bin -p
	for x in *_SIM; do cp $$x $$x.cpp; done
	g++ *.cpp -Os -g -D"IO_SIM" -o bin/sw -pthread --std=c++11 -Wall -Wextra -Wshadow -Wstrict-aliasing -Werror
	cd simulator && xbuild simulator.sln
	cp simulator/bin/Debug/simulator.exe bin/simulator

unsim:
	rm -f *_SIM.cpp
