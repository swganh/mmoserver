ACLOCAL_AMFLAGS = -I tools/m4 --install

SUBDIRS = $(subdirs) docs src

# Keep documentation files when creating a distributable version of the source.
doc_DATA = AUTHORS ChangeLog COPYING INSTALL NEWS README

# When creating a distributable version of the source we want the .svn directories 
# removed. This hook accomplishes that.
dist-hook:
	-rm -rf 'find $(distdir) -name .svn'
