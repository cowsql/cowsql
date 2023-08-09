# FROM debian:buster-slim as cowsql-lib-builder 
FROM ubuntu as cowsql-lib-builder 
ARG DEBIAN_FRONTEND="noninteractive"
ENV TZ=Europe/London
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV GOROOT=/usr/local/go
ENV GOPATH=/go
ENV PATH=$GOPATH/bin:$GOROOT/bin:$PATH

RUN apt-get update && apt-get install -y git build-essential dh-autoreconf pkg-config libuv1-dev libsqlite3-dev liblz4-dev tcl8.6 wget

WORKDIR /opt

RUN git clone https://github.com/cowsql/raft.git && \
    git clone https://github.com/cowsql/go-cowsql.git && \
    wget -c https://golang.org/dl/go1.15.2.linux-amd64.tar.gz -O - | tar -xzf - -C /usr/local

WORKDIR /opt/raft

RUN autoreconf -i && ./configure && make && make install

WORKDIR /opt/cowsql

COPY . .

RUN autoreconf -i && ./configure && make && make install

WORKDIR /opt/go-cowsql

RUN go get -d -v ./... && \
    go install -tags libsqlite3 ./cmd/cowsql-demo && \
    go install -tags libsqlite3 ./cmd/cowsql

# FROM debian:buster-slim 
FROM ubuntu
ARG DEBIAN_FRONTEND="noninteractive"
ENV TZ=Europe/London
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV PATH=/opt:$PATH

COPY --from=cowsql-lib-builder /go/bin /opt/
COPY --from=cowsql-lib-builder /usr/local/lib /usr/local/lib
COPY --from=cowsql-lib-builder \
    /usr/lib/x86_64-linux-gnu/libuv.so \
    /usr/lib/x86_64-linux-gnu/libuv.so.1\
    /usr/lib/x86_64-linux-gnu/libuv.so.1.0.0\
    /usr/lib/

COPY --from=cowsql-lib-builder \
    /lib/x86_64-linux-gnu/libsqlite3.so \
    /lib/x86_64-linux-gnu/libsqlite3.so.0 \
    /usr/lib/x86_64-linux-gnu/  
