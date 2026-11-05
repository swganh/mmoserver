#!/usr/bin/env bash
#
# Starts, stops and checks the MMOServer processes of a build made with
# build_server.sh. Run with -h for usage.

set -euo pipefail

basedir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

usage() {
    cat <<EOF
Usage: $(basename "$0") [-r] <command> [zone ...]

Commands:
  start [zone ...]   Start the Connection, Chat, Login and Ping servers,
                     then the given zones (default: tutorial tatooine)
  debug [-g] [zone ...]
                     Like start, but each server runs in its own window of
                     a tmux session, showing its output live. With -g each
                     runs under gdb, which stops at a crash and prints the
                     backtrace. Type q and Enter in a window to quit that
                     server; Ctrl-b n / Ctrl-b p switch windows, Ctrl-b d
                     detaches (servers keep running; tmux attach -t mmoserver)
  stop               Stop every server started by this script
  status             Show which servers are running
  restart [zone ...] stop, then start

Options:
  -r                 Use the release build (build/linux-release)
  -h, --help         Show this help and exit

Each server runs detached from the terminal and logs to bin/logs/<name>.out;
the previous run's log is kept as <name>.out.1.
Zones other than tutorial need their heightmap: ./build_server.sh --heightmaps <zone>
EOF
}

preset=linux-debug
if [[ ${1:-} == -r ]]; then
    preset=linux-release
    shift
fi

bin_dir="$basedir/build/$preset/bin"
run_dir="$bin_dir/run"
log_dir="$bin_dir/logs"

# Order matters: the other servers register with the ConnectionServer.
core_servers=(connection chat login ping)

executable() {
    case "$1" in
        connection) echo ConnectionServer ;;
        login) echo LoginServer ;;
        ping) echo PingServer ;;
        chat) echo ChatServer ;;
        *) echo ZoneServer ;;
    esac
}

pid_of() {
    local pid_file="$run_dir/$1.pid" pid
    [[ -f $pid_file ]] || return 1
    pid=$(<"$pid_file")
    # Only trust the pid if it still belongs to one of our servers.
    if [[ -d /proc/$pid ]] && grep -q "Server" "/proc/$pid/cmdline" 2>/dev/null; then
        echo "$pid"
        return 0
    fi
    # Leave the file: status keeps reporting the server as not running
    # until stop (or the next start) clears it.
    return 1
}

start_one() {
    local name=$1 exe args=() pid
    exe=$(executable "$name")
    [[ $exe == ZoneServer ]] && args=(--ZoneName "$name")

    if pid=$(pid_of "$name"); then
        echo "$name: already running (pid $pid)"
        return
    fi
    if [[ $exe == ZoneServer && $name != tutorial && ! -f $bin_dir/heightmaps/$name.hmpw ]]; then
        echo "$name: skipped, no heightmap (./build_server.sh --heightmaps $name)" >&2
        return
    fi

    # Keep the previous run's log: it may hold the reason that run ended.
    if [[ -s $log_dir/$name.out ]]; then
        mv "$log_dir/$name.out" "$log_dir/$name.out.1"
    fi

    # setsid detaches from the terminal; with stdin from /dev/null the
    # server's console command check stays out of the way.
    (cd "$bin_dir" && exec setsid "./$exe" "${args[@]}" < /dev/null > "$log_dir/$name.out" 2>&1) &
    echo $! > "$run_dir/$name.pid"
    echo "$name: started (pid $!, log logs/$name.out)"
}

# Checks the build and sets servers=(core servers, then the given zones).
prepare() {
    [[ -x $bin_dir/ConnectionServer ]] || { echo "error: no build in $bin_dir (run ./build_server.sh)" >&2; exit 1; }
    [[ -f $bin_dir/config/general.cfg ]] || { echo "error: $bin_dir/config missing (run ./build_server.sh)" >&2; exit 1; }
    mkdir -p "$run_dir" "$log_dir"

    local zones=("$@") name
    [[ ${#zones[@]} -eq 0 ]] && zones=(tutorial tatooine)
    for name in "${zones[@]}"; do
        [[ -f $bin_dir/config/$name.cfg ]] || { echo "error: unknown zone '$name' (no config/$name.cfg)" >&2; exit 1; }
    done
    servers=("${core_servers[@]}" "${zones[@]}")
}

start() {
    local name
    prepare "$@"
    for name in "${servers[@]}"; do
        start_one "$name"
        sleep 2
    done
    echo
    echo "Zones take a minute or two to load. Check with: $(basename "$0") status"
}

tmux_session=mmoserver

debug() {
    local use_gdb= name exe args pid cmd runner=
    if [[ ${1:-} == -g ]]; then
        use_gdb=1
        shift
    fi
    command -v tmux > /dev/null || { echo "error: tmux is not installed (sudo apt install tmux)" >&2; exit 1; }
    if [[ $use_gdb ]]; then
        command -v gdb > /dev/null || { echo "error: gdb is not installed (sudo apt install gdb)" >&2; exit 1; }
        # Stop only at crashes, then print where it happened. debuginfod
        # (enabled on Ubuntu) would otherwise pause each server at startup to
        # ask about downloading symbols for system libraries.
        runner="gdb -q -iex 'set debuginfod enabled off'"
        runner+=" -ex 'set pagination off' -ex 'set print thread-events off'"
        runner+=" -ex 'handle SIGPIPE nostop noprint pass' -ex 'handle SIGTERM nostop noprint pass'"
        runner+=" -ex run -ex bt --args"
    fi
    if tmux has-session -t "$tmux_session" 2> /dev/null; then
        echo "error: the debug session is already open: tmux attach -t $tmux_session" >&2
        exit 1
    fi
    prepare "$@"

    # Window 0 shows the status; the others keep showing a server's output
    # after it exits, so a crash stays on screen.
    tmux new-session -d -s "$tmux_session" -n status \
        "watch -n 5 $(printf %q "$basedir/server.sh") $( [[ $preset == linux-release ]] && echo -r ) status"

    for name in "${servers[@]}"; do
        if pid=$(pid_of "$name"); then
            echo "$name: already running (pid $pid), not opening a window"
            continue
        fi
        exe=$(executable "$name")
        args=
        [[ $exe == ZoneServer ]] && args="--ZoneName $name"
        if [[ $exe == ZoneServer && $name != tutorial && ! -f $bin_dir/heightmaps/$name.hmpw ]]; then
            echo "$name: skipped, no heightmap (./build_server.sh --heightmaps $name)" >&2
            continue
        fi
        if [[ -s $log_dir/$name.out ]]; then
            mv "$log_dir/$name.out" "$log_dir/$name.out.1"
        fi
        # exec keeps the pid, so the pid file and stop work as with start.
        cmd="cd $(printf %q "$bin_dir") && echo \$\$ > $(printf %q "$run_dir/$name.pid")"
        cmd+=" && exec > >(tee $(printf %q "$log_dir/$name.out")) 2>&1"
        cmd+=" && exec $runner ./$exe $args"
        tmux new-window -d -t "$tmux_session" -n "$name" "bash -c $(printf %q "$cmd")" \; \
            set-option -w -t "$tmux_session:$name" remain-on-exit on
        echo "$name: started in tmux window '$name' (log logs/$name.out)"
        sleep 2
    done

    if [[ -t 0 && -t 1 ]]; then
        if [[ ${TMUX:-} ]]; then
            tmux switch-client -t "$tmux_session"
        else
            exec tmux attach -t "$tmux_session"
        fi
    else
        echo
        echo "Open it with: tmux attach -t $tmux_session"
    fi
}

stop() {
    local pid_file name pid stopped=()
    for pid_file in "$run_dir"/*.pid; do
        [[ -e $pid_file ]] || continue
        name=$(basename "$pid_file" .pid)
        if pid=$(pid_of "$name"); then
            kill "$pid" 2>/dev/null || true
            # Under gdb (debug -g) the server is gdb's child.
            pkill -TERM -P "$pid" 2>/dev/null || true
            stopped+=("$name:$pid")
        else
            echo "$name: was not running; it may have crashed (see logs/$name.out)"
            rm -f "$pid_file"
        fi
    done

    # Give them a few seconds to exit, then force it.
    local entry deadline=$((SECONDS + 10))
    for entry in "${stopped[@]}"; do
        name=${entry%%:*}
        pid=${entry##*:}
        while [[ -d /proc/$pid ]] && (( SECONDS < deadline )); do
            sleep 0.5
        done
        if [[ -d /proc/$pid ]]; then
            kill -KILL "$pid" 2>/dev/null || true
            echo "$name: killed (did not exit in time)"
        else
            echo "$name: stopped"
        fi
        rm -f "$run_dir/$name.pid"
    done
    [[ ${#stopped[@]} -gt 0 ]] || echo "No servers running."

    if tmux has-session -t "$tmux_session" 2> /dev/null; then
        tmux kill-session -t "$tmux_session"
        echo "Closed the tmux debug session."
    fi
}

status() {
    local pid_file name pid found=
    for pid_file in "$run_dir"/*.pid; do
        [[ -e $pid_file ]] || continue
        name=$(basename "$pid_file" .pid)
        if pid=$(pid_of "$name"); then
            found=1
            printf '%-12s running (pid %s)  %s\n' "$name" "$pid" \
                "$(grep -v '^[[:space:]]*$' "$log_dir/$name.out" 2>/dev/null | tail -n 1)"
        else
            printf '%-12s not running (see logs/%s.out)\n' "$name" "$name"
        fi
    done
    [[ $found ]] || echo "No servers running."

    # The rules themselves are only readable by root, so just remind.
    if grep -qs '^ENABLED=yes' /etc/ufw/ufw.conf; then
        echo
        echo "Note: the ufw firewall is enabled. Game clients on other machines need"
        echo "UDP 44990-44992 open: sudo ufw allow 44990:44992/udp (check: sudo ufw status)"
    fi

    local others
    # Ours are the pids in the pid files and, under gdb (debug -g), their
    # children.
    local ours=() pid_file_pid
    for pid_file in "$run_dir"/*.pid; do
        [[ -e $pid_file ]] || continue
        pid_file_pid=$(<"$pid_file")
        ours+=("$pid_file_pid" $(pgrep -P "$pid_file_pid" || true))
    done
    others=$(pgrep -a -f '^([^ ]*/)?(Connection|Login|Ping|Chat|Zone)Server( |$)' \
        | awk -v ours=" ${ours[*]} " 'index(ours, " " $1 " ") == 0' || true)
    if [[ $others ]]; then
        echo
        echo "Also running, not started by this script (stop them to avoid port clashes):"
        echo "$others"
    fi
}

case "${1:-}" in
    start) shift; start "$@" ;;
    debug) shift; debug "$@" ;;
    stop) stop ;;
    status) status ;;
    restart) shift; stop; start "$@" ;;
    -h|--help) usage ;;
    *) usage >&2; exit 2 ;;
esac
