#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <omp.h>
#include "inc/main.hxx"
#include "commands.hxx"

using namespace std;




#pragma region CONFIGURATION
#ifndef KEY_TYPE
/** Type of vertex ids. */
#define KEY_TYPE uint32_t
#endif
#ifndef EDGE_VALUE_TYPE
/** Type of edge weights. */
#define EDGE_VALUE_TYPE float
#endif
#ifndef MAX_THREADS
/** Maximum number of threads to use. */
#define MAX_THREADS 1
#endif
#pragma endregion




#pragma region IO
/**
 * Read the specified input graph.
 * @param a read graph (output)
 * @param file input file name
 * @param format input file format
 * @param symmetric is graph symmetric?
 */
template <bool WEIGHTED=false, class G>
inline void readGraphW(G& a, const string& file, const string& format, bool symmetric=false) {
  ifstream stream(file.c_str());
  if (format=="mtx") readGraphMtxFormatOmpW<WEIGHTED>(a, stream);
  else if (format=="coo") readGraphCooFormatOmpW<WEIGHTED>(a, stream, symmetric);
  else if (format=="edgelist" || format=="csv" || format=="tsv") readGraphEdgelistFormatOmpW<WEIGHTED>(a, stream, symmetric);
  else throw std::runtime_error("Unknown input format: " + format);
  stream.close();
}


/**
 * Write the specified output graph.
 * @param x graph to write (input)
 * @param file output file name
 * @param format output file format
 * @param symmetric is graph symmetric?
 */
template <bool WEIGHTED=false, class G>
inline void writeGraph(const G& x, const string& file, const string& format, bool symmetric=false) {
  ofstream stream(file.c_str());
  if (format=="mtx") writeGraphMtxFormatOmp<WEIGHTED>(stream, x, symmetric);
  else if (format=="coo") writeGraphCooFormatOmp<WEIGHTED>(stream, x, symmetric);
  else if (format=="edgelist") writeGraphEdgelistFormatOmp<WEIGHTED>(stream, x, symmetric);
  else if (format=="csv") writeGraphEdgelistFormatOmp<WEIGHTED>(stream, x, symmetric, ',');
  else if (format=="tsv") writeGraphEdgelistFormatOmp<WEIGHTED>(stream, x, symmetric, '\t');
  else throw std::runtime_error("Unknown output format: " + format);
  stream.close();
}
#pragma endregion




#pragma region MAKE UNDIRECTED
/**
 * Run the count-disconnected-communities command.
 * @param o command options
 */
inline void runCountDisconnectedCommunities(const OptionsCountDisconnectedCommunities& o) {
  using K = KEY_TYPE;
  using E = EDGE_VALUE_TYPE;
  DiGraph<K, None, E> x;
  printf("Reading graph %s ...\n", o.inputFile.c_str());
  if (o.weighted) readGraphW<true> (x, o.inputFile, o.inputFormat, o.symmetric);
  else            readGraphW<false>(x, o.inputFile, o.inputFormat, o.symmetric);
  println(x);
  // Symmetrize graph.
  if (!o.symmetric) {
    symmetrizeOmpU(x);
    print(x); printf(" (symmetrize)\n");
  }
  // Read community membership.
  vector<K> membership(x.span());
  ifstream membershipStream(o.membershipFile.c_str());
  printf("Reading community membership %s ...\n", o.membershipFile.c_str());
  if (o.membershipKeyed) readVectorW<true> (membership, membershipStream, o.membershipStart);
  else                   readVectorW<false>(membership, membershipStream, o.membershipStart);
  // Count the number of disconnected communities.
  auto fc = [&](auto u) { return membership[u]; };
  size_t ncom = communities(x, membership).size();
  size_t ndis = countValue(communitiesDisconnectedOmp(x, membership), char(1));
  printf("Number of communities: %zu\n", ncom);
  printf("Number of disconnected communities: %zu\n", ndis);
  printf("\n");
}


/**
 * Run the make-undirected command.
 * @param o command options
 */
inline void runMakeUndirected(const OptionsMakeUndirected& o) {
  using K = KEY_TYPE;
  using E = EDGE_VALUE_TYPE;
  DiGraph<K, None, E> x;
  // Read graph.
  printf("Reading graph %s ...\n", o.inputFile.c_str());
  if (o.inputWeighted) readGraphW<true> (x, o.inputFile, o.inputFormat, o.inputSymmetric);
  else                 readGraphW<false>(x, o.inputFile, o.inputFormat, o.inputSymmetric);
  println(x);
  // Symmetrize graph.
  if (!o.inputSymmetric) {
    symmetrizeOmpU(x);
    print(x); printf(" (symmetrize)\n");
  }
  // Write undirected graph.
  printf("Writing undirected graph %s ...\n", o.outputFile.c_str());
  if (o.outputWeighted) writeGraph<true> (x, o.outputFile, o.outputFormat, o.outputSymmetric);
  else                  writeGraph<false>(x, o.outputFile, o.outputFormat, o.outputSymmetric);
  printf("Undirected graph written to %s.\n", o.outputFile.c_str());
  printf("\n");
}
#pragma endregion




#pragma region MAIN
/**
 * Main function.
 * @param argc argument count
 * @param argv argument values
 * @returns zero on success, non-zero on failure
 */
int main(int argc, char **argv) {
  using K = KEY_TYPE;
  using E = EDGE_VALUE_TYPE;
  // Initialize OpenMP.
  if (MAX_THREADS) omp_set_num_threads(MAX_THREADS);
  // Run the appropriate command.
  string cmd = argc>1 ? argv[1] : "";
  if (cmd=="count-disconnected-communities") {
    OptionsCountDisconnectedCommunities o = parseCountDisconnectedCommunities(argc, argv, 2);
    runCountDisconnectedCommunities(o);
  } else if (cmd=="make-undirected") {
    OptionsMakeUndirected o = parseOptionsMakeUndirected(argc, argv, 2);
    runMakeUndirected(o);
  } else {
    fprintf(stderr, "Unknown command: %s\n", cmd.c_str());
    return 1;
  }
  return 0;
}
#pragma endregion
