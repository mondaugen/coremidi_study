# This successfully builds a tags file like I want, but ctags doesn't seem to
# look for function declarations, which are all we have. FTS
FRAMEWORKPATH=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/System/Library/Frameworks/;
pcregrep -I -h -r -o1 '(__OSX[^\(]*)\(' $FRAMEWORKPATH | sort \
    | gawk '{ print $1 "+"}' | uniq > ./tags_ignore;
/usr/local/bin/ctags --excmd=n -I./tags_ignore -R $FRAMEWORKPATH;
#/usr/local/bin/ctags --excmd=n -I__OSX_AVAILABLE_STARTING+ -R $FRAMEWORKPATH;

