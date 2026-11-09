# Linux test environment for the OS course: gcc, gdb, python3, sshd.
# Built and started with:  docker compose up -d --build   (from the repo root)
FROM ubuntu:24.04

ARG USER=david
ENV DEBIAN_FRONTEND=noninteractive LANG=C.UTF-8 LC_ALL=C.UTF-8 TZ=Europe/Skopje

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential gdb make manpages manpages-dev man-db \
        python3 python3-pip \
        openssh-server sudo \
        bash-completion vim nano less tree file bc dos2unix \
        psmisc procps lsof strace shellcheck iputils-ping \
        linux-headers-generic libelf-dev kmod \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# ubuntu:24.04 ships a default "ubuntu" user on uid 1000; replace it with ours
RUN userdel -r ubuntu 2>/dev/null || true \
    && useradd -m -s /bin/bash -u 1000 "$USER" \
    && echo "$USER:os" | chpasswd \
    && usermod -aG sudo "$USER" \
    && echo "$USER ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/"$USER" \
    && echo 'cd ~/OS 2>/dev/null' >> /home/"$USER"/.bashrc

RUN mkdir -p /var/run/sshd \
    && sed -i 's/^#\?PasswordAuthentication.*/PasswordAuthentication yes/' /etc/ssh/sshd_config \
    && sed -i 's/^#\?PermitRootLogin.*/PermitRootLogin no/' /etc/ssh/sshd_config

COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

EXPOSE 22
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
