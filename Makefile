DIRS=shared core config serial fpsconfig

ifneq ($(OS),Windows_NT)
DIRS+= bluetooth po
endif

all:
	+for i in $(DIRS); do cd $$i; make all; cd ..; done

clean:
	+for i in $(DIRS); do cd $$i; make clean; cd ..; done

ifeq ($(OS),Windows_NT)
install: all
	mkdir -p setup
	cp -u -f /mingw/bin/libgcc_s_dw2-1.dll setup
	cp -u -f /mingw/bin/libiconv-2.dll setup
	cp -u -f /mingw/bin/libstdc++-6.dll setup
	cp -u -f /mingw/bin/libxml2.dll setup
	cp -u -f /mingw/bin/SDL.dll setup
	cp -u -f /mingw/bin/zlib1.dll setup
	cp -u -f /mingw/bin/libpdcursesw.dll setup
	cp -u -f /mingw/bin/libintl-8.dll setup
	cp -u -f /mingw/bin/libusb-1.0.dll setup
	cp -u -f core/gimx.exe setup
	cp -u -f config/gimx-config.exe setup
	cp -u -f serial/gimx-serial.exe setup
	cp -u -f fpsconfig/gimx-fpsconfig.exe setup
	cp -u -f /mingw/local/bin/wget-1.11.4.exe setup/wget.exe
	cp -u -f /etc/wgetrc setup/wget.ini
	mkdir -p setup/share/locale
	for translation in po/*.po; \
  do \
    po=$$(basename $${translation} .po); \
    mkdir -p setup/share/locale/$$po/LC_MESSAGES; \
    msgfmt -o setup/share/locale/$$po/LC_MESSAGES/gimx.mo po/$$po.po; \
  done

else
install: all
	for i in $(DIRS); do cd $$i; make install; cd ..; done

uninstall:
	-for i in $(DIRS); do cd $$i; make uninstall; cd ..; done

really-clean: clean uninstall
endif
