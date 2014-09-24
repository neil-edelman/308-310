# Makefile 1.1 (GNU Make 3.81; MacOSX gcc 4.2.1; MacOSX MinGW 4.3.0)

PROJ  := Simple
VA    := 1
VB    := 0
FILES := Simple Command
SDIR  := src
BDIR  := bin
BACK  := backup
#ICON  := icon.ico
EXTRA := # $(SDIR)/icon.rc
INST  := $(PROJ)-$(VA)_$(VB)
OBJS  := $(patsubst %,$(BDIR)/%.o,$(FILES)) # or $(patsubst %,$(BDIR)/%.class,$(FILES))
SRCS  := $(patsubst %,$(SDIR)/%.c,$(FILES)) # or $(patsubst %,$(SDIR)/%.java,$(FILES))
H     := $(patsubst %,$(SDIR)/%.h,$(FILES))

CC   := gcc # /usr/local/i386-mingw32-4.3.0/bin/i386-mingw32-gcc javac nxjc
CF   := -Wall -O3 -fasm -fomit-frame-pointer -ffast-math -funroll-loops -pedantic -ansi # or -std=c99 -mwindows or -g:none -O -verbose -d $(BDIR) $(SDIR)/*.java -Xlint:unchecked -Xlint:deprecation
OF   := # -framework OpenGL -framework GLUT

default: $(BDIR)/$(PROJ)
#	cp $(SDIR)/$(ICON) $(BDIR)/$(ICON)
#	sips --addIcon $(BDIR)/$(ICON)
#	DeRez -only icns $(BDIR)/$(ICON) > $(BDIR)/$(ICON).rsrc
#	Rez -append $(BDIR)/$(ICON).rsrc -o $(BDIR)/$(PROJ)
#	SetFile -a C $(BDIR)/$(PROJ)
#	rm $(BDIR)/$(ICON) $(BDIR)/$(ICON).rsrc

$(BDIR)/$(PROJ): $(OBJS)
	$(CC) $(CF) $(OF) $^ -o $@

$(BDIR)/%.o: $(SDIR)/%.c
	@mkdir -p $(BDIR)
	$(CC) $(CF) -c $? -o $@

.PHONY: setup clean backup

setup: default
	@mkdir -p $(BDIR)/$(INST)
	cp $(BDIR)/$(PROJ) readme.txt $(BDIR)/$(INST)
	rm -f $(BDIR)/$(INST)-MacOSX.dmg
	# or rm -f $(BDIR)/$(INST)-Win32.zip
	hdiutil create $(BDIR)/$(INST)-MacOSX.dmg -volname "$(PROJ) $(VA).$(VB)" -srcfolder $(BDIR)/$(INST)
	# or zip $(BDIR)/$(INST)-Win32.zip -r $(BDIR)/$(INST)
	rm -R $(BDIR)/$(INST)

clean:
	-rm -f $(OBJS)

backup:
	@mkdir -p $(BACK)
	zip $(BACK)/$(INST)-`date +%Y-%m-%dT%H%M%S`.zip readme.txt Makefile $(SRCS) $(H) $(EXTRA)
