SOURCES			= demo.c playbookx.c util.c
PLAYBOOK_IP		= 169.254.0.1
PLAYBOOK_PASS	= password
BAR_NAME		= demo.bar
EXECUTABLE		= demo
BAR_ASSETS		= bar-descriptor.xml icon.png $(EXECUTABLE)
SDKROOT			= /home/tom/bin/bbndk-2.0.0
DEBUG_TOKEN		= /home/tom/.rim/usbdebugtoken.bar

OBJECTS			= $(SOURCES:.c=.o)

ifeq ($(PLATFORM),playbook)
	# add source files for playbook only
	SOURCES += bbutil.c

	CC          = $(SDKROOT)/host/linux/x86/usr/bin/qcc
	CCFLAGS     = -V4.4.2,gcc_ntoarmv7le -w1 -D_FORTIFY_SOURCE=2 -DUSING_GL11 -g -fstack-protector-all -Dplat_playbook -I$(SDKROOT)/target/qnx6/usr/include/freetype2
	LD          = $(SDKROOT)/host/linux/x86/usr/bin/qcc
	LDFLAGS     = -lbps -lscreen -lEGL -lGLESv1_CM -lfreetype -lpng -V4.4.2,gcc_ntoarmv7le -w1 -g -Wl,-z,relro -Wl,-z,now
else
	CC = gcc
	CCFLAGS = -Wall -Dplat_linux -g
	LD = gcc
	LDFLAGS = -lglut -lGLU -lpng
endif

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXECUTABLE) $(BAR_NAME)

$(BAR_NAME): $(BAR_ASSETS)
	$(SDKROOT)/host/linux/x86/usr/bin/blackberry-nativepackager -devMode -debugToken $(DEBUG_TOKEN) -package $(BAR_NAME) $(BAR_ASSETS)

deploy: $(BAR_NAME)
	$(SDKROOT)/host/linux/x86/usr/bin/blackberry-deploy -installapp $(PLAYBOOK_IP) -password $(PLAYBOOK_PASS) $(BAR_NAME)

