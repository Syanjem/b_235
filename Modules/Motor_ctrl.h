#ifndef _MOTOR_CTRL_H_
#define _MOTOR_CTRL_H_

#include "Angle_Feedback.h"
#include "Idq_Feedback.h"
#include "pid_3loops.h"
#include "svpwm.h"


 
typedef enum
{
    STATE_MODE_RUNNING,		/* 运行态: FOC闭环工作, PWM输出有效, 电流/速度/位置环激活   */
    STATE_MODE_CALIBRATION,	/* 校准态: 电角度零点校准(angleDf_float_get(), 期间不做闭环控制    */
//    STATE_MODE_JianCe,      /* 检测态: 故障检测/参数辨识, 不输出PWM                    */
    STATE_MODE_FAULT,       /* 故障态: 过流/过压/过温/超速触发, 关PWM保护, 等待复位      */
    STATE_MODE_DEBUG,       /* 调试态: 开环强制角度输出, 用于验证PWM/采样/编码器方向     */
	STATE_MODE_STOP,
} STATE_MODE;


typedef enum
{
    FOC_RUNNING_BEGIN_MODE_POWER_UP,       /* 直接运行: 上电即启动FOC闭环(PWM立即输出)           */
    FOC_RUNNING_BEGIN_MODE_ADC_DETECTION,  /* 电流检测启动: 三相短路接地, 等ADC检测到外力转动电流 */
    FOC_RUNNING_BEGIN_MODE_CAN_SIGNAL,     /* CAN信号启动: 等上位机CAN命令(0x101)触发can_ok      */
    FOC_RUNNING_BEGIN_MODE_GPIO_EXTI,      /* GPIO外部中断启动: 等PB8/PB9下降沿触发(预留)         */
} FOC_RUNNING_BEGIN_MODE;


typedef enum
{
    FOC_RUNNING_CONTROL_MODE_I          = 0,  /* 力矩控制: Iq闭环, target_iq直接给定                */
    FOC_RUNNING_CONTROL_MODE_SPEED      = 1,  /* 速度控制: 速度环→target_iq→电流环, target_speed给定 */
    FOC_RUNNING_CONTROL_MODE_SPEED_RAMP = 2,  /* 速度梯度: 速度斜坡规划+前馈(平滑加减速)            */
} FOC_RUNNING_CONTROL_MODE;

typedef enum
{
    FOC_FAULT_STATE_NORMAL			= 0,  /* 正常: 无故障                                     */
    FOC_FAULT_STATE_OVER_CURRENT,         /* 过流: ia/ib/ic 任一相超过 ±1.0pu (±33A)           */
    FOC_FAULT_STATE_OVER_VOLTAGE,         /* 过压: vbus_V ≥ 上限 (如 28V)                      */
    FOC_FAULT_STATE_UNDER_VOLTAGE,        /* 欠压: vbus_V ≤ 下限 (如 18V), 母线跌落           */
    FOC_FAULT_STATE_OVER_TEMPERATURE,     /* 过温: NTC/MB1601B 读数 ≥ 阈值 (如 80°C)           */
    FOC_FAULT_STATE_SPEEDING,             /* 超速: speed 超过 ±1.0pu (额定转速)                */
} FOC_FAULT_STATE;


typedef struct
{
    STATE_MODE          		stateMode;			// 全局状态机: 1运行、2校准、3检测、4故障、5调试
    FOC_RUNNING_BEGIN_MODE      focRunningBeginMode;		// 1运行: foc 启动模式                   
    FOC_RUNNING_CONTROL_MODE    focRunningControlMode;	// 1运行：foc 控制模式
//    SUB_STATE         		 Sub_State;           /* 校准子状态: 保留                                 */
//    CS_STATE          		 Cs_State;           /* 参数辨识步骤: 保留                               */
    FOC_FAULT_STATE				focFaultState;	// 4故障 
} MOTOR_STATE;




/* -------------------------------------------------------------------------- *
 * 4. 校准子状态 SUB_STATE
 *    描述: 在 STATE_MODE_JiaoZhun 态下, 标识当前正在执行的校准步骤
 * -------------------------------------------------------------------------- */
typedef enum
{
    SUB_STATE_IDLE          = 0,  /* 空闲: 未在校准中                                 */
    CURRENT_CALIBRATING,         /* 电流偏置校准: 采零电流ADC值求平均, 获取offset      */
    RSLS_CALIBRATING,            /* 电阻/电感/编码器校准: R/L参数辨识 + 编码器零点      */
    FLUX_CALIBRATING,            /* 磁链校准: 施加电压测反电势, 计算磁链系数            */
} SUB_STATE;


/* -------------------------------------------------------------------------- *
 * 5. 参数辨识步骤 CS_STATE (Calibration Sequence)
 *    描述: RSLS_CALIBRATING 期间的详细步骤状态机
 *    流程: 测R → 测L → 测方向/极对数 → 测编码器零点 → 输出offset
 * -------------------------------------------------------------------------- */
typedef enum
{
    CS_STATE_IDLE = 0,           /* 空闲                                                */
    CS_MOTOR_R_START,            /* 电阻辨识: 开始, 施加d轴电压                          */
    CS_MOTOR_R_LOOP,             /* 电阻辨识: 稳态后采样id/vd, 计算 R=vd/id              */
    CS_MOTOR_R_END,              /* 电阻辨识: 结束, 保存R值                              */
    CS_MOTOR_L_START,            /* 电感辨识: 开始, 施加d轴电压阶跃                      */
    CS_MOTOR_L_LOOP,             /* 电感辨识: 采样id瞬态响应, 拟合L值                     */
    CS_MOTOR_L_END,              /* 电感辨识: 结束, 保存L值                              */
    CS_DIR_PP_START,             /* 方向/极对数辨识: 开始                                */
    CS_DIR_PP_LOOP,              /* 方向/极对数辨识: 施加q轴电压, 观察转向                */
    CS_DIR_PP_END,               /* 方向/极对数辨识: 结束, 确定编码器方向(±1)和极对数p    */
    CS_ENCODER_START,            /* 编码器零点校准: 开始, 拉电角度到0                    */
    CS_ENCODER_CW_LOOP,          /* 编码器零点校准: 正转稳定                             */
    CS_ENCODER_CCW_LOOP,         /* 编码器零点校准: 反转稳定                              */
    CS_ENCODER_END,              /* 编码器零点校准: 结束, 读机械角作为angle_zero         */
    CS_REPORT_OFFSET_LUT,        /* 输出: 上报offset查表/保存到Flash                     */
} CS_STATE;


/* -------------------------------------------------------------------------- *
 * 6. 故障状态 FAULT_STATE
 *    描述: 在 STATE_MODE_Fault 态下, 标识触发故障的具体原因
 *    触发: 比较器/ADC采样/编码器越限 → 置 motorData.state.fault_state
 *    动作: 关PWM(timer_primary_output_config DISABLE) → 等待复位
 * -------------------------------------------------------------------------- */





typedef struct
{
    Angle_Struct*   p_angle;    /* 角度反馈: 编码器机械角→电角度→速度, 由SPI0读取MT6816  */
    Idq_Struct*     p_idq;      /* 电流反馈: ADC→ia/ib/ic→Clarke→Park→id/iq, 含母线电压  */
    V_Struct*       p_v;        /* 电压输出: vd/vq→逆Park→v_alpha/v_beta→调制比k        */
    ABCpwm_Struct*  p_abcpwm;   /* PWM输出: duty_a/b/c (0~period), 写TIMER0 CCR寄存器     */
} MOTOR_COMPIONENTS;


/* -------------------------------------------------------------------------- *
 * 9. PI 控制器参数集合 MOTOR_PI_PARA
 *    描述: 三环级联控制器的参数指针集合 (外→内: 位置→速度→电流)
 *    级联:
 *      位置环(pi1) → target_speed → 速度环(pi2) → target_iq → 电流环(pi3)
 *    注意: pi3_iq/pi3_id 是独立的两个电流环, 分别控制d轴和q轴
 * -------------------------------------------------------------------------- */

typedef struct
{
    Pi_Data_Struct* p_pidata;       /* PI数据: target/feedback/id/iq/vd/vq 等运行时数据      */
    Pi_Para_Struct* pi3_iq_para;    /* q轴电流环: Kp/Ki/integral/限幅, 输出 vq              */
    Pi_Para_Struct* pi3_id_para;    /* d轴电流环: Kp/Ki/integral/限幅, 输出 vd              */
    Pi_Para_Struct* pi2_speed_para; /* 速度环: Kp/Ki/integral/限幅, 输出 target_iq          */
    Pi_Para_Struct* pi1_mangle_para;/* 位置环: Kp/Ki/integral/限幅, 输出 target_speed       */
} MOTOR_PI_PARA;



typedef struct
{
    MOTOR_COMPIONENTS  components;  /* ① 组件层: 角度/电流/电压/PWM 四个数据源指针          */
    MOTOR_STATE        state;       /* ② 状态层: 全局状态 + 运行子模式                     */
//    MOTOR_PARAMETERS   parameters;  /* 电机物理参数: R/L/磁链/极对数 (保留)                 */
//    MOTOR_CONTROLLER  Controller;  /* 控制器实例: (保留)                                  */
    MOTOR_PI_PARA      pi;          /* ③ 控制层: 三环PI参数指针集合                        */
} MOTOR_DATA;
extern MOTOR_DATA motorData;



void target_angle_update(uint16_t ta, Pi_Data_Struct* pa);


#endif
