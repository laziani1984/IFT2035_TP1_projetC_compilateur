/* fichier: "petit-comp.c" */

/* Un petit compilateur et machine virtuelle pour un sous-ensemble de C.  */
// gcc -o comp -g petit-comp.c
//      valgrind --tool=memcheck --leak-check=yes ./comp < test.c

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <setjmp.h>
#include "debug.h"

jmp_buf env;
int err_code;
/*---------------------------------------------------------------------------*/

/* Analyseur lexical. */

enum { DO_SYM, ELSE_SYM, IF_SYM, WHILE_SYM, PRINT_SYM, LBRA, RBRA, LPAR,
       RPAR, PLUS, MINUS, STAR, SLASH, PERCENT, LESS, LEQTEST, GREATER,
       GEQTEST, EQTEST, NEQTEST, SEMI, EQUAL, INT, ID, EOI };

char *words[] = { "do", "else", "if", "while", "print", NULL };

int ch = ' ';
int sym;
int int_val;
char id_name[100];

void syntax_error() { fprintf(stderr, "syntax error\n"); err_code = 1; }
void malloc_error() { fprintf(stderr, "malloc returned NULL\n"); err_code = 2; }

void next_ch() { ch = getchar(); }

void next_sym()
{
  while (ch == ' ' || ch == '\n') next_ch();
  switch (ch)
    { case '{': sym = LBRA;   next_ch(); break;
      case '}': sym = RBRA;   next_ch(); break;
      case '(': sym = LPAR;   next_ch(); break;
      case ')': sym = RPAR;   next_ch(); break;
      case '+': sym = PLUS;   next_ch(); break;
      case '-': sym = MINUS;  next_ch(); break;
      case '*': sym = STAR;   next_ch(); break;
      case '/': sym = SLASH;  next_ch(); break;
      case '%': sym = PERCENT;next_ch(); break;
      case ';': sym = SEMI;   next_ch(); break;

      //Traitement de symbole d'operation a 2 caracteres
      case '<': sym = LESS;  next_ch();
                if(ch == '=') { sym = LEQTEST; next_ch(); } break;
      case '>': sym = GREATER;  next_ch();
                if(ch == '=') { sym = GEQTEST; next_ch(); } break;
      case '=': sym = EQUAL; next_ch();
                if(ch == '=') { sym = EQTEST; next_ch(); }  break;
      case '!': next_ch();
                if(ch == '=') { sym = NEQTEST; next_ch(); }
                else syntax_error();                        break;


     //Traitement des id et des nombres
      case EOF: sym = EOI;   next_ch(); break;
      default:
        if (ch >= '0' && ch <= '9')
          {
            int_val = 0; /* overflow? */
      
            while (ch >= '0' && ch <= '9')
              {
                int_val = int_val*10 + (ch - '0');
                next_ch();
              }
      
            sym = INT;
          }
        else if (ch >= 'a' && ch <= 'z')
          {
          /*
           on regroupe les char formant un texte, si ce mot est dans words, ie cest un mot
           cle, sym = ce quil represente, sinon cest une variable et par definition du petit c,
           son nom doit etre dúne lettre. id_name sert a contenir ce mot, remis a 0 a chaque fois
           */

           int i = 0; /* overflow? */
      
            while ((ch >= 'a' && ch <= 'z') || ch == '_')
              {
                id_name[i++] = ch;
                next_ch();
              }
      
            id_name[i] = '\0'; // fin de mot
            sym = 0; // Represente do_symb (debut de lenum)
      
            while (words[sym]!=NULL && strcmp(words[sym], id_name)!=0) //(id_name in words)
            //: sym=index du word`
              sym++;
      
            if (words[sym] == NULL) // cas ou id_name not in words
              {
                if (id_name[1] == '\0') sym = ID; else syntax_error();
              }
          }
        else syntax_error();
    }
}

/*---------------------------------------------------------------------------*/

/* Analyseur syntaxique. */


enum { VAR, CST, ADD, SUB, MULT, DIV, MOD, LT, LEQ, GT, GEQ, EQ, NEQ, ASSIGN,
       IF1, IF2, WHILE, DO, PRINT, EMPTY, SEQ, EXPR, PROG };

struct node
  {
    int kind;
    struct node *o1;
    struct node *o2;
    struct node *o3;
    int val;
  };

typedef struct node node;

#define CATCH()fin: if(err_code != 0){  nettoyageASA(x); return NULL; }
#define TEST() if(err_code!=0) goto fin;


node *new_node(int k)
{
  node *x = malloc(sizeof(node));
  if(x == NULL ){malloc_error(); return NULL; }// raise error
  x->kind = k;
  x->o1 = NULL; x->o2 = NULL; x->o3 = NULL; //Iinitialisation
  return x;
}

node *paren_expr(); /* forward declaration */
void nettoyageASA(node* racine);
void print_node(node* n);
void print_tree(node* n);

node *term() /* <term> ::= <id> | <int> | <paren_expr> */
{
  node *x = NULL;

  if (sym == ID)           /* <term> ::= <id> */
    {
      x = new_node(VAR);
      TEST()
      x->val = id_name[0]-'a';
      next_sym();
      TEST()
    }
  else if (sym == INT)     /* <term> ::= <int> */
    {
      x = new_node(CST);
      TEST()
      x->val = int_val;
      next_sym();
      TEST()
    }
  else{                  /* <term> ::= <paren_expr> */
        x = paren_expr();
        TEST()
       }
  CATCH()
  return x;
}

node *mult() /* <term> | <mult> "*" <term> | <mult> "/" <term> | <mult> "%" <term> */
{
    node *x;
    x= term();
    TEST()

    while (sym == STAR || sym == SLASH || sym == PERCENT)//Conversion de la recurence
      {
        node *t = x;
        switch(sym)
          {
            case STAR:    x = new_node(MULT); break;
            case SLASH:   x = new_node(DIV);  break;
            case PERCENT: x = new_node(MOD);  break;
          }
        if(err_code == 2){ nettoyageASA(t); return NULL;}

        next_sym();
        x->o1 = t;
        TEST();
        x->o2 = term();
        TEST()

      }

    CATCH()
    return x;
}

node *sum() /* <mult> | <sum> "+" <mult> | <sum> "-" <mult> */
{
  node *x = mult();
  TEST()

  while (sym == PLUS || sym == MINUS) //Conversion de la recurence
    {
      node *t = x;
      x = new_node(sym==PLUS ? ADD : SUB);

      if(err_code == 2){ nettoyageASA(t); return NULL;} //ALLOC ECHEC

      next_sym();
      x->o1 = t;
      TEST()
      x->o2 = mult();
      TEST()

    }
  CATCH()
  return x;
}

node *test() /* <test> ::= <sum> | <sum> "<" <sum> */
{
  node *x = sum();
  TEST()

  if (sym == LESS)
    {
      node *t = x;
      x = new_node(LT);
      if(err_code == 2) { nettoyageASA(t); return NULL;}
      next_sym();
      x->o1 = t;
      TEST()
      x->o2 = sum();
      TEST()

    }

    else if (sym == LEQTEST)
      {
        node *t = x;
        x = new_node(LEQ);
        if(err_code == 2) { nettoyageASA(t); return NULL;}
        next_sym();
        x->o1 = t;
        TEST()
        x->o2 = sum();
        TEST()

      }

    else if (sym == GREATER)
      {
        node *t = x;
        x = new_node(GT);
        if(err_code == 2) { nettoyageASA(t); return NULL;}
        next_sym();
        x->o1 = t;
        TEST()
        x->o2 = sum();
        TEST()

      }

    else if (sym == GEQTEST)
      {
        node *t = x;
        x = new_node(GEQ);
        if(err_code == 2) { nettoyageASA(t); return NULL;}
        next_sym();
        x->o1 = t;
        TEST()
        x->o2 = sum();
        TEST()

      }

    else if (sym == EQTEST)
      {
        node *t = x;
        x = new_node(EQ);
        if(err_code == 2) { nettoyageASA(t); return NULL;}
        next_sym();
        x->o1 = t;
        TEST()
        x->o2 = sum();
        TEST()

      }

    else if (sym == EQTEST)
      {
        node *t = x;
        x = new_node(EQ);
        if(err_code == 2) { nettoyageASA(t); return NULL;}
        next_sym();
        x->o1 = t;
        TEST()
        x->o2 = sum();
        TEST()

      }
    else if (sym == NEQTEST)
      {
        node *t = x;
        x = new_node(NEQ);
        if(err_code == 2) { nettoyageASA(t); return NULL;}
        next_sym();
        x->o1 = t;
        TEST()
        x->o2 = sum();
        TEST()

      }
  CATCH()
  return x;

}

node *expr() /* <expr> ::= <test> | <id> "=" <expr> */
{
  node *x = NULL;

  if (sym != ID) return test();

  x = test();
  TEST()

  if (sym == EQUAL)
    {
      node *t = x;
      x = new_node(ASSIGN);
      if(err_code == 2) { nettoyageASA(t); return NULL;}
      next_sym();
      x->o1 = t;
      x->o2 = expr();
      TEST()

    }
  CATCH()
  return x;
}

node *paren_expr() /* <paren_expr> ::= "(" <expr> ")" */
{
  node *x = NULL;

  if (sym == LPAR) next_sym(); else syntax_error();
  TEST()

  x = expr();
  TEST()

  if (sym == RPAR) next_sym();
  else syntax_error();
  TEST()

  CATCH()
  return x;
}

node *statement()
{
  node *x = NULL;

  if (sym == IF_SYM)       /* "if" <paren_expr> <stat> */
    {
      x = new_node(IF1);
      TEST()
      next_sym();
      TEST()
      x->o1 = paren_expr();
      TEST()
      x->o2 = statement();
      TEST()
      if (sym == ELSE_SYM) /* ... "else" <stat> */
        { x->kind = IF2;
          next_sym();
          TEST()
          x->o3 = statement();
          TEST()
        }
    }
  else if (sym == WHILE_SYM) /* "while" <paren_expr> <stat> */
    {
      x = new_node(WHILE);
      TEST()
      next_sym();
      TEST()
      x->o1 = paren_expr();
      TEST()
      x->o2 = statement();
      TEST()
    }
  else if (sym == DO_SYM)  /* "do" <stat> "while" <paren_expr> ";" */
    {
      x = new_node(DO);
      TEST()
      next_sym();
      x->o1 = statement();
      TEST()
      if (sym == WHILE_SYM) next_sym(); else syntax_error();
      TEST()
      x->o2 = paren_expr();
      TEST()
      if (sym == SEMI) next_sym(); else syntax_error();
      TEST()
    }

   else if (sym == PRINT_SYM)  /* "print" <paren_expr> ";" */
    {
      x = new_node(PRINT);
      TEST()
      next_sym();
      TEST()
      x->o1 = paren_expr();
      TEST()
      if (sym == SEMI) next_sym(); else syntax_error();
      TEST()

    }

  else if (sym == SEMI)    /* ";" */
    {
      x = new_node(EMPTY);
      TEST()
      next_sym();
      TEST()
    }
  else if (sym == LBRA)    /* "{" { <stat> } "}" */
    {
      x = new_node(EMPTY);
      TEST()
      next_sym();
      TEST()

      while (sym != RBRA )
        {
          if(err_code != 0) goto fin; //On evite des boucle infini lors d'un longjmp
          node *t = x;
          x = new_node(SEQ);
          if(err_code == 2) { nettoyageASA(t); return NULL;}
          x->o1 = t;
          x->o2 = statement();
          TEST()
        }

      next_sym();
      TEST()
    }
  else                     /* <expr> ";" */
    {
      x = new_node(EXPR);
      TEST()

      x->o1 = expr();
      TEST()
      if (sym == SEMI) next_sym(); else syntax_error();
      TEST()
    }

  /*
    Dans le cas d'un appel a une des fonction derreur, on va liberer tous le sous-arbre creer,
    et on retourne NULL pour permettre aux autres arbres de se liberer sans double free.
  */

  CATCH()

  return x;

}
/*
    Fonction helper: afficher le noeud, suivie dun espace
*/

void print_node(node* n)
{

    if( n == NULL ){ puts("NULL "); return;}

    switch(n->kind)
    {
        case 0: puts("var "); break;
        case 1: puts("cst "); break;
        case 2: puts("add "); break;
        case 3: puts("sub "); break;
        case 4: puts("lt "); break;
        case 5: puts("assign "); break;
        case 6: puts("if1 "); break;
        case 7: puts("if2 "); break;
        case 8: puts("while "); break;
        case 9: puts("do "); break;
        case 10: puts("print "); break;
        case 11: puts("empty "); break;
        case 12: puts("seq "); break;
        case 13: puts("expr "); break;
        case 14: puts("prog "); break;
    }
}

/*
    On fait des liberations dans un parcours semblant a postfix, de maniere
    recursive

*/
void nettoyageASA(node* racine)
{
    if(racine == NULL) return; //cas de base

    nettoyageASA(racine->o1);
    nettoyageASA(racine->o2);
    nettoyageASA(racine->o3);

    free(racine);

}


/*

*/
node *program()  /* <program> ::= <stat> */
{
  node *x = NULL;
  x = new_node(PROG);
  TEST()

  next_sym();
  TEST()
  x->o1 = statement();
  TEST()


  CATCH()
  return x;

}

/*---------------------------------------------------------------------------*/

/* Generateur de code. */

enum { ILOAD, ISTORE, BIPUSH, DUP, POP, IADD, ISUB, IMULT, IDIV, IMOD, IPRINT,
       GOTO, IFEQ, IFNE, IFLT, RETURN };

typedef signed char code;

code object[1000], *here = object;

void gen(code c) { *here++ = c; } /* overflow? */

#ifdef SHOW_CODE
#define g(c) do { printf(" %d",c); gen(c); } while (0)
#define gi(c) do { printf("\n%s", #c); gen(c); } while (0)
#else
#define g(c) gen(c)
#define gi(c) gen(c)
#endif

void fix(code *src, code *dst) { *src = dst-src; } /* overflow? */

void c(node *x)
{ switch (x->kind)
    { case VAR   : gi(ILOAD); g(x->val); break;

      case CST   : gi(BIPUSH); g(x->val); break;

      case ADD   : c(x->o1); c(x->o2); gi(IADD); break;

      case SUB   : c(x->o1); c(x->o2); gi(ISUB); break;

      case MULT  : c(x->o1); c(x->o2); gi(IMULT); break;

      case DIV   : c(x->o1); c(x->o2); gi(IDIV); break;

      case MOD   : c(x->o1); c(x->o2); gi(IMOD); break;

      case LT    : gi(BIPUSH); g(1);
                   c(x->o1);
                   c(x->o2);
                   gi(ISUB);
                   gi(IFLT); g(4);
                   gi(POP);
                   gi(BIPUSH); g(0); break;

      case LEQ   : gi(BIPUSH); g(0); //Equivalent a not ( o2 < o1)
                   c(x->o2);
                   c(x->o1);
                   gi(ISUB);
                   gi(IFLT); g(4);
                   gi(POP);
                   gi(BIPUSH); g(1); break;

      case GT   :  gi(BIPUSH); g(1); //Equivalent a ( o2 < o1)
                   c(x->o2);
                   c(x->o1);
                   gi(ISUB);
                   gi(IFLT); g(4);
                   gi(POP);
                   gi(BIPUSH); g(0); break;

      case GEQ   :  gi(BIPUSH); g(0); //Equivalent a  not ( o1 < o2)
                   c(x->o1);
                   c(x->o2);
                   gi(ISUB);
                   gi(IFLT); g(4);
                   gi(POP);
                   gi(BIPUSH); g(1); break;

      case EQ    : gi(BIPUSH); g(1);
                   c(x->o1);
                   c(x->o2);
                   gi(ISUB);
                   gi(IFEQ); g(4);
                   gi(POP);
                   gi(BIPUSH); g(0); break;

      case NEQ   : gi(BIPUSH); g(0);
                   c(x->o1);
                   c(x->o2);
                   gi(ISUB);
                   gi(IFEQ); g(4);
                   gi(POP);
                   gi(BIPUSH); g(1); break;

      case ASSIGN: c(x->o2);
                   gi(DUP);
                   gi(ISTORE); g(x->o1->val); break;

      case IF1   : { code *p1;
                     c(x->o1);
                     gi(IFEQ); p1 = here++;
                     c(x->o2); fix(p1,here); break;
                   }

      case IF2   : { code *p1, *p2;
                     c(x->o1);
                     gi(IFEQ); p1 = here++;
                     c(x->o2);
                     gi(GOTO); p2 = here++; fix(p1,here);
                     c(x->o3); fix(p2,here); break;
                   }

      case WHILE : { code *p1 = here, *p2;
                     c(x->o1);
                     gi(IFEQ); p2 = here++;
                     c(x->o2);
                     gi(GOTO); fix(here++,p1); fix(p2,here); break;
                   }

      case DO    : { code *p1 = here; c(x->o1);
                     c(x->o2);
                     gi(IFNE); fix(here++,p1); break;
                   }

      case PRINT : { c(x->o1);
                     gi(IPRINT); break;
                   }

      case EMPTY : break;

      case SEQ   : c(x->o1);
                   c(x->o2); break;

      case EXPR  : c(x->o1);
                   gi(POP); break;

      case PROG  : c(x->o1);
                   gi(RETURN); break;
    }

}

/*---------------------------------------------------------------------------*/

/* Machine virtuelle. */

int globals[26];

void run()
{
  int stack[1000], *sp = stack; /* overflow? */
  code *pc = object;

  for (;;)
    switch (*pc++)
      {
        case ILOAD : *sp++ = globals[*pc++];             break;
        case ISTORE: globals[*pc++] = *--sp;             break;
        case BIPUSH: *sp++ = *pc++;                      break;
        case DUP   : sp++; sp[-1] = sp[-2];              break;
        case POP   : --sp;                               break;
        case IADD  : sp[-2] = sp[-2] + sp[-1]; --sp;     break;
        case ISUB  : sp[-2] = sp[-2] - sp[-1]; --sp;     break;
        case IMULT : sp[-2] = sp[-2] * sp[-1]; --sp;     break;
        case IDIV  : sp[-2] = sp[-2] / sp[-1]; --sp;     break;
        case IMOD  : sp[-2] = sp[-2] % sp[-1]; --sp;     break;
        case IPRINT: printf("%d\n", *--sp);              break;
        case GOTO  : pc += *pc;                          break;
        case IFEQ  : if (*--sp==0) pc += *pc; else pc++; break;
        case IFNE  : if (*--sp!=0) pc += *pc; else pc++; break;
        case IFLT  : if (*--sp< 0) pc += *pc; else pc++; break;
        case RETURN: return;
    }
}

/*---------------------------------------------------------------------------*/

/* Programme principal. */

int main()
{
  int i;
  node* x = program();
  if (x == NULL ) return 1; //Erreur de syntax ou d'allocation
  c(x);
  nettoyageASA(x);


#ifdef SHOW_CODE
  printf("\n");
#endif

  for (i=0; i<26; i++)
    globals[i] = 0;

  run();

  for (i=0; i<26; i++)
    if (globals[i] != 0)
      printf("%c = %d\n", 'a'+i, globals[i]);

  return 0;
}

/*---------------------------------------------------------------------------*/
