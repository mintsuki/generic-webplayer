.PHONY: all clean install uninstall distclean dist-clean

all:
	$(MAKE) -C work all

clean:
	$(MAKE) -C work clean

install:
	$(MAKE) -C work install

uninstall:
	$(MAKE) -C work uninstall

distclean: dist-clean

dist-clean:
	rm -rf work

