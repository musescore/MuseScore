FROM ericfont/armv7hf-debian-qemu:jessie

RUN [ "cross-build-start" ]

RUN apt-get update

# need to be able to use https for wget
RUN apt-get install ca-certificates wget #git

# get prebuilt AppImageKit
RUN wget https://bintray.com/artifact/download/ericfont/prebuilt-AppImageKit/AppImageKit-5_built-in-armv7hf-jessie.tar.gz
RUN tar -xvzf AppImageKit-5_built-in-armv7hf-jessie.tar.gz
RUN rm AppImageKit-5_built-in-armv7hf-jessie.tar.gz

# add AppImageKit dependencies
RUN apt-get -y install libfuse-dev libglib2.0-dev cmake git libc6-dev binutils fuse python

RUN [ "cross-build-end" ]  
