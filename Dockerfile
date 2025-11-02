FROM alpine:latest AS base
RUN apk update \
   && apk upgrade \
   && apk add --no-cache \
     cmake \
     make \
     g++ \
     gcc \
    git

WORKDIR /LR2ArenaEx
COPY LR2ArenaEx/ ./

WORKDIR /LR2ArenaEx/standalone
RUN mkdir build
WORKDIR /LR2ArenaEx/standalone/build
RUN cmake .. -DCMAKE_CXX_FLAGS="-static-libgcc -static-libstdc++" -DCMAKE_EXE_LINKER_FLAGS="-static"
RUN make


FROM alpine:latest AS app

LABEL org.opencontainers.image.source=https://git.pfy.ch/pfych/LR2ArenaEX
LABEL org.opencontainers.image.description="LR2Arena standalone server"
LABEL org.opencontainers.image.licenses=MIT

WORKDIR /LR2ArenaEx
COPY --from=base /LR2ArenaEx/standalone/build/lr2arenaex-server lr2arenaex-server

RUN chmod +x lr2arenaex-server
CMD ["./lr2arenaex-server"]
