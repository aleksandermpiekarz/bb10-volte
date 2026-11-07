#!/usr/bin/env bash
# Sourced by bb-ssh, bb-send and bb-fetch. No credentials are stored here.
set -euo pipefail
bb_host=${BB_HOST:-169.254.0.1}
bb_key=${BB_KEY_WIN:-D:\\blackberry\\keys\\id_rsa4x}
bb_openssh=${BB_OPENSSH_DIR:-/mnt/c/Windows/System32/OpenSSH}
bb_options=(-i "$bb_key" -o IdentitiesOnly=yes -o BatchMode=yes
    -o ConnectTimeout=10 -o StrictHostKeyChecking=yes
    -o HostKeyAlgorithms=+ssh-rsa -o PubkeyAcceptedAlgorithms=+ssh-rsa
    -o MACs=+hmac-sha1)
[[ $bb_host =~ ^[a-zA-Z0-9.-]+$ ]] || { echo 'Invalid BB_HOST' >&2; exit 2; }

bb_remote_path() {
    # Legacy SCP invokes the remote shell: accept literal absolute paths only.
    [[ $1 =~ ^/[a-zA-Z0-9_./-]+$ ]] || {
        echo 'Remote path must be absolute, without spaces or shell characters' >&2
        exit 2
    }
}
