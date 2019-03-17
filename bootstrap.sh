#!/bin/bash

GIT_URL="ssh://git@git.motis-project.de/dl/pkg.git"

echo "[bootstrap.sh] fetch precompiled 'pkg' from ${GIT_URL}"
git archive --format=tar --remote=$GIT_URL HEAD pkg | tar xf - > pkg
chmod +x pkg

echo "[bootstrap.sh] load local deps"
./pkg -l

echo "[bootstrap.sh] done."
