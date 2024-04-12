FROM library/ubuntu:20.04
ENV TZ=Etc/UTC DEBIAN_FRONTEND=noninteractive
COPY setup.sh /setup.sh
COPY install_mu.sh /install_mu.sh
RUN bash -ex setup.sh
