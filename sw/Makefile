ARDUINO?=arduino
MAIN=sw.pde

.PHONY: all verify upload sim unsim

all: verify

verify: unsim
	$(ARDUINO) --verify $(MAIN)

upload: unsim
	$(ARDUINO) --upload $(MAIN)

sim:
	for x in *_SIM; do cp $$x $$x.cpp; done
	g++ *.cpp -Os -g -D"IO_SIM" -o sw --std=c++0x -Wall -Wextra -Wshadow -Wstrict-aliasing -Werror

unsim:
	rm -f *_SIM.cpp
