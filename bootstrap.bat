echo [bootstrap.bat] fetch precompiled pkg
git clone --bare --branch master --depth 1 git@git.motis-project.de:dl/pkg.git
git --git-dir pkg.git show HEAD:pkg.exe > pkg.exe

echo [bootstrap.bat] load local deps
pkg.exe -l

echo [bootstrap.bat] done.
