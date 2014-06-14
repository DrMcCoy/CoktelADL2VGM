include $(top_srcdir)/Makefile.common

SUBDIRS = \
          common \
          adlib \
          $(EMPTY)

bin_PROGRAMS = cokteladl2vgm

cokteladl2vgm_SOURCES = \
                        cokteladl2vgm.cpp \
                        $(EMPTY)

cokteladl2vgm_LDADD = \
                      adlib/libadlib.la \
                      common/libcommon.la \
                      $(LDADD) \
                      $(EMPTY)
