/* Fichier: "debug.h" */

/*
 * Ce fichier d'entete permet de detecter automatiquement certaines
 * erreurs de gestion memoire.  Les appels aux fonctions de librairie
 * "malloc", "calloc" et "free" sont interceptes pour verifier que
 * tous les blocs de memoire qui sont alloues (avec malloc et calloc)
 * ont ete recuperes (avec free) a la fin de l'execution de "main".
 * Le fichier "debug.h" doit etre inclu au tout debut du fichier C qui
 * l'utilise.
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdlib.h>

int MAIN ();

int nb_blocs;

int main ()
{
  int result;

  nb_blocs = 0;

  result = MAIN ();

  if (nb_blocs != 0)
    fprintf (stderr, "%d BLOCS NON DESALLOUES\n", nb_blocs);

  return result;
}

void *MALLOC (int n, int ligne)
{
  char *p;

  if (n < 0)
    {
      fprintf (stderr,
               "PARAMETRE NEGATIF PASSE A MALLOC A LA LIGNE %d\n",
	       ligne);
      exit (1);
    }

  p = (char*) malloc (n+sizeof (int));

  if (p == NULL)
    return NULL;

  nb_blocs++;

  *(int*)p = -1-n;

  return (void*)(p+sizeof (int));
}

void *CALLOC (int n, int m, int ligne)
{
  char *p;

  if (n < 0 || m < 0)
    {
      fprintf (stderr,
	       "PARAMETRE NEGATIF PASSE A CALLOC A LA LIGNE %d\n",
               ligne);
      exit (1);
    }

  p = (char*) calloc (n*m+sizeof (int),1);

  if (p == NULL)
    return NULL;

  nb_blocs++;

  *(int*)p = -1-n*m;

  return (void*)(p+sizeof (int));
}

void FREE (void *ptr, int ligne)
{
  char *p;
  int n;

  if (ptr == NULL)
    {
      fprintf (stderr, "POINTEUR NUL PASSE A FREE A LA LIGNE %d\n", ligne);
      exit (1);
    }

  p = (char*)ptr - sizeof (int);

  n = -1-*(int*)p;

  if (n < 0)
    {
      fprintf (stderr,
	       "DONNEE DEJA DESALLOUEE PASSEE A FREE A LA LIGNE %d\n",
               ligne);
      exit (1);
    }

  nb_blocs--;

  *(int*)p = 0;

/*
  free (p);
*/
}

#define main MAIN
#define malloc(n) MALLOC(n,__LINE__)
#define calloc(n,m) CALLOC(n,m,__LINE__)
#define free(p) FREE(p,__LINE__)

#endif