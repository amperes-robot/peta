ARDUINO ?= arduino
MAIN = "sw.ino"

CXX = avr-g++
CXX_FLAGS = -g -Os -Wall -Wextra -Werror -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=atmega128 -DF_CPU=16000000L -DARDUINO=10604 -DARDUINO_AVR_TINAH -DARDUINO_ARCH_AVR
LD_FLAGS = -Os -Wl,--gc-sections -mmcu=atmega128

EXEC = peta.hex
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS: %.cpp = bin/%.o)

verify:
	$(ARDUINO) --verify $(MAIN)

gui:
	$(ARDUINO) $(MAIN)

upload:
	$(ARDUINO) --upload $(MAIN)

# TODO

$(EXEC): $(OBJS)
	$(CXX) $(LD_FLAGS) $(OBJS) -o $(EXEC) $(OBJS)

%.o: %.cpp
	@mkdir -p bin/
	$(CXX) -c $(CC_FLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)
