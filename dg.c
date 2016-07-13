#include "dg.h"
#include "util.h"
#include <string.h>

DG_edge DG_Edge(int adjvex, DG_edge next, void *info)
{
  DG_edge edge = checked_malloc(sizeof(*edge));

  edge->adjvex = adjvex;
  edge->next   = next;
  edge->info   = info;

  return edge;
}

DG_vertex DG_Vertex(void *info, DG_edge firstEdge)
{
  DG_vertex vertex = checked_malloc(sizeof(*vertex));

  vertex->info      = info;
  vertex->firstEdge = firstEdge;

  return vertex;
}

DG DG_makeEmptyDG()
{
  DG dg = checked_malloc(sizeof(*dg));

  dg->verticeCnt = 0;
  dg->cycles = NULL;
  dg->tpg = NULL;
  dg->tpg_sort_rev_index = 0;

  memset(dg->edge_to, -1, sizeof(dg->edge_to));
  memset(dg->visited, 0, sizeof(dg->visited));
  memset(dg->on_stack, 0, sizeof(dg->on_stack));

  return dg;
}

int indexOfVertex(DG dg, void *info)
{
  int i;

  for (i = 0; i < dg->verticeCnt; i++) {
    if (vertice[i]->info == info) {
      return i;
    }
  }

  return -1;
}

DG DG_enterEdge(DG dg, void *info1, void *info2)
{
  int v1_index = indexOfVertex(info1);
  int v2_index = indexOfVertex(info2);

  if (v1_index == -1) {
    v1_index = dg->verticeCnt;
    dg->vertice[dg->verticeCnt++] = DG_Vertex(info1, NULL);
  }

  if (v2_index == -1) {
    v2_index = dg->verticeCnt;
    dg->vertice[dg->verticeCnt++] = DG_Vertex(info2, NULL);
  }

  DG_edge edge = dg->vertice[v1_index]->firstEdge;
  DG_edge new_edge = DG_Edge(v2_index, NULL, NULL);
  if (!edge) {
    dg->vertice[v1_index]->firstEdge = new_edge;
  } else {
    while (edge->next) {
      edge = edge->next;
    }
    edge->next = new_edge;
  }

  return dg;
}

void DG_dfs_from(DG dg, int v)
{
  DG_edge edge;
  int w;
  int i;
  int j;

  dg->visited[v] = 1;
  dg->on_stack[v] = 1;

  edge = dg->vertice[v]->firstEdge;
  while(edge) {
    // stop on first cycle
    if (dg->cycles) { return; }

    w = edge->adjvex;
    if (!dg->visited[w]) {
      dg->edge_to[w] = v;
      DG_dfs_from(dg, w);
    } else if (dg->on_stack[w]) {
      dg->cycles = checked_malloc(
          (dg->verticeCnt + 1) * sizeof(*dg->cycles));
      for (i = v, j = 0; i != w; i = dg->edge_to[v], j++) {
        dg->cycles[j] = i;
      }

      dg->cycles[j++] = w;
      dg->cycles[j++] = v;
    }
  }

  // reverse postorder in a DAG provides a topological order
  dg->tpg[dg->tpg_sort_rev_index++] = v;
  dg->on_stack[v] = 0;
}

void DG_dfs(DG dg)
{
  int i;

  /* now we know the graph's size, so we can initialize the
   * topological array to store the topological sort result
   */
  dg->tpg = checked_malloc(dg->verticeCnt * sizeof(*dg->tpg));

  for (i = 0; i < dg->verticeCnt; i++) {
    if (!dg->vertice[i]) {
      DG_dfs_from(dg, i);
    }
  }
}

int DG_cycle(DG dg)
{
  return dg->cycles;
}
