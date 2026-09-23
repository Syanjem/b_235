#ifndef _MOTOR_CTRL_H_
#define _MOTOR_CTRL_H_

#include "BSP.h"

#include "Angle_Feedback.h"
#include "Idq_Feedback.h"
#include "pid_3loops.h"
#include "svpwm.h"

typedef enum
{
    STATE_MODE_STANDBY,		// 待机模式
    STATE_MODE_WORKING,	  	// 工作模式
	STATE_MODE_STOPPED,		// 停机模式
	STATE_MODE_CALIBRATING,	// 校准模式
} STATE_MODE;
#define STATE_MODE_SWITCH_OFF	0u
#define STATE_MODE_SWITCH_ON	1u
/* 状态切换请求: 中断层置位, 主循环消费 */
typedef struct {
    uint8_t     switch_request;   // 0=无请求, 1=有请求  (原 STATE_MODE_SWITCH_FLAG)
    STATE_MODE  target_mode;      // 目标状态            (原 STATE_MODE_FLAG)
} STATE_REQUEST;

/* 故障码 (恢复被注释的 FAULT_STATE) */
typedef enum {
    FAULT_NONE = 0,             // 无故障
    FAULT_OVER_CURRENT,         // 过流
    FAULT_OVER_VOLTAGE,         // 过压
    FAULT_UNDER_VOLTAGE,        // 欠压
    FAULT_OVER_TEMPERATURE,     // 过温
    FAULT_OVER_SPEED,           // 超速
    FAULT_START_FAIL,           // 启动失败 (原 iq16_up 触发的场景)
} FAULT_STATE;

/* 子模式 启动方式 */
typedef enum
{
    START_MODE_POWERUP,     /* 直接运行: 上电即启动FOC闭环(PWM立即输出)           */
    START_MODE_ADC,  		/* 电流检测启动: 三相短路接地, 等ADC检测到外力转动电流 */
    START_MODE_CAN,     	/* CAN信号启动: 等上位机CAN命令(0x101)触发can_ok      */
    START_MODE_EXTI,     	/* GPIO外部中断启动: 等PB8/PB9下降沿触发(预留)         */
} SUB_MODE_START;

/* 子模式 工作模式 */
typedef enum
{
    CONTROL_MODE_I          = 0,  /* 力矩控制: Iq闭环, target_iq直接给定                */
    CONTROL_MODE_SPEED      = 1,  /* 速度控制: 速度环→target_iq→电流环, target_speed给定 */
    CONTROL_MODE_SPEED_RAMP = 2,  /* 速度梯度: 速度斜坡规划+前馈(平滑加减速)            */
} SUB_MODE_CONTROL;

/* 3.停机，子模式 */
//typedef enum
//{
//} STOPPED_SUB_MODE

/* 4.校准，子模式 */
//typedef enum
//{
//} CALIBRATING_SUB_MODE

/* 5.调试，子模式 */
//typedef enum
//{
//} DEBUG_SUB_MODE


/* 状态层 */
typedef struct {
    STATE_MODE        mode;            // 当前状态      (原 stateMode)
    STATE_REQUEST     request;         // 切换请求      (原 stateFlag 的部分)
//    FAULT_STATE       fault;           // 故障码 (新增)
    SUB_MODE_START    start_mode;      // 待机子模式    (原 standby_subMode_start)
    SUB_MODE_CONTROL  control_mode;    // 工作子模式    (原 working_subMode_control)
} MOTOR_STATE;

typedef struct
{
    Angle_Struct*   p_angle;    /* 角度反馈: 编码器机械角→电角度→速度, 由SPI0读取MT6816  */
    Idq_Struct*     p_idq;      /* 电流反馈: ADC→ia/ib/ic→Clarke→Park→id/iq, 含母线电压  */
    V_Struct*       p_v;        /* 电压输出: vd/vq→逆Park→v_alpha/v_beta→调制比k        */
    ABCpwm_Struct*  p_abcpwm;   /* PWM输出: duty_a/b/c (0~period), 写TIMER0 CCR寄存器     */
} MOTOR_COMPONENTS;

typedef struct {
    Pi_Data_Struct*   p_data;        // 运行时数据 (原 p_pidata)
    Pi_Para_Struct*   p_mangle;      // 位置环     (原 pi1_mangle_para)
    Pi_Para_Struct*   p_speed;       // 速度环     (原 pi2_speed_para)
    Pi_Para_Struct*   p_iq;          // q轴电流环  (原 pi3_iq_para)
    Pi_Para_Struct*   p_id;          // d轴电流环  (原 pi3_id_para)
} MOTOR_PI;


#define ATK_OFF		0u
#define ATK_ON		1u
/* 调试/功能开关 (原 STATE_FLAG 里的 ATK_FLAG 拆出来) */
typedef struct {
    uint8_t     atk_enable;        // ATK 调试发送开关
    // 后续可加: debug_enable, log_enable 等
} MOTOR_SWITCH;

/* 运行时数据 (原散落在 Foc.c 的 static 变量) */
typedef struct {
    uint32_t    stop_delay_num;        // 运行周期计数 (原 p_num)
    uint16_t     iq16_up;             // 过流计数 (原 iq16_up)
    uint16_t     adc_trig_cnt;      // ADC 触发去抖计数
} MOTOR_RUNTIME;

/* 顶层 */
typedef struct {
    MOTOR_STATE       state;
    MOTOR_COMPONENTS  components;
    MOTOR_PI          pi;
    MOTOR_RUNTIME      runtime;       // 新增
    MOTOR_SWITCH       sw;            // 新增
} MOTOR_DATA;
extern MOTOR_DATA motorData;



void motor_state_mode_config_task(void);
void motor_standby_config(void);
void motor_working_config(void);
void motor_stopped_config(void);






























//typedef enum
//{
//    SUB_STATE_IDLE          = 0,  /* 空闲: 未在校准中                                 */
//    CURRENT_CALIBRATING,         /* 电流偏置校准: 采零电流ADC值求平均, 获取offset      */
//    RSLS_CALIBRATING,            /* 电阻/电感/编码器校准: R/L参数辨识 + 编码器零点      */
//    FLUX_CALIBRATING,            /* 磁链校准: 施加电压测反电势, 计算磁链系数            */
//} SUB_STATE;

//typedef enum
//{
//    CS_STATE_IDLE = 0,           /* 空闲                                                */
//    CS_MOTOR_R_START,            /* 电阻辨识: 开始, 施加d轴电压                          */
//    CS_MOTOR_R_LOOP,             /* 电阻辨识: 稳态后采样id/vd, 计算 R=vd/id              */
//    CS_MOTOR_R_END,              /* 电阻辨识: 结束, 保存R值                              */
//    CS_MOTOR_L_START,            /* 电感辨识: 开始, 施加d轴电压阶跃                      */
//    CS_MOTOR_L_LOOP,             /* 电感辨识: 采样id瞬态响应, 拟合L值                     */
//    CS_MOTOR_L_END,              /* 电感辨识: 结束, 保存L值                              */
//    CS_DIR_PP_START,             /* 方向/极对数辨识: 开始                                */
//    CS_DIR_PP_LOOP,              /* 方向/极对数辨识: 施加q轴电压, 观察转向                */
//    CS_DIR_PP_END,               /* 方向/极对数辨识: 结束, 确定编码器方向(±1)和极对数p    */
//    CS_ENCODER_START,            /* 编码器零点校准: 开始, 拉电角度到0                    */
//    CS_ENCODER_CW_LOOP,          /* 编码器零点校准: 正转稳定                             */
//    CS_ENCODER_CCW_LOOP,         /* 编码器零点校准: 反转稳定                              */
//    CS_ENCODER_END,              /* 编码器零点校准: 结束, 读机械角作为angle_zero         */
//    CS_REPORT_OFFSET_LUT,        /* 输出: 上报offset查表/保存到Flash                     */
//} CS_STATE;



#endif
