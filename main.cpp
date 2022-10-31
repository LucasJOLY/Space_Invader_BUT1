/*!
*\file main.cpp
*\brief  SAÉ 1.02 Space Invader
*\authors David GINI, Lucas JOLY,Mathieu YAHIA-AMAR
*\date 13 Janvier 2022
* Programme pour le rendu du projet de SAÉ 1.02
* Jeu Space Invader
*/
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <random>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fstream>

using namespace std;

/*!
*\fn lectureConfigChar
*\brief Fonction qui recupere les paramètres (caractères) du fichier YAML pour les utiliser
*\param paramConfig  Recherche dans le fichier le nom de la configuration
*\return valeur
*/
char lectureConfigChar (string paramConfig){
    ifstream ifs ("../SpaceInvaders1/config.yaml"); // Apelle le fichier .yaml
    //si le fichier n'existe pas
    if (!ifs){ // test si le fichier existe
        cerr << "fichier n'existe pas" <<endl;
        //on arrete le pgm
        exit (-1);
    }
    for (string str; ifs >> str ; ){ // Pour toutes les lignes du fichier
        if(str == paramConfig){ // On compare la valeur avant les ":" a notre nom de configuration
        //on extrait le ':'
        char deuxPoint;
        ifs >> deuxPoint;
        if (deuxPoint != ':') { // si l'espacement entre la clé et la valeur n'est pas ":"
            cerr << "mauvais caractère" <<endl;
            //on arrete le pgm
            exit (-1);
        }
        getline (ifs, str); // On recuprere la valeur
        str.erase(0, 1); // On efface l'espace entre la valeur et le ":"
        char valeur = str[0]; // On met notre valeur en type caractère
        return valeur; // On renvoie la valeur
        }
    }
    ifs.close(); // On ferme le fichier
}
/*!
*\fn lectureConfigString
*\brief Fonction qui recupere les paramètres (string) du fichier YAML pour les utiliser
*\param paramConfig  Recherche dans le fichier le nom de la configuration
*\return valeur
*/
string lectureConfigString (string paramConfig){
    ifstream ifs ("../SpaceInvaders1/config.yaml"); // Apelle le fichier .yaml
    //si le fichier n'existe pas
    if (!ifs){ // test si le fichier existe
        cerr << "fichier n'existe pas" <<endl;
        //on arrete le pgm
        exit (-1);
    }
    for (string str; ifs >> str ; ){ // Pour toutes les lignes du fichier
        if(str == paramConfig){ // On compare la valeur avant les ":" a notre nom de configuration
        //on extrait le ':'
        char deuxPoint;
        ifs >> deuxPoint;
        if (deuxPoint != ':') { // si l'espacement entre la clé et la valeur n'est pas ":"
            cerr << "mauvais caractère" <<endl;
            //on arrete le pgm
            exit (-1);
        }
        getline (ifs, str); // On recuprere la valeur
        str.erase(0, 1); // On efface l'espace entre la valeur et le ":""
        string valeur = str; // On met notre valeur en type string
        return valeur; // On renvoie la valeur
        }
    }
    ifs.close(); // On ferme le fichier
}

const char KRight               = lectureConfigChar("DROITE");  // Déplacement vers la droite
const char KLeft                = lectureConfigChar("GAUCHE");  // Déplacement vers la gauche
const char KShoot               = lectureConfigChar("TIRER");  // Lancer de torpille

//  Constantes liées à l'envahisseur

const char KTrooper = lectureConfigChar("TROOPER"); //Forme du trooper
const char KRanger = lectureConfigChar("RANGER"); //Forme du ranger
const char KTank = lectureConfigChar("TANK"); // Forme du tank
const char KTorpedo = lectureConfigChar("TORPILLE"); // Forme de la torpille
const char KBalle = lectureConfigChar("BALLE"); // Forme des balles

// Constantes liées au joueur

const string Difficulty (lectureConfigString("DIFFICULTE")); // Niveau de difficulté
const char KJoueur = 'A'; // Forme du joueur
const char KMissile = '|'; // Forme du missile

// Constantes liées à l'eapace (l'écran)

const unsigned nLignes   = 10+2;   // Nombre de lignes de l'écran (de l'espace)
const unsigned nColonnes  = 15+2;   // Nombre de colonnes de l'écran (de l'espace)


unsigned KRatioMeInvaders = 4; // Nombre de fois où c'est le tour du joueur pour un tour de l'envahisseur
unsigned KTour = 1;


struct termios saved_attributes; // Structure utilisée pour le mode non canonique

/*!
*\fn reset_input_mode
*\brief Fonction qui permet de renitialiser l'utilisation des touches en mode canonique.
*\author TP10 de M.Casali
*/
void reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

/*!
*\fn set_input_mode
*\brief Fonction qui permet de rendre l'utilisation des touches de déplacement ou de tir en mode non-canonique
*\author TP10 de M.Casali
*/
void set_input_mode (void) // Fonction TP 10 pour mode non canonique
{
  struct termios tattr;
  /* Make sure stdin is a terminal. */
  if (!isatty (STDIN_FILENO))
    {
      fprintf (stderr, "Not a terminal.\n");
      exit (EXIT_FAILURE);
    }

  /* Save the terminal attributes so we can restore them later. */
  tcgetattr (STDIN_FILENO, &saved_attributes);
  atexit (reset_input_mode);

  /* Set the funny terminal modes. */
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  tattr.c_cc[VMIN] = 0;
  tattr.c_cc[VTIME] = 4;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}


/*!
*\struct missile
 * \brief Structure des missiles du héro
 */
struct missile
{
    char carMissile; /*!< Caractere du missile (sa forme) */
    unsigned posX; /*!< Position X du missile */
    unsigned posY; /*!< Position Y du missile */
    bool isAlive; /*!< Etat de vie du missile en vie (true) ou non (false) */
};

/*!
 *\fn creerMissile
 * \brief  Cette fonction permet de creer un missile avec la structure missile
 * \param posX position X du missile
 * \param posY position Y du missile
 * \return missile
 */
missile creerMissile(unsigned posX, unsigned posY){
    missile MyMissile; // Creer un objet MyMissile d ela structure missile
    MyMissile.carMissile = KMissile; // assigne le caractère correspondant parmis les constantes
    MyMissile.posX = posX; // Assigne la position X
    MyMissile.posY = posY;// Assigne la position Y
    MyMissile.isAlive = false; // Le missile est de base mort
    return MyMissile; // On renvoie l'object
}

missile Missile = creerMissile(0,0);    // Creation du missile "Missile" positioné en 0,0

/*!
 *\struct vaisseau
 * \brief Structure du vaisseau de notre personnage
 */
struct vaisseau
{
    char carVaisseau; /*!< Caractère (forme) du vaisseau */
    unsigned posX; /*!< Position X du vaisseau */
    unsigned posY; /*!< Position Y du vaisseau */
    bool isAlive; /*!< Etat de vie du vaisseau en vie (true) ou non (false) */
    unsigned pv; /*!< Point de vie du vaisseau */
};

/*!
 * \fn creerVaisseau
 * \brief Fonction qui crée le vaisseau de notre personnage
 * \param posX Position X du vaisseau
 * \param posY Position Y du vaisseau
 * \return vaisseau
 */
vaisseau creerVaisseau(unsigned posX, unsigned posY){
    vaisseau MyPerso; // Création de l'object MyPerso selon la structure vaisseau
    MyPerso.carVaisseau = KJoueur; // On assigne la forme grâce a la constante KJoueur
    if (Difficulty == "facile"){ // Si la difficulté est facile
        MyPerso.pv = 6; // Alors pv = 6
    }
    if (Difficulty == "normal"){ // Si la difficulté est normal
        MyPerso.pv = 3; // Alors pv = 3
    }
    if (Difficulty == "difficile"){ // Si difficulté est difficile
        MyPerso.pv = 1; // Alors pv =1
    }
    MyPerso.posX = posX; // On assigne la position X
    MyPerso.posY = posY; // On assigne la position Y
    MyPerso.isAlive = true; // On assigne le fait que le vaisseau soit en vie
    return MyPerso; // On renvoie l'objet
}
/*!
*\struct invader
* \brief Structure des Invaders
*/
struct invader
{
    unsigned id; /*!< Identifiant de l'invader */
    string classe; /*!< Classe de l'invader */
    char carInvader; /*!< Caractère (forme) de l'invader */
    unsigned posX; /*!< Position X de l'invader */
    unsigned posY; /*!< Position Y de l'invader */
    string sensMove = "droite"; /*!< Sens de mouvement au départ de l'invader */
    bool isAlive = true; /*!< Etat de vie du vaisseau en vie (true) ou non (false) de base il est true */
    unsigned hp; /*!< Point de vie de l'invader */
};

/*!
 * \fn creerEnnemi
 * \brief Fonction qui créer un invader
 * \param id Son identifiant
 * \param classe Sa classe
 * \param posX Sa position X
 * \param posY Sa position Y
 * \param sensMove Son sens de mouvement
 * \return invader
 */
invader creerEnnemi(unsigned id, string classe, unsigned posX, unsigned posY, string sensMove) //note - afficher tous les invaders d'un coup, puis uniquement gérer les déplacements
{
    invader vaisseauEnnemi; // On créer l'objet vaisseauEnnemi de la structure invader
    vaisseauEnnemi.id = id; // On assigne l'identifiant entré en paramètre
    vaisseauEnnemi.classe = classe; // On assigne la classe entré en paramètre
    vaisseauEnnemi.posX = posX; // On assigne la position X
    vaisseauEnnemi.posY = posY; // On assigne la position Y
    vaisseauEnnemi.sensMove = sensMove; // On assigne le sens de mouvmement entré en paramètre

    if (classe == "ranger") // Si la classe est ranger
    {
        vaisseauEnnemi.hp = 1; // ON assigne 1 PV
        vaisseauEnnemi.carInvader = KRanger; // On assigne le caractère correspondant dans les constantes ici : KRanger
    }
    else if (classe == "trooper") // Si la classe est trooper
    {
        vaisseauEnnemi.hp = 2; // On assigne 2 PV
        vaisseauEnnemi.carInvader = KTrooper; // On lui donne le caractère de la constante KTrooper
    }
    else if (classe == "tank") // Si la classe est tank
    {
        vaisseauEnnemi.hp = 5; // On assigne 5 PV
        vaisseauEnnemi.carInvader = KTank; // On lui donne le caractère de la constante KTank
    }

    return vaisseauEnnemi; // On renvoie l'objet
}

unsigned score = 0; // Initialision du score a 0 de base
vaisseau vaisseauJoueur = creerVaisseau(nColonnes / 2, nLignes - 2); // Création du vaisseau de notre personnage positioné au milieu de la derniere ligne

/*!
 * \fn iterInvader
 * \brief Instancie et place trois lignes d'invaders de classes aléatoires peu importe la taille de l'espace
 * \return listeEnnemi
 */
vector<invader> iterInvader ()
{
    vector<invader> listeEnnemi; //Liste de chaque entité invader
    unsigned ligne = 1; //Ligne actuelle
    unsigned tabEstImpair = 0; //Indique si la taille de l'espace est impaire
    string sensMove = "droite"; //Sens de mouvement actuel
    string invaderClass; //Classe de l'invader
    unsigned seed; //Graine de génération aléatoire
    srand(time(NULL)); //Random seed

    if (nColonnes % 2 != 0) //Vérifie si l'espace est impair
        tabEstImpair = 1;

    for (unsigned i = 0; i < (nColonnes - 2)*1.5; ++i) // Pour N invaders - N étant le nombre nécessaire pour remplir 3 lignes
    {
        seed = rand() % 100 + 1; //Nombre aléatoire pour déterminer les classes
        if (seed > 1 && seed < 51) //50% de chances d'être un trooper
            invaderClass = "trooper";
        else if (seed > 51 && seed < 81) //30% de chances d'être un ranger
            invaderClass = "ranger";
        else //20% de chances d'être un tank
            invaderClass = "tank";

        if (i == 0) //Si premier invader
            listeEnnemi.push_back(creerEnnemi(0,invaderClass,1,1,"droite")); //On le place en haut à gauche du tableau

        else if (ligne % 2 == 0) //Sinon, si on est sur une ligne paire
        {
            if (listeEnnemi[i-1].posX - 2 + tabEstImpair < 1 + tabEstImpair) //Est ce que l'invader d'avant était collé au bord du tableau?
            {
                sensMove = "droite"; //On change de sens de mouvement
                listeEnnemi.push_back(creerEnnemi(i,invaderClass,1,listeEnnemi[i-1].posY+1,sensMove)); //On place l'invader une ligne plus bas, à gauche
                ++ligne; //On passe à la ligne suivante
            }
            else
            {
                listeEnnemi.push_back(creerEnnemi(i,invaderClass, listeEnnemi[i-1].posX-2, listeEnnemi[i-1].posY, sensMove)); //Sinon, on le place à sa gauche
            }
         }

        else //Sinon, on est sur une ligne impaire
        {
            if (listeEnnemi[i-1].posX + 1 >= nColonnes - 2) //Est ce que l'invader d'avant était collé au bord du tableau?
            {
                sensMove = "gauche"; //On change le sens de mouvement
                listeEnnemi.push_back(creerEnnemi(i,invaderClass,nColonnes - 2 - tabEstImpair,listeEnnemi[i-1].posY+1,sensMove)); //On place l'invader une ligne plus bas, à droite
                ++ligne; //On passe à la ligne suivante
            }
            else
            {
                listeEnnemi.push_back(creerEnnemi(i,invaderClass, listeEnnemi[i-1].posX+2, listeEnnemi[i-1].posY, sensMove)); //Sinon, on le place à sa droite
            }
        }
    }
    return listeEnnemi; //On renvoie le vecteur listeEnnemi avec tous les invaders bien instanciés
}

vector <invader> listeEnnemi = iterInvader(); // Création des invaders

/*!
 * \brief Fonction qui gère la position de tous les Invaders
 */
void manageInvaders () // Fonction qui gère les Invaders
{
    if (KTour == KRatioMeInvaders) // Si le tour actuel = le rapport du nombre de tour des Invaders
    {
        for (unsigned i = 0; i < listeEnnemi.size(); ++i) // Boucle de 0 jusqu'au nombre max d'ennemis
        {
            if(listeEnnemi[i].sensMove == "droite") // Si l'invader va à droite
            {
                if (listeEnnemi[i].posX+1 == nColonnes-1) // Si la position X de l'invader +1 == au nombre de colonnes -1
                {
                    listeEnnemi[i].posY = listeEnnemi[i].posY + 1; // La position Y de l'invader +1
                    listeEnnemi[i].sensMove = "gauche"; // L'invader va à gauche
                }
                else
                {
                    listeEnnemi[i].posX = listeEnnemi[i].posX + 1; // Sinon la position X de l'invader +1
                }
            }
            else
            {
                if (listeEnnemi[i].posX-1 == 0) // Si la position X-1 de l'invader vaut 0
                {
                    listeEnnemi[i].posY = listeEnnemi[i].posY + 1; // La position Y de l'invader +1
                    listeEnnemi[i].sensMove = "droite"; // L'invader va à droite
                }
                else
                {
                    listeEnnemi[i].posX = listeEnnemi[i].posX - 1; // La position X de l'Invader -1
                }
            }
        }
        KTour = 0; // Le nombre de tour = 0
    }
    else
        ++KTour; // Sinon +1 tour
}
/*!
 * \brief Structure de la torpille
 * \struct torpille
 */
struct torpille // Structure d'une torpille
{
    unsigned id; /*!< Identifiant de la torpilles */
    char carTorpille = KTorpedo; /*!< Caractère de la torpille */
    unsigned posX; /*!< Position X de la torpille */
    unsigned posY; /*!< Position Y de la torpille */
    unsigned dmg; /*!< Nombre de dégats de la torpille */
    bool isAlive = false; /*!< Statut du torpille définit de base sur non existante */
};
/*!
 * \fn creerTorpille
 * \brief Fonction qui créer la torpille avec toute les informations
 * \param id
 * \param posX
 * \param posY
 * \param classeTorpille
 * \return
 */
torpille creerTorpille (unsigned id, unsigned posX, unsigned posY, string classeTorpille) // Fonction de création des torpilles
{
    torpille Torpille;
    Torpille.id = id; // Identifiant de la torpille
    Torpille.isAlive = true; // Etat de la torpille
    Torpille.posX = posX; // Position X de la torpille
    Torpille.posY = posY; // Position Y de la torpille
    if (classeTorpille == "ranger")
    {
        Torpille.dmg = 3; // Dégats de la torpille du ranger = 3
        Torpille.carTorpille = KBalle;
    }
    else
        Torpille.dmg = 1; // Degat d'un torpille classique

    return Torpille;
}

vector<torpille> listeTorpilles; // Tableau de la liste des torpilles
bool salveEnCours;

/*!
 * \brief Fonction qui gère quelle classe peut tirer des torpilles
 */
void shootTorpille () // Fonction qui gère le tir de la torpille
{
    unsigned maxY = 1; // Postion la plus basse des invaders
    if (!salveEnCours) // Si la salve n'est pas en cours
    {
        for (unsigned i = 0; i < listeEnnemi.size(); ++i) // Boucle de 0 jusqu'au nombre d'ennemis max
        {
            if (listeEnnemi[i].posY > maxY && listeEnnemi[i].isAlive) // Si la postion Y de l'ennemi est sup à la postion la plus basse des invader et que l'invader est vivant
                maxY = listeEnnemi[i].posY; // Position la plus basse des invader = la position de l'invader
        }
        for (unsigned j = 0; j < listeEnnemi.size(); ++j) // Boucle de 0 jusqu'au nombre d'ennemis max
        {
            if (listeEnnemi[j].posY == maxY && listeEnnemi[j].isAlive && listeEnnemi[j].classe != "tank") // Si la position Y de l'invader vaut la position mini des invader et que l'invader est vivant et que la classe de l'invader n'est "tank"
            {
                listeTorpilles.push_back(creerTorpille(listeEnnemi[j].id, listeEnnemi[j].posX, listeEnnemi[j].posY, listeEnnemi[j].classe)); // Création d'une case en plus dans la liste des torpilles
            }
        }
        salveEnCours = true; // Activation de la salve
    }
}

/*!
 * \fn manageTorpille
 * \brief Gère les déplacements de toutes les torpilles et leur collision avec les bords du tableau/le vaisseau
 */
void manageTorpille ()
{
    for (unsigned i = 0; i < listeTorpilles.size(); ++i) //Pour chaque entité torpille répertoriée
    {
        ++listeTorpilles[i].posY; //On fait avancer d'une case
        if (listeTorpilles[i].posX == vaisseauJoueur.posX && listeTorpilles[i].posY == vaisseauJoueur.posY) //Si la position de n'importe quelle torpille est égale à la position du joueur
        {
            if (listeTorpilles[i].dmg > vaisseauJoueur.pv) //Condition pour éviter un résultat négatif
                vaisseauJoueur.pv = 0; //On met les points de vie à 0 à la place
            else
                vaisseauJoueur.pv = vaisseauJoueur.pv - listeTorpilles[i].dmg; //On enlève des points de vie en fonction des dégâts de la torpille

            if (score < 100) //Condition pour éviter un résultat négatif
                score = 0; //On met le score à 0 à la place
            else
                score = score - 100; //On enlève 100 points au score total

            listeTorpilles[i].isAlive = false; //Après la collision, la torpille "meurt"
        }
    }

    for (unsigned j = 0; j < listeTorpilles.size(); ++j) //On parcourt toutes les entités torpilles
        if (!listeTorpilles[j].isAlive || listeTorpilles[j].posY > nColonnes) //Si elle est vivante mais en dehors du tableau
        {
            listeTorpilles.erase(listeTorpilles.begin()+j); //On la supprime
        }
    if (listeTorpilles.size() == 0) //S'il ne reste plus de torpilles
        salveEnCours = false; //Alors la salve est finie
}

/*!
 * \fn shootMissile
 * \brief Fonction qui gère le missile
 */
void shootMissile(){

  if (Missile.isAlive == true){ // Si le missile existe
     --Missile.posY; // La position Y du missile diminue de 1

      for (unsigned k = 0; k < listeEnnemi.size(); ++k) // Boucle de 0 au nombre d'ennemi max
        if(Missile.posX == listeEnnemi[k].posX && Missile.posY == listeEnnemi[k].posY && Missile.isAlive == true && listeEnnemi[k].isAlive == true ){ // Si la position X du missile
            cout << k; // On affiche k
            Missile.isAlive = false; // Le missile "meurt"
            Missile.posY = 0;
            Missile.posX = 0;
            --listeEnnemi[k].hp; // Les points de vie de l'ennemi diminuent
            if(listeEnnemi[k].hp == 0){
                listeEnnemi[k].isAlive = false; // Si tous les ennemis sont morts
                score = score + 300; // Le score augemente de 300
            }
        }
      if(Missile.posY == 0){
          Missile.isAlive = false; // le missile "meurt"
      }
   }
}

/*!
 * \fn manageHero
 * \brief Fonction qui gère le héros (position etc)
 */
void manageHero(){
    char Key;
    set_input_mode(); // Fonction pour l'entrée
    for( unsigned i = 0; i<1; i++){
        read (STDIN_FILENO, &Key, 1);
        if( Key == KLeft && vaisseauJoueur.posX != 1){ // Si la touche pour aller à gauche est préssée et que le héros n'est pas au bord du tableau
            vaisseauJoueur.posX = vaisseauJoueur.posX - 1;  // Le héros va à gauche
        }
        else if(Key == KRight && vaisseauJoueur.posX != nColonnes - 2){ // Si la touche pour aller à droite est préssée et que le héros n'est pas au bord du tableau
            vaisseauJoueur.posX = vaisseauJoueur.posX + 1;  // Le héros ca à droite
        }
        else if( Key == KShoot && !Missile.isAlive){ // Si la touche pour tirer est préssée et que le missile existe
        {
            Missile.isAlive = true;
            Missile.posY = vaisseauJoueur.posY;
            Missile.posX = vaisseauJoueur.posX;
            shootMissile(); // Fonction pour déplacer la torpille
        }
        }
    }
}

/*!
 * \fn ClearScreen
* \brief Fonction qui nettoie le terminal
*/
void ClearScreen ()
{
    cout << "\033[H\033[2J"; // Reinitialise le terminal
}

/*!
 * \fn afficherTableau
 * \brief Fonction qui affiche le tableau et les positions de tout les objets présent
 */
void afficherTableau(){
    ClearScreen(); // On nettoie le terminal

    char tableau[nColonnes][nLignes]; // On créer une matrice qui dépend des constantes nColonnes et nLignes

    //Choix des caractères ASCII pour chaque élément du tableau
    unsigned caraVertical = 45;
    unsigned caraHorizontal = 124;
    unsigned coinCadre = 43;

    // Créations des bordures et du vide à l'intérieur
    for (unsigned i = 0; i < nLignes; ++i) // On parcourt les lignes du tableau
    {
        for (unsigned j = 0; j < nColonnes; j++) // On parcourt les colonnes
        {
            if (j == 0 || j == nColonnes - 1) // Si j vaut 0 ou alors j vaut colonnes -1
            {
                tableau[j][i] = caraHorizontal; // A la position [j][i] de la matrice on met le caractère horizontal
            }
            else if (i == 0 || i == nLignes - 1) // Si i vaut 0 ou alors i vaut lignes -1
            {
                tableau[j][i] = caraVertical; // A la position [j][i] de la matrice on met le caractère vertical
            }
            else
            {
                tableau[j][i] = ' '; // Sinon on met un espace qui correspond au vide
                for (unsigned k = 0; k < listeEnnemi.size(); ++k) // On parcourt tous les invaders
                {
                    if (j == listeEnnemi[k].posX && i == listeEnnemi[k].posY) // Si j vaut la position X de l'invader et i vaut la position Y de l'invader
                    {
                        if(listeEnnemi[k].isAlive == true) // Si l'invader est en vie
                            tableau[j][i] = listeEnnemi[k].carInvader;  // Alors a la positon [j][i] de la matrice on met le caractère de l'invader
                    }
                 }

                for (unsigned l = 0; l < listeTorpilles.size(); ++l) // On parcourt toutes les torpilles
                {
                    if (j == listeTorpilles[l].posX && i == listeTorpilles[l].posY) // Si j vaut la position X de la torpille et i vaut la position Y de la torpille
                        tableau[j][i] = listeTorpilles[l].carTorpille; // Alors a la positon [j][i] de la matrice on met le caractère de la torpille
                }
                 if (j == vaisseauJoueur.posX && i == vaisseauJoueur.posY) // Si j vaut la position X du vaisseau et i vaut la position Y du vaisseau
                 {
                     if(vaisseauJoueur.isAlive == true) // Si le vaisseau est en vie
                     tableau[j][i] = vaisseauJoueur.carVaisseau; // Alors a la positon [j][i] de la matrice on met le caractère du vaisseau

                 }
                 if((j == Missile.posX && i == Missile.posY)) // Si j vaut la position X du missile et i vaut la position Y du mssile
                 {
                      if(Missile.isAlive == true) // Si le missile est en vie
                      tableau[j][i] = Missile.carMissile; // Alors a la positon [j][i] de la matrice on met le caractère du missile

                  }
                }
            }
        }


    // Création des coins
    tableau[0][0] = coinCadre;
    tableau[nColonnes - 1][0] = coinCadre;
    tableau[0][nLignes - 1] = coinCadre;
    tableau[nColonnes - 1][nLignes - 1] = coinCadre;

    // Affichage du reste du tableau
    for (unsigned i = 0; i < nLignes; ++i) {
        for (unsigned j = 0; j < nColonnes; ++j) {
            cout << tableau[j][i];
        }
        cout << "\n";
    }
}

/*!
 * \brief The tabHighScore struct
 * \struct tabHighScore
 */
struct tabHighScore {
    string nomJoueur; //nom du joueur
    unsigned score; //score du joueur
};

/*!
 * \fn isHigher
 * \brief Teste si l'argument 2 est supérieur ou égal à l'argument 1
 * \param joueur1 - struct du joueur 1
 * \param joueur2 - struct du joueur 2
 * \return bool
 */
bool isHigher (tabHighScore joueur1, tabHighScore joueur2)
{
    return joueur1.score <= joueur2.score;
}

/*!
 * \fn ecritureDansFichier
 * \brief Ecrit sur une ligne deux paramètres séparés par une virgule
 * \param nomJoueur - nom du joueur
 * \param score - son score
 */
void ecritureDansFichier(string nomJoueur, unsigned score)
{
   ofstream fichier("../SpaceInvaders1/score.txt",ios::out | ios::app);
   if(fichier)
   {
       fichier << score << "," << nomJoueur << endl;
   }
   fichier.close();
}

/*!
 * \fn clearFichier
 * \brief Supprime tous les contenus du fichier .txt
 */
void clearFichier ()
{
    ofstream ofs;
    ofs.open("../SpaceInvaders1/score.txt",ofstream::out | ofstream::trunc); //Ouvrir un fichier en mode "tronquer" supprime tous ses contenus
    ofs.close(); //On ferme le fichier
}

/*!
 * \fn triHighScore
 * \brief Trie les high scores au sein du .txt, puit les réecrit dans l'ordre
 */
void triHighScore()
{
   ifstream fichier("../SpaceInvaders1/score.txt", ios::in);  // on ouvre en lecture

   if(fichier)  // Si l'ouverture a fonctionné
   {
       string ligne; //Ligne lue
       unsigned score; //Pour récupérer le score et le comparer aux autres
       string nomJoueur; //Pour récuperer le nom du joueur
       vector<tabHighScore> tabScores; //Pour répertorier l'ensemble des struct tabHighScores
       unsigned i = 0; //Compteur de boucle
       while(getline(fichier, ligne))  // Tant que l'on peut mettre la ligne dans "contenu"
       {
           while(ligne[i] != ',') //La virgule est le séparateur de données dans le fichier .txt
           {
               score = score + (ligne[i]); //Ajout du caractère au high score
               ++i; //Incrémentation du compteur boucle
           }
           for (unsigned j = i+1; j < ligne.size(); ++i) //On lit le reste de la ligne à partir de la virgule
           {
               nomJoueur = nomJoueur + ligne[j]; //Ajout du caractère au nom du joueur
           }
           //On affecte le nom et son score à un struct, à l'indice i de tabScores
           tabScores[i].nomJoueur = nomJoueur;
           tabScores[i].score = unsigned(score);
           //On réinitialise les variables pour le prochain tour de boucle
           score = 0;
           nomJoueur.clear();
       }
       sort(tabScores.begin(),tabScores.end(),isHigher); //Tri du vecteur tabScores
       clearFichier(); //On supprime les contenus du fichier avant de les réecrire dans l'ordre
       fichier.close(); //On ferme le fichier avant de le rouvrir en mode écriture
       for (unsigned i = 0; i < tabScores.size(); ++i) //Pour chaque high score dans le tableau...
       {
           ecritureDansFichier(tabScores[i].nomJoueur, tabScores[i].score); //On l'écrit sur une ligne
       }
   }
   else //Si on a pas pu ouvrir le fichier
   {
       cerr << "Impossible d'ouvrir le fichier !" << endl; //On le signale
   }
}

/*!
 * \fn affichageHighScore
 * \brief Affiche et formate correctement les high scores dans le fichier score.txt
 */
void affichageHighScore()
{
    cout << "salut" << endl;

    ifstream fichier("../SpaceInvaders1/score.txt", ios::in); //On ouvre le fichier en lecture

    string ligne; //Ligne lue
    string score; //Pour récupérer le score
    string nomJoueur; //Pour récupérer le nom du joueur
    unsigned i = 0; //Compteur de boucle
    unsigned classement = 1; //Indice du classement du joueur actuel
    while (getline(fichier,ligne) && classement <= 10) //On opère pour les 10 premières lignes, ou tant qu'il y en a
    {
        while(ligne[i] != ',') //La virgule est le séparateur de données dans le fichier .txt
        {
            score = score + ligne[i]; //Ajout du caractère au high score
            ++i; //Incrémentation du compteur boucle
        }
        for (unsigned j = i+1; j < ligne.size(); ++i) //On lit le reste de la ligne à partir de la virgule
        {
            nomJoueur = nomJoueur + ligne[j]; //Ajout du caractère au nom du joueur
        }
        cout << classement << ". " << nomJoueur << " ....... " << score << endl; //On formate l'affichage du classement, du nom et du score associés
        //On réinitialise les variables pour le prochain tour de boucle
        nomJoueur.clear();
        score.clear();
        ++classement; //On incrémente le classement
    }
    fichier.close();
}

/*!
 * \fn main
 * \brief Jeu assemblé en entier
 */
int main()
{
    afficherTableau(); // On affiche le tableau de base
    unsigned invadersVivants = 0; // Nombre d'invader en vie
    bool gameOver = false; // On dit que nous n'avons pas perdu
    while(gameOver == false) // Tant que nous avons pas perdu :
    {
        manageHero(); // On gère les déplacement du vaisseau
        manageInvaders(); // Les invaders se déplace
        shootMissile(); // On check si on doit lancer un missile
        shootTorpille(); // On check si les invaders doivent lancer une torpille
        manageTorpille(); // On gère les torpilles
        afficherTableau(); // On re affiche le tableau
        cout << endl << "HP - " << vaisseauJoueur.pv << endl; // On affiche les pv du vaisseau
        cout << endl << "SCORE - " << score << endl; // On affiche le score

        reset_input_mode(); // On remet en mode canonique
        if (vaisseauJoueur.pv <= 0) // Si on a plus de point de vie
            gameOver = true; // Alors on perd
        for (unsigned i = 0; i < listeEnnemi.size(); ++i) // On parcourt les invadersq
        {
            if (listeEnnemi[i].posY == vaisseauJoueur.posY && listeEnnemi[i].isAlive == true) // Si les invaders arivent sur la même ligne que le vaisseau et qu'ils sont en vie
            {
                gameOver = true; // Alors on perd
            }
            if (listeEnnemi[i].isAlive == true){ // Si l'invader est en vie
                 ++invadersVivants; // Alors on rajoute 1 au nombre d'invader en vie
            }
            if (invadersVivants == 0){ // Si il y a 0 invader
                gameOver = true; // Alors on perds
            }        
        }
        cout << endl << "Nombre : " << invadersVivants << endl;
        invadersVivants = 0; // On remet à 0 le nombre d'invaders viants
    }
    ClearScreen(); // On nettoie
    cout << "GAME OVER" << endl << endl; // On affiche game over
    cout << "Entre ton nom: "; // On demande d'entrer notre nom
    string nomJoueur; // On créer une variable de nom
    getline(cin,nomJoueur);
    cout << nomJoueur << "! Ton score est de " << score <<" point(s). Bravo!" << endl << endl; // On affiche le score et le nom
    ecritureDansFichier(nomJoueur,score); // On écrit dans un fichier le classement
    triHighScore(); // On trie du meilleur au pire socre
    cout << "Voici le top 10 des meilleurs scores :" << endl << endl; // On affiche le top 10
    affichageHighScore();
}
