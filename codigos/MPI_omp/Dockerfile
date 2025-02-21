FROM mpioperator/openmpi-builder:v0.6.0 AS builder

COPY contaPalavras.c /src/contaPalavras.c
RUN mpicc \
    -fopenmp \
    -DPALAVRAS_PATH='"/data/palavras.txt"' \
    -o /contaPalavras \
    /src/contaPalavras.c

FROM mpioperator/openmpi:v0.6.0

RUN apt-get update && apt-get install -y libgomp1
COPY --from=builder /contaPalavras /home/mpiuser/contaPalavras
