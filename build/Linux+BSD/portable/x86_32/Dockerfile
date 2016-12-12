FROM ivochkin/ubuntu-i386:precise
ADD Recipe /Recipe
RUN linux32 --32bit i386 bash -ex Recipe && apt-get clean autoclean
RUN apt-get autoremove -y && rm -rf /var/lib/{apt,dpkg,cache,log}/ /tmp/*
