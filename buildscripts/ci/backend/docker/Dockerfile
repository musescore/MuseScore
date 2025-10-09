FROM library/ubuntu:22.04
ENV TZ=Etc/UTC DEBIAN_FRONTEND=noninteractive
COPY setup.sh /setup.sh
COPY install_mu.sh /install_mu.sh
COPY install_muse_sounds.sh /install_muse_sounds.sh
COPY muse_sounds_install_all.sh /muse_sounds_install_all.sh
COPY muse_sounds_clear_all.sh /muse_sounds_clear_all.sh
RUN bash -ex setup.sh
