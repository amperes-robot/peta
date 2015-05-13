BACKEND=TINAH
ARDUINO?=arduino
MAIN=sw.pde

.PHONY: all verify upload move unmove

all: move
	$(ARDUINO) --verify $(MAIN)

verify: move

upload: move
	$(ARDUINO) --upload $(MAIN)

move:
	cp $(BACKEND) io.cpp
