# Prepare source

Check working tree is clean:

    git status

Update NEWS:

    git log --pretty=format:'- \%s (\%an)' $oldv..HEAD

Update version number in top-level CMakeLists.txt.

Check it still builds.

Publish changes:

    git commit
    git push

# Pack source

Get releaseme:

    git clone kde:releaseme
    cd releaseme

Create tarball.
- For stable updates:

    ./homerun.rb -u $svnusername --git-branch homerun/$x.$y --version $x.$y.$z

- For .0 releases:

    ./homerun.rb -u $svnusername --version $x.$y.$z

Check it builds:

    tar xf homerun-$newv.tar.bz2
    cd homerun-$newv
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=../install ..
    make all
    make check
    make install

Check translations have been installed.

If it is a new .0 release, create a branch (if $newv is $x.$y.0, $branch is homerun/$x.$y)

    git checkout -b $branch
    git push -u origin $branch:$branch

If ok, create $newv tag:

    git tag -a $newv

Push:

    git push
    git push --tags

# Publish

Upload to download.kde.org, following instructions from:
<http://download.kde.org/README_UPLOAD>

Add new version number on:
<https://bugs.kde.org/editversions.cgi?product=homerun>

Update PPA

Upload on http://kde-apps.org

Blog
