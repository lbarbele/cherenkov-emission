#!/bin/bash

# A lista de runs do CORSIKA
RUNLIST="100 200 201 202 203 204 205 206 207 208 210 211 212 213 220 230"

# O diretório de entrada (arquivos do CORSIKA)
INPDIR=/veritas/userspace/vitor/userspace4/corsika-76300/output

# Diretório de saída
OUTDIR=/veritas/userspace/vitor/saida-luan

# Redireciona a saída para um log
if [ -z ${kLog} ]
then
  export kLog=1
  ./analisa.sh > >(tee -a ${OUTDIR}/analisa.out) 2> >(tee -a ${OUTDIR}/analisa.err >&2)
  exit 0
fi

# Compila o leitor, se necessário
make

for RUN in ${RUNLIST}
do
  ./readCorsika ${INPDIR} ${OUTDIR} ${RUN}
done
