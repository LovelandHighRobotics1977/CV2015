#! /bin/sh

### BEGIN INIT INFO
# Provides:		cvs1977
# Required-Start:	
# Required-Stop:	
# Default-Start:	2 3 4 5
# Default-Stop:		
# Short-Description:	Computer Vision System Team 1977
### END INIT INFO

set -e

# /etc/init.d/ssh: start and stop the cvs1977 daemon

test -x /usr/bin/cvs1977 || exit 0

umask 022

if test -f /etc/default/ssh; then
    . /etc/default/ssh
fi

. /lib/lsb/init-functions

export PATH="${PATH:+$PATH:}/usr/bin"

case "$1" in
  start)
	log_daemon_msg "Starting cvs1977 server" "cvs1977" || true
	if start-stop-daemon --start --oknodo --pidfile /var/run/cvs1977.pid --exec /usr/bin/cvs1977; then
	    log_end_msg 0 || true
	else
	    log_end_msg 1 || true
	fi
	;;
  stop)
	log_daemon_msg "Stopping cvs1977 server" "cvs1977" || true
	if start-stop-daemon --stop --oknodo --pidfile /var/run/cvs1977.pid; then
	    log_end_msg 0 || true
	else
	    log_end_msg 1 || true
	fi
	;;

  reload|force-reload)
	;;

  restart)
	log_daemon_msg "Restarting cvs1977 server" "cvs1977" || true
	start-stop-daemon --stop --oknodo --retry 30 --pidfile /var/run/cvs1977.pid
	if start-stop-daemon --start --oknodo --pidfile /var/run/cvs1977.pid --exec /usr/bin/cvs1977; then
	    log_end_msg 0 || true
	else
	    log_end_msg 1 || true
	fi

	;;

  status)
	status_of_proc -p /var/run/cvs1977.pid /usr/bin/cvs1977 cvs1977 && exit 0 || exit $?
	;;

  *)
	log_action_msg "Usage: /etc/init.d/cvs1977 {start|stop|reload|force-reload|restart|status}" || true
	exit 1
esac

exit 0
