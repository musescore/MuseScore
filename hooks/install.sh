#!/usr/bin/env bash

HOOKSDIR=./.git/hooks/

rm -f "$HOOKSDIR/pre-commit"
ln ./hooks/pre-commit "$HOOKSDIR/pre-commit"