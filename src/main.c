#include "sapi.h"              // <= sAPI header
#include "../../TPFSM/inc/TPFSM.h"
#include "board.h"

static void initHardware(void);

static void initHardware(void) {
	/*	Board_Init();*/
	boardConfig();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
}

/*MAQUINA DE ESTADOS*/
typedef struct fsm_s {
	uint8_t state_last;
	uint8_t state;
	uint8_t state_next;
} fsm_t;

typedef void (*fsm_state_function_t)(fsm_t *fsm);

enum {
	ESTADO_OFF = 1, ESTADO_ON, ESTADO_BLINKING, ESTADO_ERROR
};

/*PROTOTIPO DE FUNCIONES MAQUINA DE ESTADO*/
void FSM_Init(fsm_t *fsm, uint8_t state_init);
void FSM_NextState_Set(fsm_t *fsm, uint8_t state_next);
void FSM_Run(fsm_t *fsm, fsm_state_function_t Entry, fsm_state_function_t Loop,
		fsm_state_function_t Exit);
uint8_t FSM_GetState(fsm_t *fsm);

void Estado_ON_Entry(fsm_t *fsm);
void Estado_ON(fsm_t *fsm);

void Estado_OFF_Entry(fsm_t *fsm);
void Estado_OFF(fsm_t *fsm);

void Estado_Blinking_Entry(fsm_t *fsm);
void Estado_Blinking(fsm_t *fsm);

void Estado_Error(fsm_t *fsm);

void FSM_Init(fsm_t *fsm, uint8_t state_init) {
	fsm->state_last = 0;
	fsm->state = state_init;
	fsm->state_next = 0;
}

/*DEFINICION DE FUNCIONES MAQUINAS DE ESTADO*/
void FSM_NextState_Set(fsm_t *fsm, uint8_t state_next) {
	fsm->state_next = state_next;
}

void FSM_Run(fsm_t *fsm, fsm_state_function_t Entry, fsm_state_function_t Loop,
		fsm_state_function_t Exit) {
	if (fsm->state_last != fsm->state && NULL != Entry) {
		Entry(fsm);
	}

	if (NULL != Loop) {
		Loop(fsm);
	}

	if (fsm->state_next != fsm->state && NULL != Exit) {
		Exit(fsm);
	}
	fsm->state_last = fsm->state;
	fsm->state = fsm->state_next;
}

uint8_t FSM_GetState(fsm_t *fsm) {
	if (gpioRead(TEC1) == 0) {
		if (fsm->state == ESTADO_OFF)
			FSM_NextState_Set(fsm, ESTADO_ON);
		if (fsm->state == ESTADO_ON)
			FSM_NextState_Set(fsm, ESTADO_BLINKING);
		if (fsm->state == ESTADO_BLINKING)
			FSM_NextState_Set(fsm, ESTADO_OFF);
		fsm->state = fsm->state_next;
	}
	return fsm->state;
}

void Estado_ON_Entry(fsm_t *fsm) {

	gpioWrite(LEDB, ON);
}

void Estado_ON(fsm_t *fsm) {
	FSM_NextState_Set(fsm, ESTADO_ON);
}

void Estado_OFF_Entry(fsm_t *fsm) {
	gpioWrite(LEDG, OFF);
	gpioWrite(LEDB, OFF);
}

void Estado_OFF(fsm_t *fsm) {
	FSM_NextState_Set(fsm, ESTADO_OFF);
}

void Estado_Blinking_Entry(fsm_t *fsm) {
	gpioWrite(LEDB, OFF);
	gpioWrite(LEDG, ON);
}

void Estado_Blinking(fsm_t *fsm) {
	FSM_NextState_Set(fsm, ESTADO_BLINKING);
}

void Estado_Error(fsm_t *fsm) {

}

int main(void) {

	initHardware();
	fsm_t fsm;
	FSM_Init(&fsm, ESTADO_OFF);

	while (1) {
		switch (FSM_GetState(&fsm)) {
		case ESTADO_OFF:
			FSM_Run(&fsm, Estado_OFF_Entry, Estado_OFF, NULL);
			break;
		case ESTADO_ON:
			FSM_Run(&fsm, Estado_ON_Entry, Estado_ON, NULL);
			break;
		case ESTADO_BLINKING:
			FSM_Run(&fsm, Estado_Blinking_Entry, Estado_Blinking, NULL);
			break;
		default:
			FSM_Run(&fsm, NULL, Estado_Error, NULL);
			break;
		}

	}
}

/*==================[end of file]============================================*/

