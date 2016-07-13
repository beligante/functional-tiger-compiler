#ifndef __DG_H__
#define __DG_H__

/*
 * directed graph's simple implementation, used for
 * type-checking
 */

#define MAX_VERTEX_NUM 20
typedef struct DG_edge_ *DG_edge;
typedef struct DG_ DG
struct DG_edge_ {
  int adjvex;
  DG_edge next;
  void *info; // info of the edge
};
struct DG_vertex {
  void *info; // info of the vertex
  DG_edge firstEdge;
};
struct DG_ {
  DG_vertex vertice[MAX_VERTEX_NUM];
  int verticeCnt;

  // cycles of the DG if any
  int *cycles;

  // topological sort result if no cycles
  int *tpg;
  // topological sort reverse index into tpg
  int tpg_sort_rev_index;

  /* 
   * auxiliary data
   */
  // marked whether a vertex is visited
  int visited[MAX_VERTEX_NUM];

  // marked whether a vertex is on the visiting path
  int on_stack[MAX_VERTEX_NUM];

  // edge_to[v] = previous vertex on path to v
  int edge_to[MAX_VERTEX_NUM];
};


DG_edge DG_Edge(int adjvex, DG_edge next, void *info);
DG_vertex DG_Vertex(void *info, DG_edge firstEdge);
DG DG_makeEmptyDG();
int indexOfVertex(DG dg, void *info);

// enter edge: v1 -> v2
DG DG_enterEdge(DG dg, void *info1, void *info2);

void DG_dfs_from(DG dg, int v);

// only called once for a single DG
void DG_dfs(DG dg);

int DG_cycle(DG dg);

#endif
