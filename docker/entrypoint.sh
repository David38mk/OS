#!/bin/bash
# Install the host's public key (mounted read-only at /keys/authorized_keys) with the
# strict ownership and permissions sshd demands, then run sshd in the foreground.
set -e
user=david
if [ -f /keys/authorized_keys ]; then
    install -d -m 700 -o "$user" -g "$user" "/home/$user/.ssh"
    install -m 600 -o "$user" -g "$user" /keys/authorized_keys "/home/$user/.ssh/authorized_keys"
fi
exec /usr/sbin/sshd -D -e
