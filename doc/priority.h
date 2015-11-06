/*
 * Matemat controller
 * Process priority list
 */

/*
 * Priority levels:
 * 0 = Application
 * 1 = System
 * 2 = I/O
 * 3 = Interrupt
*/
#define PRIORITY_LEVELS 4
#define PRIORITY_APP 0
#define PRIORITY_SYS 1
#define PRIORITY_IO 2
#define PRIORITY_INT 3

/* Queue lengths */
#define QLEN_LEVEL0 16
#define QLEN_LEVEL1 4
#define QLEN_LEVEL2 4
#define QLEN_LEVEL3 4

/* Main process */
#define MAIN_PRIORITY PRIORITY_SYS
#define MAIN_QLEN QLEN_LEVEL1

/* LED driver */
#define LED_PRIORITY PRIORITY_IO
#define LED_QLEN QLEN_LEVEL2

