FROM fedora

#########
### Install deps
#########
RUN dnf update -y && \
  dnf install -y gcc cmake boost-devel cpprest-devel git gcc-c++ openssl-devel

#########
### Clone repo
#########
WORKDIR /build
RUN git clone https://github.com/bschiffthaler/gofer2
WORKDIR /build/gofer2/build


#########
### Build Gopher2 source
#########
RUN rm -rf * && \
  cmake .. && \
  make && \
  cp gofer2 /usr/local/bin


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
CMD ["gofer2","/run/conf.json"]
