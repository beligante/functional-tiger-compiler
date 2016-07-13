#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

void *checked_malloc(int len)
{
  void *p = malloc(len);
  if (!p) {
    fprintf(stderr,"\nRan out of memory!\n");
    exit(1);
  }
  return p;
}

string String(char *s)
{
  string p = checked_malloc(strlen(s)+1);
  strcpy(p,s);
  return p;
}

U_boolList U_BoolList(bool head, U_boolList tail)
{
  U_boolList list = checked_malloc(sizeof(*list));
  list->head = head;
  list->tail = tail;
  return list;
}

int max(int a, int b)
{
  return (a > b) ? a : b;
}

char *remove_ext (char* mystr, char dot, char sep) {
    char *retstr, *lastdot, *lastsep;

    if (mystr == NULL)
        return NULL;
    if ((retstr = malloc (strlen (mystr) + 1)) == NULL)
        return NULL;

    strcpy (retstr, mystr);
    lastdot = strrchr (retstr, dot);
    lastsep = (sep == 0) ? NULL : strrchr (retstr, sep);

    if (lastdot != NULL) {
        if (lastsep != NULL) {
            if (lastsep < lastdot) {
                *lastdot = '\0';
            }
        } else {
            *lastdot = '\0';
        }
    }
    return retstr;
}

void indent(FILE *out, int d) {
  int i;
  for (i = 0; i <= d; i++) fprintf(out, " ");
}