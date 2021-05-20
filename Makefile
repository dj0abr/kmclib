# makefile for kmclib on Linux (PC desktop and ARM)
# usage:
# make clean ... delete all build files
# make ... create the library kmclib

CXXFLAGS = -Wall -O3 -std=c++0x -Wno-write-strings -Wno-narrowing -I /usr/include/freetype2
OBJ = kmlib/kmtimer.o kmlib/km_helper.o kmlib/kmfifo.o\
ttyUSB/identifySerUSB.o ttyUSB/serial_helper.o udp/udp.o\
websocket/ws.o websocket/ws_callbacks.o websocket/websocketserver.o websocket/sha1.o websocket/base64.o websocket/handshake.o\
i2c_rpi/i2c_rpi.o i2c_rpi/max11615.o i2c_rpi/mcp23017.o\
serial/serial.o\
stepper/stepper.o\
motor/motor.o\
gps/gps.o gps/sunposition.o\
rotary/encoder.o\
display/display.o display/display_draw.o display/tjpgd.o display/bargraph.o display/drawfont.o\
touch/xpt2046_rpi.o touch/touch_button.o\
autotuner/autotuner.o autotuner/bar_graphs.o autotuner/touch_buttons.o autotuner/menu.o autotuner/showvals.o

default: $(OBJ)
	g++ $(CXXFLAGS) -c $(OBJ)
	ar -rcs kmclib.a $(OBJ)

clean:
	rm -rf *.o *.a
	rm -rf udp/*.o
	rm -rf ttyUSB/*.o
	rm -rf websocket/*.o
	rm -rf kmlib/*.o
	rm -rf i2c_rpi/*.o
	rm -rf serial/*.o
	rm -rf test/*.o
	rm -rf stepper/*.o
	rm -rf motor/*.o
	rm -rf gps/*.o
	rm -rf rotary/*.o
	rm -rf display/*.o
	rm -rf autotuner/*.o
	rm -rf touch/*.o
