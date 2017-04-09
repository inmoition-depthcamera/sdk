
#ifndef __SDK_PROTOCOL_H__
#define __SDK_PROTOCOL_H__

#include "inttypes.h"

#pragma pack(1)

typedef struct
{
	uint8_t deviceAddr;
	uint8_t functionCode;
	uint16_t startAddr;
	uint32_t len;
}SdkProtocolHeader;

typedef struct
{
	uint16_t ultrasound[8];
	uint8_t dropSendor;
	uint16_t irSensor;
	uint8_t collisionSensor;
	uint32_t leftEncoderPos;                  //当前左边里程计的积分位置
	uint32_t rightEcoderPos;                  //当前右边里程计的积分位置
	uint32_t lineVelocity;
	uint32_t anglarVelocity;
	uint8_t chargeStatus;
	uint8_t batteryStatus;
	uint16_t errorState;
}ChassisStatusRegister;

typedef struct
{
	uint32_t lineVelocity;
	uint32_t angularCelocity;
}ChassisControlRegister;

typedef struct
{
	//超声波传感器
	uint16_t ultrasoundPos[8];
	uint16_t ultrasoundPerspective[8];
	//防跌落传感器
	uint16_t dropSensorPos[8];
	uint16_t dropSensorPerspective[8];
	//红外传感器
	uint16_t irSensoPos[16];
	uint16_t irSensoPerspective[16];
	//碰撞传感器
	uint16_t collisionSensorAngle[16];
	uint16_t collisionSensorPerspective[16];
}ChassisParamRegister;

typedef struct
{
	uint32_t vectorTargetDistance;
	uint32_t vectorTargetAngle;
	float relativeTargetPosX;
	float relativeTargetPosY;
	uint32_t setLineVelocity;
	uint32_t setAngularVelocity;
	uint8_t backuint8_tge;
	uint8_t backuint8_tgeThreshold;
	uint32_t setBackuint8_tgePosX;
	uint32_t setBackuint8_tgePosY;
	uint8_t startMapping;
	uint8_t stopMapping;
	uint8_t setDefaultMap;
	uint8_t emergencyStop;
}AlgControlRegister;

typedef struct
{
	uint8_t workMode;
	uint32_t lineVelocity;
	uint32_t angularVelocity;
	uint32_t posX;
	uint32_t poxY;
	uint32_t posSita;
	uint16_t errorState;
}AlgStatusRegister;

#pragma pack()

// Device Address
#define ALG_ADDRESS 0x11
#define MCU_ADDRESS 0x12

// Function Code

//      ALG
#define ALG_STATUS_REG         0x01
#define ALG_CONTROL_REG_READ   0x02
#define ALG_CONTROL_REG        0x03
#define ALG_TARGET_POINTS_READ 0x04
#define ALG_TARGET_POINTS      0x05
#define ALG_PLANED_PATH        0x06
#define ALG_LIDAR_RAW_DATA     0x07
#define ALG_LIDAR_MAP          0x08
#define ALG_ULTROSON_MAP       0x09
#define ALG_SWITCH_MAP         0x0A
#define ALG_TOTAL_MAP          0x0B

// ALG_CONTROL_REG ADDRESS
/*
case  0: app->OnRxNewControlCommand(CID_TARGET_RUNNER, data, len); break;//SetVectorTargetDistance
case 16: app->OnRxNewControlCommand(CID_SPEED_CONTROL, data, len); break;//SetLineVolocity
case 24: app->OnRxNewControlCommand(CID_BACK_CHARGE, data, len); break;//BackCharge
case 34: app->OnRxNewControlCommand(CID_MAP_BUILDER, data, len); break;//StartMapping
case 37: app->OnRxNewControlCommand(CID_IDLE, data, len); break;//EmergencyStop
*/
#define ALG_CR_ADDR_SET_TARGET    0
#define ALG_CR_ADDR_SPEED_CONTROL 16
#define ALG_CR_ADDR_BACK_CHARGE   24
#define ALG_CR_ADDR_MAP_BUILDER   34
#define ALG_CR_ADDR_IDLE          37


//      MCU
#define MCU_CONTROL_REG_READ   0x01
#define MCU_CONTROL_REG        0x02
#define MCU_STATUS_REG         0x03
#define MCU_PARAM_REG          0x04

#endif