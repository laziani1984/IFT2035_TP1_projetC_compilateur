# IFT2035_TP1_projetC_compilateur

IFT2035 – Travail pratique #1 – 2018.10.06
GESTION M´EMOIRE, POINTEURS ET COMPILATION
Marc Feeley
Le TP1 a pour but de vous faire pratiquer la programmation imp´erative et en particulier les concepts
suivants : les ´enonc´es de contrˆole, la gestion m´emoire manuelle, les pointeurs et la compilation des
langages imp´eratifs.
Vous avez `a r´ealiser un programme en C ainsi que r´ediger un rapport contenant une analyse du
programme.
1 Programmation
Vous devez r´ealiser un petit compilateur qui est bas´e sur le programme petit-comp.c qui a ´et´e
pr´esent´e en classe et en d´emonstration et dont le code est disponible sur la page Studium du
cours. Le codage est donc `a faire en langage C. Vous devez exploiter les forces de ce langage pour
exprimer au mieux votre programme (e.g. l’arithm´etique sur les pointeurs et setjmp/longjmp si
c’est appropri´e).
Vous devez modifier le programme petit-comp.c pour atteindre les objectifs suivants :
• Le programme doit d´etecter lorsqu’une allocation m´emoire dynamique a ´echou´ee (e.g. malloc
a retourn´e NULL) et faire un traitement d’exception appropri´e, incluant l’affichage d’un message
indiquant la situation et une terminaison du programme.
• Il faut ´eliminer toutes les fuites de m´emoire. Donc tout espace m´emoire allou´e avec malloc
soit ˆetre r´ecup´er´e avec free avant la fin de l’ex´ecution du programme. Ceci doit se passer
mˆeme si le programme termine `a cause d’une erreur syntaxique dans le programme compil´e
ou un manque de m´emoire. Dans la version d’origine de petit-comp.c les noeuds de l’ASA
ne sont jamais r´ecup´er´es et vous devez donc y rem´edier. Si vous utilisez malloc pour d’autres
allocations, ces espaces m´emoire doivent aussi ˆetre r´ecup´er´es.
• Vous devez ´etendre le compilateur pour qu’il puisse compiler des programmes du langage
pr´ecis´e dans la prochaine section. Cela vous demandera d’´etendre l’analyseur syntaxique, le
g´en´erateur de code, le jeu d’instructions de la machine virtuelle, et l’interpr`ete de bytecode.
2 Langage
Le langage source accept´e par la version d’origine de petit-comp.c est un langage imp´eratif avec
une syntaxe inspir´ee du langage C et des types de donn´ees et structures de contrˆole limit´ees. En
effet il y a seulement le type entier et 26 variables globales implicitement d´eclar´ees (chaque lettre
minuscule de l’alphabet). Il y a seulement les op´erateurs +, -, < et l’affectation. Les ´enonc´es
disponibles sont le if, le while, le do/while et le bloc.
Lorsque le compilateur est ex´ecut´e il lit de son entr´ee standard le programme source `a compiler.
`A
l’aide d’une redirection on peut faire en sorte que le compilateur lise le programme source d’un
fichier. Par exemple :
% ./petit-comp < prog.c
1
Le compilateur lira le contenu du fichier prog.c, l’analysera pour s’assurer qu’il respecte la gram-
maire du langage, construira un ASA qui repr´esente le programme, g´en´erera le bytecode `a partir
de l’ASA et fera l’ex´ecution du bytecode avec l’interpr`ete de la machine virtuelle.
Le compilateur doit ˆetre ´etendu pour accepter le langage sp´ecifi´e par la grammaire suivante. Les
nouveaut´es sont indiqu´ees par un commentaire `a droite (qui ne fait pas partie de la grammaire).
<program> ::= <stat>
<stat> ::= "if" <paren_expr> <stat>
| "if" <paren_expr> <stat> "else" <stat>
| "while" <paren_expr> <stat>
| "do" <stat> "while" <paren_expr> ";"
| ";"
| "{" { <stat> } "}"
| <expr> ";"
| <id> ":" <stat> ***nouveau***
| "break" [ <id> ] ";" ***nouveau***
| "continue" [ <id> ] ";" ***nouveau***
| "goto" <id> ";" ***nouveau***
| "print" <paren_expr> ";" ***nouveau***
<expr> ::= <test>
| <id> "=" <expr>
<test> ::= <sum>
| <sum> "<" <sum>
| <sum> "<=" <sum> ***nouveau***
| <sum> ">" <sum> ***nouveau***
| <sum> ">=" <sum> ***nouveau***
| <sum> "==" <sum> ***nouveau***
| <sum> "!=" <sum> ***nouveau***
<sum> ::= <mult> ***nouveau***
| <sum> "+" <mult> ***nouveau***
| <sum> "-" <mult> ***nouveau***
<mult> ::= <term> ***nouveau***
| <mult> "*" <term> ***nouveau***
| <mult> "/" <term> ***nouveau***
| <mult> "%" <term> ***nouveau***
<term> ::= <id>
| <int>
| <paren_expr>
<paren_expr> ::= "(" <expr> ")"
2
Le langage a des nouvelles formes d’expressions. La cat´egorie <mult> a ´et´e ajout´ee pour les
op´erateur multiplicatifs *, / et %. D’autre part la cat´egorie <test> a ´et´e ´etendue pour compl´eter
les op´erateur de comparaison avec <=, >, >=, == et !=. Ces op´erateurs font le mˆeme calcul qu’en C.
Entre autres, les op´erateurs de comparaison retournent la valeur 0 ou 1, repr´esentant respectivement
“faux” et “vrai”. Notez que vous devrez ajouter des nouvelles instructions et bytecodes `a la machine
virtuelle pour pouvoir compiler ces nouvelles op´erations (inspirez-vous de IADD et IFLT).
Le langage a aussi des nouvelles formes d’´enonc´es (cat´egorie <stat>). Il y a un ´enonc´e print(...)
pour imprimer une valeur enti`ere et passer `a la prochaine ligne. Vous devez ajouter une instruction
`a la machine virtuelle pour implanter le print(...).
Il y a aussi la possibilit´e d’´etiquetter n’importe quel ´enonc´e avec un identificateur (l`a aussi
limit´e aux 26 identificateurs d’une lettre). Une ´etiquette ne peut pas ˆetre r´eutilis´ee ailleurs dans le
programme. Une ´etiquette peut ˆetre sp´ecifi´ee comme destination de l’´enonc´e goto. On peut aussi
´etiquetter un ´enonc´e de boucle et `a l’int´erieur du corps de la boucle avoir un break ou continue
qui cible cet ´enonc´e. Le comportement de ces ´enonc´es est identique au langage Java (sauf que les
´enonc´es bloc ne peuvent pas ˆetre ´etiquett´es). Voici un exemple de programme qui utilise ces ´enonc´es
:
{
  y = 1;
  p:
  while (1) {
    y = y*2;
    if (y > 100) break;
    x = y;
    while (x > 0) {
      if (x == 5) {
      print(y);
      continue p;
      }
    x = x-3;
    }
  }
}
Le compilateur doit signaler une erreur lorsque le programme utilise une ´etiquette dans un goto,
break ou continue qui n’est pas d´eclar´ee, ou qui ne d´esigne pas une boucle englobante dans le cas
de break et continue. Il faut aussi signaler une erreur lorsqu’un break ou continue est utilis´e
ailleurs que dans le corps d’une boucle.
Pour implanter ces ´enonc´es de contrˆole il y a d´ej`a tout ce qu’il faut dans le jeu d’instructions
de la machine virtuelle. Donc n’ajoutez rien de nouveau pour ¸ca. Cependant, les instructions de
branchement de la machine virtuelle sont limit´ees `a -128..+127 comme distance de branchement.
Le compilateur doit donner un message d’erreur `a la compilation si la distance de la cible de
branchement est trop ´eloign´ee (dans la version d’origine cette situation n’est pas v´erifi´ee).
Dans la version d’origine de petit-comp.c `a la fin de l’ex´ecution le contenu des variables ayant
une valeur diff´erente de 0 est affich´e. Ce comportement doit ˆetre retir´e car le langage a maintenant
l’´enonc´e print(...) pour faire des affichages.
Il est primordial d’´eviter les fuites de m´emoire et les pointeurs fous. Lorsque votre programme
sera test´e, nous ferons varier artificiellement l’espace m´emoire disponible `a votre programme. Il est donc tout `a fait possible que n’importe quel appel `a la fonction malloc retournera NULL.
Votre programme C doit seulement inclure les fichiers d’entˆete “stdio.h”, “stdlib.h” et
“strings.h”. Il doit se compiler et ex´ecuter sans erreur sous linux avec les commandes :
% gcc -o ./petit-comp petit-comp.c
% ./petit-comp < prog.c
3 Rapport
Vous devez r´ediger un rapport qui :
1. Explique bri`evement le fonctionnement g´en´eral du programme (maximum de 1 page au total).
2. Explique comment les probl`emes de programmation suivants ont ´et´e r´esolus (en 2 `a 4 pages
au total) :
(a) comment se fait le traitement des manques de m´emoire?
(b) comment se fait le traitement des erreur de syntaxe?
(c) comment se fait la r´ecup´eration de l’espace m´emoire?
(d) comment se fait l’analyse syntaxique des nouvelles formes syntaxiques?
(e) quelles sont les nouvelles instructions que vous avez ajout´ees `a la machine virtuelle?
(f) comment sont implant´es les nouveaux types d’´enonc´es (print, goto, break et continue)?
4 ´Evaluation
• Ce travail compte pour 15 points dans la note finale du cours. Indiquez vos noms clairement
au tout d´ebut du programme. Vous devez faire le travail par groupes de 2 personnes.
Vous devez confirmer la composition de votre ´equipe (noms des co´equipiers) au
d´emonstrateur au plus tard le 11 octobre. Si vous ne trouvez pas de partenaire
d’ici quelques jours, venez me voir.
• Le programme sera ´evalu´e sur 8 points et le rapport sur 7 points. Un programme qui plante `a
l’ex´ecution, mˆeme dans une situation extrˆeme, se verra attribuer z´ero sur 8 (c’est un incitatif
`a bien tester votre programme). Assurez-vous de pr´evoir toutes les situations d’erreur (en
particulier un appel `a la fonction malloc de C qui retourne NULL).
• Vous devez remettre votre rapport (un fichier “.pdf”) et le programme (votre fichier petit-comp.c)
au plus tard le jeudi 1 novembre `a 23:55 *** sur le site Studium du cours.*** Notez que votre
programme doit ˆetre contenu dans un seul fichier.
• L’´el´egance et la lisibilit´e du code, l’exactitude et la performance, la lisibilit´e du rapport, et
l’utilisation d’un fran¸cais sans fautes sont des crit`eres d’´evaluation.
