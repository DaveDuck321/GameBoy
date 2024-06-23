port=$((1024 + ($$ % 1000)))

# Launch the emulator
(trap '' INT; exec ./gb.out --listen $port)&
gb_pid=$!
echo "emulator pid $gb_pid"
trap "kill $gb_pid" EXIT

# Launch the debugger
(echo "gdb-remote $port" & cat) | lldb $1 -O 'settings set stop-disassembly-display always'
