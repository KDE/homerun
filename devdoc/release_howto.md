# Prepare source

Check working tree is clean:

    git status

Update NEWS:

    git log --pretty=format:'- \%s (\%an)' $oldv..HEAD

Check version number in top-level CMakeLists.txt

Check it still builds

Publish changes:

    git commit
    git push

Note commit-id.

# Pack source

Prepare createtarball:

    cd kdesdk/scripts/createtarball

    # Update version and commit-id ("gitTag" key) in config.ini
    vi config.ini

Create tarball:

    ./create_tarball.rb -a homerun -u $svnusername

Check it builds:

    tar xf homerun-$newv.tar.bz2
    cd homerun-$newv
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=../install ..
    make check
    make install

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

Add new version number on https://bugs.kde.org

Update PPA

Upload on http://kde-apps.org

Blog
