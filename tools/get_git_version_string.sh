#!/bin/sh

VERSION="$(git describe --tags --dirty --match release-*)"

if [ $? -eq 0 ] ;
then
  echo "$VERSION" | sed 's,^release-,,;s,-,+,;s,-,~,;'
  exit 0
fi

# with shallow clones, there may be no tags, so the first ^
# try to get version string will fail

# in that case let's at least return the commit hash

# Lightroom Classic edition: the fork has no release-* tags (and CI clones
# are shallow), so put the darktable version it is based on in front of the
# hash: features keyed on the version (AI model downloads) need it
BASE_VERSION="5.6.2"

VERSION="$(git describe --always --dirty)"
if [ $? -eq 0 ] ;
then
  echo "${BASE_VERSION}+0~g${VERSION}"
  exit 0
fi

# failed for some reason. let's just propagate
echo "unknown-version"
exit 0 # to not fail the whole build.
