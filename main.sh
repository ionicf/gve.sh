#!/usr/bin/env bash
src="gve.sh"
out="$HOME/Logs/$src$1.log"
ulimit -s unlimited
mkdir -p "$HOME/Logs"
printf "" > "$out"

# Download program
if [[ "$DOWNLOAD" != "0" ]]; then
  rm -rf $src
  git clone https://github.com/ionicf/$src
  cd $src
fi

# Fixed config
: "${KEY_TYPE:=uint32_t}"
: "${EDGE_VALUE_TYPE:=float}"
: "${MAX_THREADS:=64}"
: "${VERSION:=1.0.0}"
# Define macros (dont forget to add here)
DEFINES=(""
"-DKEY_TYPE=$KEY_TYPE"
"-DEDGE_VALUE_TYPE=$EDGE_VALUE_TYPE"
"-DMAX_THREADS=$MAX_THREADS"
"-DVERSION=\"$VERSION\""
)

# Build tool
g++ ${DEFINES[*]} -std=c++17 -O3 -fopenmp main.cxx
if [[ "$?" != "0" ]]; then exit 1; fi

# Run tool on a single graph
runTool() {
  # $1: input file name (without extension)
  # $2: is graph weighted (0/1)
  # $3: is graph symmetric (0/1)
  opt2=""
  opt3=""
  opt4=""
  opt5=""  # set to "-t" for outputting graph in symmetric format
  if [[ "$2" == "1" ]]; then opt2="-w"; fi
  if [[ "$3" == "1" ]]; then opt3="-s"; fi
  if [[ "$2" == "1" ]]; then opt4="-x"; fi
  stdbuf --output=L ./a.out make-undirected -i "$1" -o "$1.undirected" "$opt2" "$opt3" "$opt4" "$opt5" 2>&1 | tee -a "$out"
}

# Run tool on all graphs
if [[ "$RUN" != "0" ]]; then
  # runTool ~/Data/web-Stanford.mtx    0 0
  runTool ~/Data/indochina-2004.mtx  0 0
  runTool ~/Data/uk-2002.mtx         0 0
  runTool ~/Data/arabic-2005.mtx     0 0
  runTool ~/Data/uk-2005.mtx         0 0
  runTool ~/Data/webbase-2001.mtx    0 0
  runTool ~/Data/it-2004.mtx         0 0
  runTool ~/Data/sk-2005.mtx         0 0
  runTool ~/Data/com-LiveJournal.mtx 0 1
  runTool ~/Data/com-Orkut.mtx       0 1
  runTool ~/Data/asia_osm.mtx        0 1
  runTool ~/Data/europe_osm.mtx      0 1
  runTool ~/Data/kmer_A2a.mtx        0 1
  runTool ~/Data/kmer_V1r.mtx        0 1
fi
