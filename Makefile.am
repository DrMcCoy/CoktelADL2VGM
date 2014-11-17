include $(top_srcdir)/Makefile.common

ACLOCAL_AMFLAGS = -I m4 --install

EXTRA_DIST = \
             README.md \
             Doxyfile \
             README.dox \
             $(EMPTY)

dist_doc_DATA = \
                COPYING \
                AUTHORS \
                ChangeLog \
                README.md \
                $(EMPTY)

SUBDIRS = \
          src \
          $(EMPTY)

doxygen:
	doxygen

doxygen-clean:
	rm -rf doc/doxygen/html/
	rm -rf doc/doxygen/latex/
	rm -rf doc/doxygen/man/
