include $(top_srcdir)/Makefile.common

SUBDIRS = \
          common \
          adlib \
          gob \
          $(EMPTY)

noinst_HEADERS = \
                 convert.hpp \
                 $(EMPTY)

bin_PROGRAMS = cokteladl2vgm

cokteladl2vgm_SOURCES = \
                        convert.cpp \
                        cokteladl2vgm.cpp \
                        $(EMPTY)

cokteladl2vgm_LDADD = \
                      gob/libgob.la \
                      adlib/libadlib.la \
                      common/libcommon.la \
                      $(LDADD) \
                      $(EMPTY)
