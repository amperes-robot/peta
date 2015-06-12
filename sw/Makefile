ARDUINO?=arduino
MAIN="sw.ino"

ifneq (, $(findstring CYGWIN, $(shell uname)))
	MAIN:="$(shell cygpath -wa $(MAIN))" else
endif

.PHONY: all verify upload

all: verify

gui:
	$(ARDUINO) $(MAIN)

verify:
	$(ARDUINO) --verify $(MAIN)

upload:
	$(ARDUINO) --upload $(MAIN)
