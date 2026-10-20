/*
 * Sur la montre, l'ecran s'eteint et revient a la tuile principale apres un delai ; l'app demande
 * de bloquer ce retour pendant un quiz. Ici il n'y a pas de tuile principale, donc rien a bloquer.
 */
#ifndef _DISPLAY_H
    #define _DISPLAY_H
    static inline bool display_get_block_return_maintile( void ) { return( false ); }
    static inline void display_set_block_return_maintile( bool block ) { (void)block; }
#endif
