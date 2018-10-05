FROM fedora

#########
### Install deps
#########
RUN dnf update -y && \
  dnf install -y gcc cmake boost-devel cpprest-devel git gcc-c++ openssl-devel \
                 tclap

#########
### Clone repo
#########
WORKDIR /build
RUN git clone https://autodocker:autodocker@microasp.upsc.se/bastian/gopher2.git
WORKDIR /build/gopher2/build


#########
### Build Gopher2 source
#########
RUN rm -rf * && \
  cmake .. && \
  make && \
  cp gopher2 /usr/local/bin


#########
### Cleanup
#########
WORKDIR /
RUN rm -rf /build

WORKDIR /run
VOLUME /run

#########
### PORT and CMD
#########
EXPOSE 5432
CMD ["gopher2","/run/conf.json"]