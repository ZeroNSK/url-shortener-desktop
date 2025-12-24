#!/bin/bash

if [ ! -f "./program" ] || [ ! -f "./url_server" ] || [ ! -f "./url_shortener_gui" ]; then
    make all-gui > /dev/null 2>&1
fi
./program --server --port 6379 > /dev/null 2>&1 &
DB_PID=$!

sleep 1

./url_server > /dev/null 2>&1 &
SERVER_PID=$!

sleep 1

./url_shortener_gui

kill $SERVER_PID 2>/dev/null
kill $DB_PID 2>/dev/null