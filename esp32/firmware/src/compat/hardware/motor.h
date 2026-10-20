/* La T-HMI n'a pas de moteur de vibration : le retour haptique de l'app ne fait rien. */
#ifndef _MOTOR_H
    #define _MOTOR_H
    #include <stdint.h>
    static inline void motor_vibe( int32_t duration, bool enforced = false ) { (void)duration; (void)enforced; }
#endif
