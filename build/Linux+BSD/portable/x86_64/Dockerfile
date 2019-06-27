FROM library/ubuntu:14.04
ADD Recipe /Recipe
RUN bash -ex Recipe && apt-get clean autoclean
RUN apt-get autoremove -y && rm -rf /var/lib/{apt,dpkg,cache,log}/ /tmp/*
