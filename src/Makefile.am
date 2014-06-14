include $(top_srcdir)/Makefile.common

SUBDIRS = \
          common \
          $(EMPTY)

noinst_HEADERS = \
                 adlib.hpp \
                 adlplayer.hpp \
                 $(EMPTY)

bin_PROGRAMS = cokteladl2vgm

cokteladl2vgm_SOURCES = \
                        adlib.cpp \
                        adlplayer.cpp \
                        cokteladl2vgm.cpp \
                        $(EMPTY)

cokteladl2vgm_LDADD = \
                      common/libcommon.la \
                      $(LDADD) \
                      $(EMPTY)
