
#ifndef OWI_SLAVE_H_
#define OWI_SLAVE_H_

#define	OWI_IDLE			0
#define	OWI_WF1				1
#define	OWI_RESET_WF1		2
#define	OWI_WF_PRESENCE0	3
#define	OWI_WF_PRESENCE1	4

#define	OWI_ST_CMD			0
#define	OWI_ST_LISTEN		1
#define	OWI_ST_ANSWER		2

void owi_init(void);
uint8_t temp[2];

#endif /* OWI_SLAVE_H_ */
