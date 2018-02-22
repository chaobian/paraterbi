#!/bin/sh
exec cat| sed "s/^[In\ file\ included\ from\ ]*\S*\///g" | sed "s/note:/\nnote:/g" | sed "s/error:/\nerror:/g" #| sed 's/\/home\/marius\\/prog\/paraterbi\/src\///g'
