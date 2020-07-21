#include <stddef.h>
#include <vitasdk.h>
#include <taihen.h>
#include <psp2/motion.h> 
#include <libk/string.h>
#include <stdlib.h>
#include <taipool.h>

#include "main.h"
#include "profile.h"
#include "ui.h"
#include "common.h"
#include "remap.h"

uint8_t used_funcs[HOOKS_NUM];
char titleid[16];
uint16_t TOUCH_SIZE[4] = {
	1920, 1088,	//Front
	1919, 890	//Rear
};
const uint32_t btns[PHYS_BUTTONS_NUM] = {
	SCE_CTRL_CROSS, SCE_CTRL_CIRCLE, SCE_CTRL_TRIANGLE, SCE_CTRL_SQUARE,
	SCE_CTRL_START, SCE_CTRL_SELECT, SCE_CTRL_LTRIGGER, SCE_CTRL_RTRIGGER,
	SCE_CTRL_UP, SCE_CTRL_RIGHT, SCE_CTRL_LEFT, SCE_CTRL_DOWN, SCE_CTRL_L1,
	SCE_CTRL_R1, SCE_CTRL_L3, SCE_CTRL_R3
};
int model;
uint8_t internal_touch_call = 0;
uint8_t internal_ext_call = 0;

static uint64_t startTick;
static uint8_t delayedStartDone = 0;

static uint8_t current_hook = 0;
static SceUID hooks[HOOKS_NUM];
static tai_hook_ref_t refs[HOOKS_NUM];

void delayedStart(){
	delayedStartDone = 1;
	// Enabling analogs sampling 
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
	sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG_WIDE);
	// Enabling gyro sampling
	sceMotionReset();
	sceMotionStartSampling();
	if (gyro_options[6] == 1) sceMotionSetDeadband(1);
	else if (gyro_options[6] == 2) sceMotionSetDeadband(0);
	//ToDo decide on sceMotionSetTiltCorrection usage
	//if (gyro_options[7] == 1) sceMotionSetTiltCorrection(0); 
	
	// Enabling both touch panels sampling
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
	
	// Detecting touch panels size
	SceTouchPanelInfo pi;	
	int ret = sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &pi);
	if (ret >= 0){
		TOUCH_SIZE[0] = pi.maxAaX;
		TOUCH_SIZE[1] = pi.maxAaY;
	}
	ret = sceTouchGetPanelInfo(SCE_TOUCH_PORT_BACK, &pi);
	if (ret >= 0){
		TOUCH_SIZE[2] = pi.maxAaX;
		TOUCH_SIZE[3] = pi.maxAaY;
	}
}

void checkForDelayedStart(){
	//Activate delayed start
	if (!delayedStartDone 
		&& startTick + settings_options[3] * 1000000 < sceKernelGetProcessTimeWide()){
		delayedStart();
	}
}

// Simplified generic hooking function
void hookFunction(uint32_t nid, const void *func) {
	hooks[current_hook] = taiHookFunctionImport(&refs[current_hook],TAI_MAIN_MODULE,TAI_ANY_LIBRARY,nid,func);
	current_hook++;
}

int sceCtrlPeekBufferPositive_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[0], port, ctrl, count);
	patchToExt(&ctrl[ret - 1]);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 0, POSITIVE);
	used_funcs[0] = 1;
	return ret;
}

int sceCtrlPeekBufferPositive2_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[1], port, ctrl, count);
	patchToExt(&ctrl[ret - 1]);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 1, POSITIVE);
	used_funcs[1] = 1;
	return ret;
}

int sceCtrlReadBufferPositive_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[2], port, ctrl, count);
	patchToExt(&ctrl[ret - 1]);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 2, POSITIVE);
	used_funcs[2] = 1;
	return ret;
}

int sceCtrlReadBufferPositive2_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[3], port, ctrl, count);
	patchToExt(&ctrl[ret - 1]);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 3, POSITIVE);
	used_funcs[3] = 1;
	return ret;
}

int sceCtrlPeekBufferPositiveExt_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[4], port, ctrl, count);
	patchToExt(&ctrl[ret - 1]);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 4, POSITIVE);
	used_funcs[4] = 1;
	return ret;
}

int sceCtrlPeekBufferPositiveExt2_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[5], port, ctrl, count);
	if (internal_ext_call) return ret;
	checkForDelayedStart();
	ret = remap(ctrl, ret, 5, POSITIVE);
	used_funcs[5] = 1;
	return ret;
}

int sceCtrlReadBufferPositiveExt_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[6], port, ctrl, count);
	patchToExt(&ctrl[ret - 1]);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 6, POSITIVE);
	used_funcs[6] = 1;
	return ret;
}

int sceCtrlReadBufferPositiveExt2_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[7], port, ctrl, count);
	if (internal_ext_call) return ret;
	checkForDelayedStart();
	ret = remap(ctrl, ret, 7, POSITIVE);
	used_funcs[7] = 1;
	return ret;
}

int sceCtrlPeekBufferNegative_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[8], port, ctrl, count);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 8, NEGATIVE);
	used_funcs[8] = 1;
	return ret;
}

int sceCtrlPeekBufferNegative2_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[9], port, ctrl, count);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 9, NEGATIVE);
	used_funcs[9] = 1;
	return ret;
}

int sceCtrlReadBufferNegative_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[10], port, ctrl, count);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 10, NEGATIVE);
	used_funcs[10] = 1;
	return ret;
}

int sceCtrlReadBufferNegative2_patched(int port, SceCtrlData *ctrl, int count) {
	int ret = TAI_CONTINUE(int, refs[11], port, ctrl, count);
	checkForDelayedStart();
	ret = remap(ctrl, ret, 11, NEGATIVE);
	used_funcs[11] = 1;
	return ret;
}

int sceTouchRead_patched(SceUInt32 port, SceTouchData *pData, SceUInt32 nBufs) {
	int ret = TAI_CONTINUE(int, refs[12], port, pData, nBufs);
	used_funcs[12] = 1;
	return retouch(port, pData, ret, 0);
}

int sceTouchRead2_patched(SceUInt32 port, SceTouchData *pData, SceUInt32 nBufs) {
	int ret = TAI_CONTINUE(int, refs[13], port, pData, nBufs);
	used_funcs[13] = 1;
	return retouch(port, pData, ret, 1);
}

int sceTouchPeek_patched(SceUInt32 port, SceTouchData *pData, SceUInt32 nBufs) {
	int ret = TAI_CONTINUE(int, refs[14], port, pData, nBufs);
	used_funcs[14] = 1;
	return retouch(port, pData, ret, 2);
}

int sceTouchPeek2_patched(SceUInt32 port, SceTouchData *pData, SceUInt32 nBufs) {
	int ret = TAI_CONTINUE(int, refs[15], port, pData, nBufs);
	used_funcs[15] = 1;
	return retouch(port, pData, ret, 3);
}

int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, int sync) {
	ui_draw(pParam);
	used_funcs[16] = 1;
	return TAI_CONTINUE(int, refs[16], pParam, sync);
}	

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {
	
	// Getting game Title ID
	sceAppMgrAppParamGetString(0, 12, titleid , 256);
	
	// For some reason, some system Apps are refusing to start 
	// if this plugin is active; so stop the
	// initialization of the module.
	if(!strcmp(titleid, "") ||
		(strcmp(titleid, "NPXS10028") && //not Adrenaline
			strcmp(titleid, "NPXS10013") && //not PS4Link
			strstr(titleid, "NPXS")))	 //System app
		return SCE_KERNEL_START_SUCCESS;
	
	//Set current tick for delayed startup calculation
	startTick = sceKernelGetProcessTimeWide();
	
	// Setup stuffs
	loadSettings();
	loadGlobalConfig();
	loadGameConfig();
	model = sceKernelGetModel();
	
	// Initializing used funcs table
	for (int i = 0; i < HOOKS_NUM; i++) {
		used_funcs[i] = 0;
	}
	
	// Initializing taipool mempool for dynamic memory managing
	taipool_init(1024 + 1 * (
		sizeof(SceCtrlData) * (HOOKS_NUM-5) * BUFFERS_NUM + 
		2 * sizeof(SceTouchData) * 4 * BUFFERS_NUM));
	remapInit();
	
	// Hooking functions
	hookFunction(0xA9C3CED6, sceCtrlPeekBufferPositive_patched);
	hookFunction(0x15F81E8C, sceCtrlPeekBufferPositive2_patched);
	hookFunction(0x67E7AB83, sceCtrlReadBufferPositive_patched);
	hookFunction(0xC4226A3E, sceCtrlReadBufferPositive2_patched);
	hookFunction(0xA59454D3, sceCtrlPeekBufferPositiveExt_patched);
	hookFunction(0x860BF292, sceCtrlPeekBufferPositiveExt2_patched);
	hookFunction(0xE2D99296, sceCtrlReadBufferPositiveExt_patched);
	hookFunction(0xA7178860, sceCtrlReadBufferPositiveExt2_patched);
	hookFunction(0x104ED1A7, sceCtrlPeekBufferNegative_patched);
	hookFunction(0x81A89660, sceCtrlPeekBufferNegative2_patched);
	hookFunction(0x15F96FB0, sceCtrlReadBufferNegative_patched);
	hookFunction(0x27A0C5FB, sceCtrlReadBufferNegative2_patched);
	if(!strcmp(titleid, "NPXS10013")) //PS4Link
		return SCE_KERNEL_START_SUCCESS;
	hookFunction(0x169A1D58, sceTouchRead_patched);
	hookFunction(0x39401BEA, sceTouchRead2_patched);
	hookFunction(0xFF082DF0, sceTouchPeek_patched);
	hookFunction(0x3AD3D0A1, sceTouchPeek2_patched);
	
	// For some reason, some Apps are refusing to start 
	// with framebuffer hooked; so skip hooking it
	if(!strcmp(titleid, "NPXS10028") || //Adrenaline
			!strcmp(titleid, "NPXS10013") || //PS4Link
			strstr(titleid, "PSPEMU"))	//ABM
		return SCE_KERNEL_START_SUCCESS;
	
	hookFunction(0x7A410B64, sceDisplaySetFrameBuf_patched);
	
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

	// Freeing hooks
	while (current_hook-- > 0) {
		taiHookRelease(hooks[current_hook], refs[current_hook]);
	}
	
    taipool_term();
		
	return SCE_KERNEL_STOP_SUCCESS;
	
}