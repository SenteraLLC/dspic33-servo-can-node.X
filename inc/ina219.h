

#ifndef INA219_H
#define	INA219_H


// Function Prototypes =========================================================

void INA219Init ( void );
void INA219Service ( void );
uint16_t INA219AmpGet ( void );
uint16_t INA219VoltGet ( void );

#endif	/* INA219_H */

