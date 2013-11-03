#!/bin/bash

pushd $(dirname $(readlink -f $0)) >/dev/null
ln -s "$(readlink -f git-pre-commit-hook.sh)" ../.git/hooks/pre-commit
popd >/dev/null
