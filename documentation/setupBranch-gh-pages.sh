#!/bin/bash

############
# Create or recreate a branch gh-pages.
# The name is fix because of interpretation in github / github-pages

# CONFIG PART
# Set the name of the remote alias (normally set to "origin")
REMOTE=origin
REMOTE=my

# First check if the current branch is clean.
# if not, show info and terminate
if [ $(git status --porcelain | wc -l) != 0 ] ; then
    echo "Warning: Current branch has uncommitted content:";
    git status;
    echo "Aborting.";
    exit -1;
fi

# Save current branch info
CURRENT_BRANCH=$(git branch | egrep "^\\*" | cut -d" " -f2)

# Assume we are in a directory controlled by git for ckb-next.
# First delete a remote branch gh-pages, ignore failure
git push ${REMOTE} --delete gh-pages

# Remove local branch, ignore failure
git branch -D gh-pages

# Beginning with this line, no error should occur.
set -e

# Create a new branch, detach it from all other branches
# and use this new branch
git checkout --orphan gh-pages

# Remove all existing content, because only generated docs
# will be allowed in this branch
git rm -rf .

# Now clone and copy the prepared content and cleanup
(cd /tmp; git clone https://github.com/frickler24/gh-pages-ckb-next.git)
cp -R /tmp/gh-pages-ckb-next/* .
rm -rf /tmp/gh-pages-ckb-next/

# Commit and push our resulting branch
git add --all
git commit -am"First Clear commit to gh-pages"
git push ${REMOTE} gh-pages

# check out the previous branch (i.e. master)
git checkout ${CURRENT_BRANCH}
exit 0
