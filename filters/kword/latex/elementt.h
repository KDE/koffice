/* 
 * @memo Projet d'Infographie : Fichier en-t�te de la classe Groupe.
 * 
 * @name groupe.hxx
 * Ce fichier contient les d�clarations de la classe Groupe d�velopp�e
 * dans le cadre d'un projet d'infographie de 3e ann�e.\\
 * Consulter la documentaion relative � Groupe.cxx pour obtenir de plus amples
 * informations sur l'impl�mentation.
 * @author Robert Jacolin
 * @version 0.0.1 \\
 * CREATION     : 15/12/2000 \\
 * MODIFICATION :                                      
 * @see Ellipse.hxx Droite.hxx Hyperbole.hxx Cercle.hxx Figure.hxx
 */

#ifndef __ELEMENTT_H__
#define __ELEMENTT_H__

#include "textzone.h"

/**
* La classe Groupe permet de manipuler un ensemble de figures quelconques.
* Pour cela, il fournit un classe pour g�rer un liste de figure (classe 
* Groupe) ainsi qu'un it�rateur (Groupe_Iterator).
* La liste de figures peut etre vide. 
* @memo Liste de figures, parcourt d'une liste de figures
*/
class ElementT
{
	TextZone*  _texte;         /* Pointeur sur l'element de donnee */
	ElementT* _next;           /* Pointeur sur l'element suivant.  */

	private:

	protected:
		
	public:
/**
 * @name Constructeurs
 */
//@{
	/// Constructeur par d�faut
	ElementT() { 
		_texte = 0;
		_next  = 0;
	}
	/// Constructeur par recopie
	ElementT(ElementT * eltt) {
		_texte = eltt->get_texte();
		_next  = eltt->get_next();
	}
//@}

/**
 * @name Destructeurs
 */
//@{
	  /// Destructeur
	virtual ~ElementT() {
		rem_texte();
		rem_next();
	}
//@}

/**
 * @name Accesseurs
 */
//@{
	TextZone* get_texte () const { return _texte; }
	ElementT* get_next  () const { return _next;  }
//@}

/**
 * @name Modifieurs
 */
//@{
	void set_texte (const TextZone*);        /* Ajoute une figure � la fin */
	void rem_texte ();                       /* Enleve la derniere ou      */
	void set_next  (ElementT*);              /* la premiere figure.        */
	void rem_next  ();
//@}

/**
 * @name Operateurs
 */
//@{
	/// Operateur d'affectation
	ElementT& operator = (const ElementT &);
 //@}
/**
 * ^name Fonctions de delegation
 */
//@{

//@}
};

#endif /* __ELEMENTT_H__ */

