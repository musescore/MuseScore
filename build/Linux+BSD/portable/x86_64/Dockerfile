FROM library/centos:6
ADD https://raw.githubusercontent.com/shoogle/MuseScore/master/build/Linux%2BBSD/portable/Recipe /Recipe
RUN bash -ex Recipe && yum clean all
