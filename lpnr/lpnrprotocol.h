#ifndef _LPNR_PROTOCOL_INCLUDED_
#define _LPNR_PROTOCOL_INCLUDED_
/*
 * This header file defines data communication for hitool lpnr function with PC tool.
 */
#if !defined linux || !defined __HI_TYPE_H__
typedef unsigned int			HI_U32;
typedef int							HI_S32;
typedef unsigned short		HI_U16;
typedef short						HI_S16;
typedef unsigned char		HI_U8;
typedef char						HI_S8;

typedef struct {
	short	x;
	short	y;
} HI_SPOINT, *PHI_SPOINT;

typedef struct {
   unsigned short	x;
   unsigned short	y;
   unsigned short	width;
   unsigned short	height;
} HI_SRECT, *PHI_SRECT;
#endif

#define SZ_LABEL			20		// 摄像机名字长度

#if !defined linux || !defined _IVE_TYPE_INCLUDED_
typedef enum {
	PLT_ERROR = -1,
	PLT_UNKNOWN = 0,
	PLT_REGULAR,		// regular plate for civilan
	PLT_CHKMT,			// consulate, HK, Macao, Training vehicle
	PLT_DBLREAR,		// double line rear plate
	PLT_POLICE,			// police vehicle
	PLT_EMBSY,				// embassy vehicle
	PLT_NENRGY,			// new energy
	PLT_SPECIAL			// special plate (港口，机场内部车牌, 例如:津港H1234)
} PLATE_TYPE_E;

typedef enum {
	PLC_ERROR = -1,
	PLC_UNKNOWN,
	PLC_BLUE,
	PLC_YELLOW,
	PLC_WHITE,
	PLC_BLACK,
	PLC_GREEN,
	PLC_YELGREEN,
} PLATE_COLOR_E;
#endif

// 触发来源枚举类型
typedef enum { 
	IMG_UPLOAD=0, 			// 返送图片分析
	IMG_HOSTTRIGGER, 		// 上位机软触发
	IMG_LOOPTRIGGER, 		// 抓拍线圈触发
	IMG_AUTOTRIG, 			// 定时自动触发 (强度测试用)
	IMG_VLOOPTRG, 			// 虚拟线圈检测触发
	IMG_OVRSPEED 				// 超速触发抓拍
} TRIG_SRC_E;

 
#define PORT_LISTEN		6008		// TCP
#define PORT_SEARCH		6009		// UDP

// [Data Type]
#if 0 // for big endian CPU architecture. we want to byte stream sending in sequence of AA BB CC xx
#define DTYP_IMAGE	0xaabbcc00		
#define DTYP_CONF		0xaabbcc11
#define DTYP_EXCONF	0xaabbcc12
#define DTYP_H264CONF  0xaabbcc13
#define DTYP_DATA		0xaabbcc22
#define DTYP_CTRL		0xaabbcc33
#define DTYP_TEXT		0xaabbcc44
#define DTYP_TIME		0xaabbcc55
#define DTYP_UPDSW	0xaabbcc66
#define DTYP_UDP		0xaabbcc77
#define DTYP_HBEAT	0xaabbcc88
#define DTYP_EVENT	0xaabbcc99
#define DTYP_QUERY	0xaabbcce0
#define DTYP_REPLY	0xaabbccf0		// reply DTYP_QUERY
#define DTYP_ACK		0xaabbccdd
#define DTYP_MASK		0xffffff00
#define DTYP_MAGIC	0xaabbcc00
#else		// for little endian CPU architechure.
#define DTYP_IMAGE	0x00ccbbaa		// camera -> host
#define DTYP_CONF		0x11ccbbaa		// both way
#define DTYP_EXCONF	0x12ccbbaa
#define DTYP_H264CONF  0x13ccbbaa
#define DTYP_OSDCONF	0x14ccbbaa
#define DTYP_OSDTEXT	0x15ccbbaa
#define DTYP_DATA		0x22ccbbaa		// camera -> host
#define DTYP_CTRL		0x33ccbbaa		// host -> camera
#define DTYP_TEXT		0x44ccbbaa		// camera -> host
#define DTYP_TIME		0x55ccbbaa		// host -> camera
#define DTYP_UPDSW	0x66ccbbaa		// host -> camera
#define DTYP_UDP		0x77ccbbaa		// host -> camera (UDP command - search and ping)
#define DTYP_HBEAT	0x88ccbbaa		// both way
#define DTYP_EVENT	0x99ccbbaa		// camera -> host
#define DTYP_QUERY	0xe0ccbbaa
#define DTYP_REPLY	0xf0ccbbaa		// reply DTYP_QUERY
#define DTYP_ACK		0xddccbbaa		// host -> camera
#define DTYP_MASK		0x00ffffff
#define DTYP_MAGIC	0x00ccbbaa

#endif

// [Data Id]
// image Id - full frame image (when data type is DTYP_IMAGE)
#define IID_LIVE				0
#define IID_CAP				1		// captured image (or uploaded image level 0)
#define IID_HEAD				2		// 车头图片(CIF大小）
#define IID_QUAD				3		// 1/4解像度图
#define IID_T3RD				4		// plate located and segmented (level 1)
#define IID_RAWYUV		5		// Raw YUV422SP or YUV420SP image
#define IID_LIVEQCIF		6
#define IID_CAPOSD			7		// 叠加用户指定字符后的抓拍全图
#define IID_QUADOSD		8		// 叠加用户指定字符后的抓拍1/4图
#define IID_VIDOB			9		// Video OB image (1/4解像度，有叠加ROI区的二值图)
// image Id - image of plate(s) area
#define IID_PLRGB			11		// plate color rectangular (level 1)
#define IID_PLBIN				12  	// plate binarized (level 1)
#define IID_PLBINCLR		13		// plate binarized after de-clutter (level 1)
#define IID_PLMSKTRM		14		// plate characters group rectangular (level 2)
#define IID_PLMSK			15		// trimed plate binarized image after filter (level 2)
// image Id - image of plate characters
#define IID_PCCUT			21		// cut
#define IID_PCROT			22		// rotated
#define IID_PCTWSTH		23		// twisted horizontal
#define IID_PCTWSTV		24		// twisted vertical
#define IID_PCNORM		25		// normalized

// data Id when data type is DTYP_QUERY. 应答是DTYP_REPLY, 同样的Data Id，内容根据DataId而异
// 如果查询的数据有错误（参数错误，例如查询不存在的车位), Reply的DataId低16位是查询的DataId, 高16位是错误码
// DTYP_QUERY可以是TCP，也可以是UDP，所以，DataId由100开始编号，和DTYP_UDP使用的DataId错误。这样
// UDP处理函数好处理，只要根据DataId处理就可以
#define QID_PKPLATE	100		// 当前车位车牌号码,对于多车位，dataAttr.val[0]为车位号. (1~x, 0表示全部车位)
												// 应答时，车牌讯息在header.plateInfo结构里面。header.plateInfo.plateId是车位号(1-based)

// Reply 的错误码（在Reply header的DataId高16位)
#define QER_PARAM		1		// 查询参数错误
#define QER_ILLEGAL	2		// 当前状态不允许查询
#define QER_INVALID	3		// 查询的QID不存在

// data Id (when data type is DTYP_DATA)
#define DID_BEGIN		0		// begin of LPNR process data/image sending
#define DID_PLATE		1		// plate data
#define DID_TIMING		2		// timing data
#define DID_END			3		// end of LPNR process data/image sending
#define DID_HBEAT		9		// heart beat (reserved)
#define DID_VERSION	10	// firmware version
#define DID_ISP			11		// ISP current setting
#define DID_SHMSTAT	12		// LPNR statistic in shared memory
#define DID_SNKEY		13		// S/N and activiated or not
#define DID_NETCONF	14		// IP, Netmask and default gateway
#define DID_CFGDATA 15		// 补充的配置内容
													//   DataAttr.val[0] - 日夜切换的曝光阀值
													//   DataAttr.u16Val[2] - 无识别重做的延时时间(ms)
													//   DataAttr.u16Val[3] - 超速值 (km/hr)
													//	 DataAttr.u16Val[4] - 超速抓拍的两个线圈距离 (mm)
													//	 DataAttr.u16Val[5] - 是否没有超速也抓拍（0-no，1-yes） 1.3.5.20 版本新增
													//   DataAttr.val[3] - 特殊车牌使能
// data Id (for lane controller parts, data type is still DTYP_DATA
#define DID_EXTDIO		101		// external GPIO state (response to CTRL_READEXTDIO)
#define DID_COMDATA	102		// 串口异步输入数据, header.size是数据长度，payload是数据内容
// data Id (for lane controller parts, data type is still DTYP_DATA)
#define DID_EXTDIO	101		// external GPIO state (response to CTRL_READEXTDIO)

// data Id (when data type is DTYP_UDP)
#define DID_SEARCH		21
#define DID_PING				22
#define DID_ANSWER		23		// answer search
#define DID_TIME			24		// 对时帧 (header的TimerAttr结构是时间)
#define DID_CTRL			25		// UDP控制帧，所有TCP的控制方式都可以接受，header.ctrlAttr是控制内容

// data Id (when data type is DTYP_TEXT )
#define TID_CAPTION		0			// show on host window caption
#define TID_PROMPT		1			// show a prompt dialog on host
#define TID_LONGTEXT		2			// long text after header. header.size gives text body (include terminaated \0).
#define TID_PLATENUM		3			// 2017-04-18 新增加的车牌号Text （分析后提前发送，比图片还早）
#define TID_QRCODE		4			// QrCode text, if header.size>0, text is in payload, otherwise, it is in header.textAttr.text

// data Id (when data type is DTYP_EVENT)
#define EID_TRIGGER		1			// an analysis is triggered.
#define EID_EXTDI			2			// external DI changed (dataAttr.val[0] di_this, val[1] di_last)
#define EID_VLDIN			3			// 车辆进入虚拟线圈（已经获取车牌识别结果）
#define EID_VLDOUT		4			// 车辆离开虚拟线圈

// data Id (when data type is DTYP_CTRL

// image format (enum for 'format' member in structure ImageAttr)
#define IFMT_JPEG				1
#define IFMT_BMP				2
#define IFMT_RGB					3
#define IFMT_YUV422SP		4
#define IFMT_YUV420SP		5
#define IFMT_YUV					6		// raw format, without any header
#define IFMT_BUTT				7
// definition of code member in structure CtrlAttr
#define CTRL_TRIGGER		1		// software trigger, no parameter
#define CTRL_IMLEVEL			2		// output itermidiate results level. param is level (0~3)
#define CTRL_SETIP				3		// set IP address (param is new IP, also in options)
#define CTRL_LIVEFEED		4		// live feed on/off (param=1 is on)
#define CTRL_ISP					5		// ISP control. param=0 means trying param=1 means confirm (write to isp.conf)
														// bit map for param 
#define CTRL_ACTIVATE			6		// option is the activate key
#define CTRL_RESETSHM			7		// reset shm data
#define CTRL_RESEND				8		// resend LPNR key data (vin image, plate rgb image, plate bin image, plate info data, timing data, lpnr attribute data
#define CTRL_SNAPCAP			9		// take a snap input image if lighting is enabled also turn on lighting
														// and send the captured image to host (jpeg)
#define CTRL_TAKEFRAME		10		// get a YUV raw image frame from VI frame buffer from VB pool
#define CTRL_PLATEONLY		11		// only send recognition result (plate# and color). do not send any image result (param=1 plate_only, param=0 with image)
#define CTRL_LIGHT					12 	// light control, ctrl.param 1 开灯，0 关灯. ctrl.option[0~3]合并为int，为开灯时间 (msec) - 只有开灯时有效，0表示一直开着。
#define CTRL_AUTOROI			13		// auto detect roi for vehicle separator camera
#define CTRL_SPEEDTRG			14		// 超速触发， param是车速值（使用于和其他测试设备联动）ctrl.param为速度值 (km/h)
#define CTRL_IRCUT					15		// 控制IR-cut ctrl.param 1 使用IR滤镜，0 不使用滤镜
// lane controller parts
#define CTRL_READEXTDIO		101	// read external GPIO
#define CTRL_WRITEEXTDO		102	// write external DO (param要控制几个点，ctrlAttr.option[i]第几个Do点，ctrlAttr.option[i+1]为Do的值。一共可同时控制12个点最多。
#define CTRL_DOPULSE			103	// output DO pulse (50% duty cycle), param为 point + count<<8 + (pulse period in msec)<<16. count 为0表示一直发送直到重新设置该point的DO值(CTRL_WRITEEXTDO)
// tranparent tty port
#define CTRL_COMSET				111	// 设置串口波特率, param为波特率, ctrlAttr.option[0]是否接收RX数据. param=0 为关闭串口
#define	CTRL_COMTX				112	// 发送串口数据， header.size为数据长度, payload是要传送的数据
#define CTRL_COMASYN			113	// 是否接收EXTCOM的RX数据 param=1接收，0不接收(默认)。
// reboot
#define CTRL_REBOOT				999
						
// ISP control arguents (ctrlAttr.param bitmap and ctrlAttr.option array for CTRL_ISP)
#define ISP_WRITE_SETTING			0x00000001	// write setting to isp.conf
#define ISP_MASK_GAMA					0x0000000e   // bit1~bit4: Gamma curve (0:1.6, 1:1.8, 2:2.0, 3:2.2, 4:user; default is 1.6)
#define ISP_SHIFT_GAMA					1
#define ISP_MASK_AEW					0x000000f0   // Auto Exposure weighting table weighting within ROI (1~10)
#define ISP_SHIFT_AEW					4
#define	ISP_EN_WDR						0x00000100	// enable WDR (0 disable, 1 enable)
#define ISP_EN_ANTIFLICKER			0x00000200	// enable anti-flicker (0 disable, 1 enable)
#define ISP_DE_AUTOWB					0x00000400  // disable auto-white balance.
#define ISP_EN_DAYNIGHTGAMMA	0x00000800	// enable day/night gamma setting (sync with PARM_EN_DAYNIGHTSW) 
#define ISP_DE_AUTOEXP				0x00001000	// disable AE
#define ISP_MASK_MINAEEXPT		0x0000e000	// 
#define ISP_SHIFT_MINAEEXPT		13
#define ISP_MASK_DEFROGVAL		0x000f0000	// 3516A 去雾强度 (0~15)*16
#define ISP_SHIFT_DEFROGVAL		16					
#define ISP_MASK_EXPTIME			0xffc00000	// manual exposure (10 bits)
#define ISP_SHIFT_EXPTIME			22

#define ISP_GetGama(p)				( ((p) & ISP_MASK_GAMA) >> ISP_SHIFT_GAMA )
#define ISP_SetGama(p,v)			(p) = ( ((p) & ~ISP_MASK_GAMA) | ( (v) << ISP_SHIFT_GAMA) )
#define ISP_GetAEW(p)					( ((p) & ISP_MASK_AEW) >> ISP_SHIFT_AEW )
#define ISP_SetAEW(p,v)				(p) = ( ((p) & ~ISP_MASK_AEW) | ( (v) << ISP_SHIFT_AEW) )
#define ISP_ENABLE_WDR(p)			( ((p) & ISP_EN_WDR) != 0 )
#define ISP_SET_ENABLE_WDR(p)	((p) |= ISP_EN_WDR)
#define ISP_CLR_ENABLE_WDR(p)	((p) &= ~ISP_EN_WDR)
#define ISP_ENABLE_ANTIFLICKER(p)  ( ((p) & ISP_EN_ANTIFLICKER) != 0 )
#define ISP_SET_ENABLE_ANTIFLICKER(p)  ((p) |= ISP_EN_ANTIFLICKER)
#define ISP_CLR_ENABLE_ANTIFLICKER(p)  ((p) &= ~ISP_EN_ANTIFLICKER)
#define ISP_ENABLE_WRITE(p)		( ((p) & ISP_WRITE_SETTING) != 0 )
#define ISP_DISABLE_AWB(p)		( ((p) & ISP_DE_AUTOWB) != 0)
#define ISP_SET_DISABLE_AWB(p)	((p) |= ISP_DE_AUTOWB)
#define ISP_CLR_DISABLE_AWB(p)	((p) &= ~ISP_DE_AUTOWB)
#define ISP_DISABLE_AE(p)				( ((p) & ISP_DE_AUTOEXP) != 0)
#define ISP_SET_DISABLE_AE(p)		((p) |= ISP_DE_AUTOEXP)
#define ISP_CLR_DISABLE_AE(p)		((p) &= ~ISP_DE_AUTOEXP)
#define ISP_GetExptime(p)					( ((p) & ISP_MASK_EXPTIME) >> ISP_SHIFT_EXPTIME )
#define ISP_SetExptime(p,v)				(p) = ( ((p) & ~ISP_MASK_EXPTIME) | ( (v) << ISP_SHIFT_EXPTIME) )
#define ISP_GetAEMinExpTime(p)	( ((p) & ISP_MASK_MINAEEXPT) >> ISP_SHIFT_MINAEEXPT )
#define ISP_SetAEMinExpTime(p,v)	(p) = ( ((p) & ~ISP_MASK_MINAEEXPT) | ( (v) << ISP_SHIFT_MINAEEXPT) )
#define ISP_GetDefrogVal(p)			( ((p) & ISP_MASK_DEFROGVAL) >> ISP_SHIFT_DEFROGVAL )
#define ISP_SetDefrogVal(p,v)		(p) = ( ((p) & ~ISP_MASK_DEFROGVAL) | ( (v) << ISP_SHIFT_DEFROGVAL) )
#define ISP_GetNormGamma(u8v)	( ((double)(u8v)) / 10 )
#define ISP_IsEnableDayNightGamma(p)	 ( ((p) & ISP_EN_DAYNIGHTGAMMA) != 0)
#define ISP_EnableDayNightGamma(p,en) \
	do { \
		if ( en ) \
			p |= ISP_EN_DAYNIGHTGAMMA; \
		else \
			p &= ~ISP_EN_DAYNIGHTGAMMA; \
	} while (0)
														/* option array for CTRL_ISP:
														 * 0: Brightness compensation: 0 ~ 255, 128 is default
														 * 1: Saturation: 0 ~ 255, 128 is default
														 * 2: Sharpness: 0 ~ 255. 128 is default.
														 * 3: Shuttle Speed (the lowest) - 1 ~ 250 (means 1/N sec.)
														 * 4: De-noize target threshold (0~255, 0 for none).
														 * 5: DRC strength (0~255, def:128)
														 * 6: CSC luminance (0~100, def 50)
														 * 7: CSC contrast value (0~100, def 50)
														 * 8: CSC hue value (0~100, def. 50)
														 * 9: CSC saturation (0~100, def. 50)
														 * 10: AG low
														 * 11: AG high
														 * 12: DG low
														 * 13: DG high
														 * 14: JPEG quality (1~99)
														 * 15: AE Exp Step (0~255) - note: set too high will cause brightness fructuation 【手动曝光时候，这里储存的是AG】
														 * 16: AE exp tolerance (0~255) 【手动曝光时候，这里储存的是DG】
														 * 17: Snap mode Analog Gain   【3516A这里储存的是 ISP Gaim Max (min是1)】
														 * 18: Snap mode Digital Gain
														 * 19: Snap mode Exposure time (100 usec)
														 * 20: Snap trigger offset time in 100 usec. negative means time before CCD scan. 
														 * 21: Manual WB color temperature (x1000) is temoerature K)
														 * 22: day time gamma value (for user defined) (3 for 0.3, 28 for 2.8 etc)
														 * 23: night time gamma value (for user define)(3 for 0.3, 28 for 2.8 etc)
														 */

// parameter options (bit difinition for 'enabler' member in structure ParamConf)
#define PARM_EN_LOOPTRI						0x00000001		// enable loop trigger
#define PARM_EN_HEXLIVE						0x00000002		// enable send 1/16 image as live feed frame
//#define PARM_EN_DBLPLATE					0x00000002		// enable double line plate
//#define PARM_EN_MULTIPLATE				0x00000004		// enable multi-plate
#define PARM_EN_QUADLIVE					0x00000004		// enable send quad frame as live feed 
#define PARM_EN_LIGHTING						0x00000008		// enable lighting control
#define PARM_EN_LOOPPULSE				0x00000010		// loop signal is pulse mode.
#define PARM_EN_RETRYNEXT					0x00000020		// enable retry again by capture a frame do LPNR again
#define PARM_EN_DGDOWNONNIGHT		0x00000040		// reduce DG and ISP gain on night time when capture (day-night detection & nighting enabled)
#define PARM_EN_FLASHLIGHT				0x00000080		// yes for flash light, no for LED light
#define PARM_EN_SENDQTRIMG				0x00000100		// send quarter image (jpeg) as captured image (浙江尤其需要)
#define PARM_EN_DAYNIGHTSW				0x00000200		// enable day-night detection and setting for AG-DG-ET capture setting
#define PARM_EN_2NDLEDLIGHT				0x00000400		// 允许第2补光灯(环境光补光，必须是LED。如果设置只在夜晚补光，那就晚上才点亮)
#define PARM_EN_VLOOP							0x00000800		// enable virtual loop
#define PARM_EN_IRLIGHT						0x00001000		// 红外补光灯
#define PARM_DE_VDRPTSAMEPLATE		0x00002000		// 禁止视频触发情况下同一个车牌重复上报
#define PARM_EN_LIGHTING_ONNIGHT		0x00004000		// 补光灯只有晚上启用
#define PARM_EN_SAVERAW					0x00008000		// 没有识别的话储存原始图片（需要SD卡）
#define PARM_ENHANCEHZ_MASK			0x003f0000		// 强化字 0~63 （目前只用到0~31）
#define PARM_ENHANCEHZ_SHIF				16
#define PARM_EN_DNLDYUV					0x00400000		// 允许下载原始图像到客户端
#define PARM_EN_YUV4ALL						0x00800000		// 每次识别结果都下载YUV图像(没有使能这个的话，只有未识别才下载）
// 新增功能
#define PARM_EN_OVLPLIMG					0x01000000		// 叠加车牌小图到1/4左下角
#define PARM_DE_CHARO						0x02000000		// 车牌禁止出现O（出现了替换为D）
#define PARM_EN_HALFGAINONREDO		0x04000000		// 无识别重做时减半增益
#define PARM_DE_FULLIMG						0x08000000		// 禁止输出全图
#define PARM_EN_HEADIMG						0x10000000		// 允许输出车头图
#define PARM_EN_QUADIMG					0x20000000		// 允许输出1/4解像度图
#define PARM_EN_SPEEDING					0x40000000		// 使能超速抓拍（只允许291一体机机型，有额外的输入点，测试线圈2个，在第二个线圈抓拍超速)
#define PARM_EN_T3RDIMG						0x80000000		// 允许输出4/9图

// bit-map of ParamConf.ex_enabler[0]
#define PARM_EN0_REDUCEGAINPERIOD			0x00000001		// 允许指定时段降低增益

#define IS_EN_LOOPTRIG(param)				((param).enabler & PARM_EN_LOOPTRI)
#define SET_EN_LOOPTRIG(param)			((param).enabler |= PARM_EN_LOOPTRI)
#define CLR_EN_LOOPTRIG(param)			((param).enabler &= ~PARM_EN_LOOPTRI)
#define IS_EN_HEXLIVE(param)				((param).enabler & PARM_EN_HEXLIVE)
#define SET_EN_HEXLIVE(param)			((param).enabler |= PARM_EN_HEXLIVE)
#define CLR_EN_HEXLIVE(param)			((param).enabler &= ~PARM_EN_HEXLIVE)
#define IS_EN_QUADLIVE(param)				((param).enabler & PARM_EN_QUADLIVE)
#define SET_EN_QUADLIVE(param)			((param).enabler |= PARM_EN_QUADLIVE)
#define CLR_EN_QUADLIVE(param)			((param).enabler &= ~PARM_EN_QUADLIVE)
//#define IS_EN_DBLPLATE(param)				((param).enabler & PARM_EN_DBLPLATE)
//#define IS_EN_MULTIPLATE(param) 		((param).enabler & PARM_EN_MULTIPLATE)
//#define SET_EN_MULTIPLATE(param) 		((param).enabler |= PARM_EN_MULTIPLATE)
//#define CLR_EN_MULTIPLATE(param) 		((param).enabler &= ~PARM_EN_MULTIPLATE)
#define IS_EN_LIGHTING(param)				((param).enabler & PARM_EN_LIGHTING)
#define SET_EN_LIGHTING(param)			((param).enabler |= PARM_EN_LIGHTING)
#define CLR_EN_LIGHTING(param)			((param).enabler &= ~PARM_EN_LIGHTING)
#define IS_EN_LOOPSIGPULSE(param)		((param).enabler & PARM_EN_LOOPPULSE)
#define SET_EN_LOOPSIGPULSE(param)	((param).enabler |= PARM_EN_LOOPPULSE)
#define CLR_EN_LOOPSIGPULSE(param)	((param).enabler &= ~PARM_EN_LOOPPULSE)
#define IS_EN_RETRYNEXT(param)			((param).enabler & PARM_EN_RETRYNEXT)
#define SET_EN_RETRYNEXT(param)			((param).enabler |= PARM_EN_RETRYNEXT)
#define CLR_EN_RETRYNEXT(param)			((param).enabler &= ~PARM_EN_RETRYNEXT)
#define IS_EN_SENDQTRIMG(param)			((param).enabler & PARM_EN_SENDQTRIMG)	// 允许发送抓拍小图 (保留，等到JPEG可以压缩1/4图)
#define IS_EN_DAYNIGHTMODE(param)		((param).enabler & PARM_EN_DAYNIGHTSW)
#define SET_EN_DAYNIGHTMODE(param)	((param).enabler |= PARM_EN_DAYNIGHTSW)
#define CLR_EN_DAYNIGHTMODE(param)	((param).enabler &= ~PARM_EN_DAYNIGHTSW)
#define IS_EN_2NDLEDLIGHT(param)		((param).enabler & PARM_EN_2NDLEDLIGHT)
#define SET_EN_2NDLEDLIGHT(param)		((param).enabler |= PARM_EN_2NDLEDLIGHT)
#define CLR_EN_2NDLEDLIGHT(param)		((param).enabler &= ~PARM_EN_2NDLEDLIGHT)
#define IS_EN_VLOOP(param)					((param).enabler & PARM_EN_VLOOP)
#define SET_EN_VLOOP(param)					((param).enabler |= PARM_EN_VLOOP)
#define CLR_EN_VLOOP(param)					((param).enabler &= ~PARM_EN_VLOOP)
#define IS_DE_VDRPTSAMEPLATE(parm)	((param).enabler & PARM_DE_VDRPTSAMEPLATE)
#define SET_DE_VDRPTSAMEPLATE(parm)	((param).enabler |= PARM_DE_VDRPTSAMEPLATE)
#define CLR_DE_VDRPTSAMEPLATE(parm)	((param).enabler &= ~PARM_DE_VDRPTSAMEPLATE)
#define IS_EN_FLASHLIGHT(param)			((param).enabler & PARM_EN_FLASHLIGHT)
#define SET_EN_FLASHLIGHT(param)		((param).enabler |= PARM_EN_FLASHLIGHT)
#define CLR_EN_FLASHLIGHT(param)		((param).enabler &= ~PARM_EN_FLASHLIGHT)
#define IS_EN_IRLIGHT(param)			((param).enabler & PARM_EN_IRLIGHT)
#define SET_EN_IRLIGHT(param)			((param).enabler |= PARM_EN_IRLIGHT)
#define CLR_EN_IRLIGHT(param)			((param).enabler &= ~PARM_EN_IRLIGHT)
#define IS_EN_LIGHTING_ONNIGHT(param)((param).enabler & PARM_EN_LIGHTING_ONNIGHT)
#define SET_EN_LIGHTING_ONNIGHT(param)((param).enabler |= PARM_EN_LIGHTING_ONNIGHT)
#define CLR_EN_LIGHTING_ONNIGHT(param)((param).enabler &= ~PARM_EN_LIGHTING_ONNIGHT)
#define IS_EN_REDUCEGAINONNIGHT(param) ((param).enabler & PARM_EN_DGDOWNONNIGHT)
#define SET_EN_REDUCEGAINONNIGHT(param)((param).enabler |= PARM_EN_DGDOWNONNIGHT)
#define CLR_EN_REDUCEGAINONNIGHT(param)((param).enabler &= ~PARM_EN_DGDOWNONNIGHT)
#define IS_EN_SAVERAW(param)				((param).enabler & PARM_EN_SAVERAW)
#define SET_EN_SAVERAW(param)				((param).enabler |= PARM_EN_SAVERAW)
#define CLR_EN_SAVERAW(param)				((param).enabler &= ~PARM_EN_SAVERAW)
#define GET_LOOPTRIG_DELAY(param) 	((param).others[0] )			// loop triggle delay
#define SET_LOOPTRIG_DELAY(param,v) ((param).others[0]=v )			// loop triggle delay
#define GET_SYNC_OFFSET(param)			((param).others[1] )			// 频闪灯同步信号偏移量(单位106usec)
#define SET_SYNC_OFFSET(param,v)		((param).others[1]=v )
#define GET_LOOPPULSE_WIDTH(param)	((param).others[2] )
#define SET_LOOPPULSE_WIDTH(param,v)((param).others[2]=v )
//#define GET_CREDIT_THRESHOLD(param)		((param).others[3] )		// not used
//#define SET_CREDIT_THRESHOLD(param,v) ((param).others[3]=v )	// not used
#define GET_REDUCEGAIN_HOUR_BEGIN(param) ((param).others[3] & 0xff)
#define GET_REDUCEGAIN_HOUR_END(param) 	 (((param).others[3]>>8) & 0xff)
#define GET_PLATE_WIDTH(param)			((param).others[4])
#define SET_PLATE_WIDTH(param,v)			((param).others[4]=v)
#define GET_FREQLIGHT_DUTYRATIO(param) ((param).others[5] )
#define SET_FREQLIGHT_DUTYRATIO(param,v) ((param).others[5]=v )
//#define GET_FONT_HEIGHT(param)			((param).CHA3[0] )
//#define GET_TILT_ANGLE(param)				((param).CHA3[1] )
//#define SET_TILT_ANGLE(param,v)			((param).CHA3[1]=v )
//#define GET_PAN_ANGLE(param)				((param).CHA3[2] )
//#define SET_PAN_ANGLE(param,v)			((param).CHA3[2]=v )
//#define GET_ROTATE_ANGLE(param)			((param).CHA3[3] )
//#define SET_ROTATE_ANGLE(param,v)		((param).CHA3[3]=v )
#define GET_ENHANCEHZ(param)				(((param).enabler & PARM_ENHANCEHZ_MASK) >> PARM_ENHANCEHZ_SHIF )
#define SET_ENHANCEHZ(param,v)			(param).enabler = ((param).enabler & ~PARM_ENHANCEHZ_MASK) | ((v) << PARM_ENHANCEHZ_SHIF)
#define IS_DE_FULLIMG(param)				((param).enabler & PARM_DE_FULLIMG)
#define SET_DE_FULLIMG(param)			((param).enabler |= PARM_DE_FULLIMG)
#define CLR_DE_FULLIMG(param)			((param).enabler &= ~PARM_DE_FULLIMG)
#define  IS_EN_HEADIMG(param)				((param).enabler & PARM_EN_HEADIMG)
#define  SET_EN_HEADIMG(param)			((param).enabler |= PARM_EN_HEADIMG)
#define  CLR_EN_HEADIMG(param)			((param).enabler &= ~PARM_EN_HEADIMG)
#define  IS_EN_QUADIMG(param)				((param).enabler & PARM_EN_QUADIMG)
#define  SET_EN_QUADIMG(param)			((param).enabler |= PARM_EN_QUADIMG)
#define  CLR_EN_QUADIMG(param)			((param).enabler &= ~PARM_EN_QUADIMG)
#define  IS_EN_SPEEDING(param)				((param).enabler & PARM_EN_SPEEDING)
#define  SET_EN_SPEEDING(param)			((param).enabler |= PARM_EN_SPEEDING)
#define  CLR_EN_SPEEDING(param)			((param).enabler &= ~PARM_EN_SPEEDING)
#define  IS_EN_T3RDIMG(param)				((param).enabler & PARM_EN_T3RDIMG)
#define  SET_EN_T3RDIMG(param)			((param).enabler |= PARM_EN_T3RDIMG)
#define  CLR_EN_T3RDIMG(param)			((param).enabler &= ~PARM_EN_T3RDIMG)
#define IS_EN_OVLPLIMG(param)			((param).enabler & PARM_EN_OVLPLIMG)
#define SET_EN_OVLPLIMG(param)		(param).enabler |= PARM_EN_OVLPLIMG)
#define CLR_EN_OVLPLIMG(param)		(param).enabler &= ~PARM_EN_OVLPLIMG)
#define IS_DE_CHARO(param)				((param).enabler & PARM_DE_CHARO)
#define IS_EN_HALFGAINONREDO(param) ((param).enabler & PARM_EN_HALFGAINONREDO)
// for paramConf.ex_enabler[0] & paramConf.ex_enabler[1]
#define IS_EN_REDUCEGAIN_PERIOD(param)  ((param).ex_enabler[0] & PARM_EN0_REDUCEGAINPERIOD)

// parameter options (bit difinition for 'enabler' member in structure ExtParamConf)
#define EXTPARM_EN_OVLPLATE		0x00000001
#define EXTPARM_EN_OVLTIME		0x00000002
#define EXTPARM_EN_OVLLABEL		0x00000004
#define EXTPARM_EN_OVLRCTIME	0x00000008		// 使能叠加识别时间
#define EXTPARM_EN_OVLMASK		0x0000000f
// other config enablers
#define EXTPARM_EN_TIMERTRIG	0x00000010		// 使能定时触发 （时间周期在CfgData.u16Val[8]）
#define IS_EN_TIMERTRIG(exparm)	(((exparm).enabler & EXTPARM_EN_TIMERTRIG)!=0)
#define EXTPARM_EN_FIXNUMLST	0x00000020
#define IS_EN_FIXNUMLST(exparm)	(((exparm).enabler & ENTPARM_EN_FIXNUMLST)!=0)

#define IS_EN_OVERLAY(exparm)		(((exparm).enabler & EXTPARM_EN_OVLMASK)!=0)
#define SET_EN_OVERLAY(exparm)		(((exparm).enabler |= EXTPARM_EN_OVLMASK))
#define CLR_EN_OVERLAY(exparm)		(((exparm).enabler &= ~EXTPARM_EN_OVLMASK))
#define IS_EN_OVLPLATE(exparm)	(((exparm).enabler & EXTPARM_EN_OVLPLATE) != 0)
#define SET_EN_OVLPLATE(exparm)	(((exparm).enabler |= EXTPARM_EN_OVLPLATE))
#define CLR_EN_OVLPLATE(exparm)	(((exparm).enabler &= ~EXTPARM_EN_OVLPLATE))
#define IS_EN_OVLTIME(exparm)		(((exparm).enabler & EXTPARM_EN_OVLTIME) != 0)
#define SET_EN_OVLTIME(exparm)	(((exparm).enabler |= EXTPARM_EN_OVLTIME))
#define CLR_EN_OVLTIME(exparm)	(((exparm).enabler &= ~EXTPARM_EN_OVLTIME))
#define IS_EN_OVLLABEL(exparm)	(((exparm).enabler & EXTPARM_EN_OVLLABEL) != 0)
#define SET_EN_OVLLABEL(exparm)	(((exparm).enabler |= EXTPARM_EN_OVLLABEL))
#define CLR_EN_OVLLABEL(exparm)	(((exparm).enabler &= ~EXTPARM_EN_OVLLABEL))
#define IS_EN_OVLRCTIME(exparm)	(((exparm).enabler & EXTPARM_EN_OVLRCTIME) != 0)
#define SET_EN_OVLRCTIME(exparm)	(((exparm).enabler |= EXTPARM_EN_OVLRCTIME))
#define CLR_EN_OVLRCTIME(exparm)	(((exparm).enabler &= ~EXTPARM_EN_OVLRCTIME))
#define EXTPARM_SAMEPLATE_MASK	0x00000C00		// 视频触发时，连续几次识别相同才上报 0~3 
#define EXTPARM_SAMEPLATE_SHIFT 10
#define EXTPARM_FORMAT_MASK		0x0000F000		
#define EXTPARM_FORMAT_SHIFT	12
#define EXTPARM_FONT_IDMASK			0x000F0000		// 0: 宋体，1:细明体，2:楷体，3:华文云彩（中空字)
#define EXTPARM_FONT_IDSHIFT		16
#define EXTPARM_SIZE_IDMASK			0x00F00000		// 0:24, 1:32, 2:40, 3:48. 4:56. 5:64
#define EXTPARM_SIZE_IDSHIFT		20
#define EXTPARM_FGCOLOR_IDMASK	0x0F000000	// 白色;黑色;冷灰色;橙色;金黄色;红色;棕色;蓝色;靛青;天蓝;草绿;森林绿;紫色
#define EXTPARM_FGCOLOR_IDSHIFT	24
#define EXTPARM_BGCOLOR_IDMASK	0xF0000000	// 无色;白色;黑色;冷灰色;橙色;金黄色;红色;棕色;蓝色;靛青;天蓝;草绿;森林绿;紫色
#define EXTPARM_BGCOLOR_IDSHIFT	28
#define EXTPARM_GET_SAMEPLATECNT(p)			( (((p).enabler) & EXTPARM_SAMEPLATE_MASK) >> EXTPARM_SAMEPLATE_SHIFT)
#define EXTPARM_GET_FORMAT(p)			( (((p).enabler) & EXTPARM_FORMAT_MASK) >> EXTPARM_FORMAT_SHIFT)
#define EXTPARM_GET_FONTID(p)			( (((p).enabler) & EXTPARM_FONT_IDMASK) >> EXTPARM_FONT_IDSHIFT)
#define EXTPARM_GET_SIZEID(p)			( (((p).enabler) & EXTPARM_SIZE_IDMASK) >> EXTPARM_SIZE_IDSHIFT)
#define EXTPARM_GET_FGCOLORID(p)	( (((p).enabler) & EXTPARM_FGCOLOR_IDMASK) >> EXTPARM_FGCOLOR_IDSHIFT)
#define EXTPARM_GET_BGCOLORID(p)	( (((p).enabler) & EXTPARM_BGCOLOR_IDMASK) >> EXTPARM_BGCOLOR_IDSHIFT)
#define EXTPARM_SETID(p,id,mask,shift) \
	do { \
		(p).enabler &= ~mask; \
		(p).enabler |= (id) << shift; \
	} while(0)
#define EXTPARM_SET_SAMEPLATECNT(p,cnt) EXTPARM_SETID(p,fmt,EXTPARM_SAMEPLATE_MASK,EXTPARM_SAMEPLATE_SHIFT)
#define EXTPARM_SET_FORMAT(p,fmt)  EXTPARM_SETID(p,fmt,EXTPARM_FORMAT_MASK,EXTPARM_FORMAT_SHIFT)
#define EXTPARM_SET_FONTID(p,id)  EXTPARM_SETID(p,id,EXTPARM_FONT_IDMASK,EXTPARM_FONT_IDSHIFT)
#define EXTPARM_SET_SIZEID(p,id)  EXTPARM_SETID(p,id,EXTPARM_SIZE_IDMASK,EXTPARM_SIZE_IDSHIFT)
#define EXTPARM_SET_FGCOLORID(p,id)  EXTPARM_SETID(p,id,EXTPARM_FGCOLOR_IDMASK,EXTPARM_FGCOLOR_IDSHIFT)
#define EXTPARM_SET_BGCOLORID(p,id)  EXTPARM_SETID(p,id,EXTPARM_BGCOLOR_IDMASK,EXTPARM_BGCOLOR_IDSHIFT)
#define EXTPARM_GET_LABEL(p)			(p.label)
#define EXTPARM_SET_LABEL(p,lb)	\
	do { \
		memcpy(p.label,lb,SZ_LABEL); \
		p.label[SZ_LABEL-1] = '\0'; \
	while (0) 
	
// enabler bit to report camera capabilities (while report camera version header) - P3 
#define CAP_VIRTUALLOOP					0x00000001		// ENABLE_VIRTUALLOOP
#define CAP_VIDEOOSD						0x00000002		// ENABLE_VIDEOOSD
#define CAP_FLASHLIGHT					0x00000004		// HAS_FLASH_CONTROL==_YES
#define CAP_DAYNIGHTSWTH				0x00000008		// ENABLE_DAYNIGHTSWITCH
#define CAP_VIDEOBASEDSNAP			0x00000010		// ENABLE_VIDEOBASEDSNAP
#define CAP_FLIPMIRROW					0x00000020		// ENABLE_FLIPMIRROW
#define CAP_DYNAMICPLATEWIDTH	0x00000040		// ENABLE_DYNAMIC_PLATEWIDTH
#define CAP_EXTGPIO							0x00000080		// ENABLE_EXTGPIO
#define CAP_GPIO_DIMODE					0x00000100		// 1 normal close, 0 normal open
#define CAP_GPIO_DOMODE				0x00000200		// 1 normal close, 0 normal open
#define CAP_EXTGPIO_DIMODE			0x00000400		// 1 normal close, 0 normal open
#define CAP_EXTGPIO_DOMODE			0x00000800		// 1 normal close, 0 normal open
#define CAP_HAS_SDCARD					0x00001000		// 1 camera installed with SD card
#define CAP_HAS_RTC							0x00002000		// 1 camera has RTC
#define CAP_VHEAD_IMAGE					0x00004000		// ENABLE_VEHEAD_JPEG
#define CAP_QUAD_IMAGE					0x00008000		// ENABLE_QUAD_JPEG
#define CAP_VEHCOLORDETECTION	0x00010000		// ENABLE_VEHCOLORDETECTION
#define CAP_T3RD_IMAGE					0x00020000		// ENABLE_T3RD_JPEG
#define CAP_ALWAYSSETFLAG			0x80000000		// this bit always set, so that client could know this param. is useful

typedef struct tagTimeAttr
{
	HI_S16	year;			// 2013
	HI_S16	month;		// 1~12
	HI_S16	day;			// 1~31
	HI_S16 	hour;			// 0 ~ 23
	HI_S16	minute;		// 0 ~ 59
	HI_S16	second;		// 0 ~ 59
	HI_S16	msec;			// 0 ~ 999
} TimeAttr;

typedef struct tagImageAttr
{
	HI_S16	width;			// not required for JPG and BMP format. Only required for raw format (RGB, 422SP, 420SP)
	HI_S16	height;			// not required for JPG and BMP format. Only required for raw format (RGB, 422SP, 420SP)
	HI_S16	format;		
	HI_S16	index;			// 1 based image index (for IID_PLxx, this is plate index, for IID_PC, this plate * 10 + char Id)
											// for example, 11 is plate 1, char 1. 12 is plate 1 char 2 etc.)
	char		basename[24];		// image name, null terminated
} ImageAttr;

typedef struct tagCtrlAttr
{
	HI_S32	code;
	HI_S32	param;
	HI_U8		option[24];
} CtrlAttr;

typedef struct tagPlateInfo
{
	HI_S16	plateId;		// usually is one. Unless multiple plates detection is enabled.
	HI_S16	plateCode;	// low byte is color code PLATE_COLOR_E, high byte is type code PLATE_TYPE_E
	char  	chNum[12];	// plate string
	HI_U8	MatchRate[8];
	//HI_S8	MatchListPos[8];
	HI_SRECT plate_rect;	// plate rect relative to whole image
} PlateInfo;

#define PLATEINFO_INITIALIZER \
	{ 0, 0,{0,0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0} }
	
typedef struct tagTimeInfo
{
	HI_S32	totalElapsed;
	HI_S32	totalProcess;
#if 0
	HI_S32	timeSCW;				// capture
	HI_S32	timeRegion;
	HI_S32	timeSgmt;
	HI_S32	timeMatch;
#else
	HI_S32	timeDelay;		// 触发(线圈信号)到开始执行(采集图像)的延时
	HI_S32	timeCap;			// 采集图像花的时间
	HI_S32	timeCap2;			// 重做一次的采集时间
	HI_S32 	timeProc2;		// 重做一次的处理时间
#endif	
	HI_S32	timeFile;
	HI_S32	timeJPGEn;
} TimeInfo;

typedef struct tagParamConf {
	HI_S16	ROI[4];				// top, right, bottom, left
	union {
		HI_S16	CHA3[4];			// Char Min. Height and three angles (reserved)
		HI_U32	ex_enabler[2];
	};
	HI_S32	enabler;			// bit-wise enabler
	HI_S16	others[6];		// 0 线圈触发延时
									// 1 低8位: 频闪灯同步信号偏移量（106usec为单位）(not used now) 
									//   高8位: 
									// 2 loop signal pulse width (if loop signal is pulse type)
									// 3 可信度阀值(not used)
									//    抓拍降低曝光时段: begin hour + end hour<<8 (2017-11-24)
									// 4 车牌宽度 (for ShaiHe engine)
									// 5 低8位：频闪灯duty ratio (50表示50%)
									//   高8位：抓拍降低曝光比例(高8位，2表示1/2, 4表示1/4, 0 为默认)
} ParamConf;

typedef struct tagExtParamConf {
	HI_S16	ROI[4];				// secondary ROI for AE 
	HI_S32	enabler;				// bit-wise (0: overlay plate, 1: overlay time, 2: overlay label
	char		label[SZ_LABEL];
} ExtParamConf;

typedef enum  {
	BASELINE_PROFILE =0,
	MAIN_PROFILE,
	HIGH_PROFILE
} Enum_H264Profile;

typedef enum {
	MODE_CBR = 1,
	MODE_VBR,
	MODE_ABR,
	MODE_FIXQP
} Enum_H264RcMode;

typedef struct tagH264Conf {
	Enum_H264Profile  enProfile;
	Enum_H264RcMode enRcMode;
	HI_U32	u32BitRate;			//  max. bitrate for VBR,  or target bitrate for CBR. not used for FixQP Rc Mode
	HI_S16  s16QP[2];					// [0] is min, [1] is max for VBR, [0] is I-Qp, [1] is P-Qp for FixQP mode
	union {
		HI_U8	 u8Param[16];		// 0 是 GOP, 其他都还没用
		HI_U16	u16Param[8];
		HI_U32	u32Param[4];
	};
} H264Conf;


#define OSD_EN_TIMESTAMP	0x00000001
#define OSD_EN_LOGO				0x00000002
#define OSD_EN_LABEL			0x00000004
#define OSD_EN_PLATE			0x00000008
#define OSD_EN_TARGET1 		0x00000010	// 在视频上叠加车牌1位置矩形 (this one and OSD_ENABLE_QUADRIAL1 must be either or)
#define OSD_EN_TARGET2 		0x00000020	// 在视频上叠加车牌2位置矩形 (this one and OSD_ENABLE_QUADRIAL2 must be either or)
#define OSD_EN_TARGET3 		0x00000040	// 在视频上叠加车牌3位置矩形 (this one and OSD_ENABLE_QUADRIAL3 must be either or)
#define OSD_EN_TARGET4 		0x00000080	// 在视频上叠加车牌3位置矩形 (this one and OSD_ENABLE_QUADRIAL4 must be either or)
#define OSD_EN_QUADRIAL1	0x00000100
#define OSD_EN_QUADRIAL2	0x00000200
#define OSD_EN_QUADRIAL3	0x00000400
#define OSD_EN_QUADRIAL4	0x00000800
#define OSD_EN_ROI				0x00001000	// this one and OSD_ENABLE_QUADRIAL1 must be either or

#define OSD_IS_EN_LOGO(oc)				( ((oc)->enabler & OSD_EN_LOGO)==OSD_EN_LOGO)
#define OSD_IS_EN_LABEL(oc)				( ((oc)->enabler & OSD_EN_LABEL)==OSD_EN_LABEL )
#define OSD_IS_EN_TIMESTAMP(oc)		( ((oc)->enabler & OSD_EN_TIMESTAMP)==OSD_EN_TIMESTAMP )
#define OSD_IS_EN_PLATE(oc)				( ((oc)->enabler & OSD_EN_PLATE)==OSD_EN_PLATE )
#define OSD_IS_EN_QUADRIAL(oc,n)	( ((oc)->enabler & (OSD_EN_QUADRIAL1 << (n)) ) != 0 )		// n is 0~3
#define OSD_IS_EN_TARGET(oc,n)		( ((oc)->enabler & (OSD_EN_TARGET1 << (n)) ) != 0)			// n is 0~3
#define OSD_IS_EN_ROI(oc)					( ((oc)->enabler & OSD_EN_ROI) == OSD_EN_ROI )
#define OSD_SET_EN_LOGO(oc)				( ((oc)->enabler |= OSD_EN_LOGO))
#define OSD_SET_EN_LABEL(oc)			( ((oc)->enabler |= OSD_EN_LABEL))
#define OSD_SET_EN_TIMESTAMP(oc)	( ((oc)->enabler |= OSD_EN_TIMESTAMP))
#define OSD_SET_EN_PLATE(oc)			( ((oc)->enabler |= OSD_EN_PLATE))
#define OSD_SET_EN_ROI(oc)				( ((oc)->enabler |= OSD_EN_ROI))
#define OSD_CLR_EN_LOGO(oc)				( ((oc)->enabler &= ~OSD_EN_LOGO))
#define OSD_CLR_EN_LABEL(oc)			( ((oc)->enabler &= ~OSD_EN_LABEL))
#define OSD_CLR_EN_TIMESTAMP(oc)	( ((oc)->enabler &= ~OSD_EN_TIMESTAMP))
#define OSD_CLR_EN_PLATE(oc)			( ((oc)->enabler &= ~OSD_EN_PLATE))
#define OSD_CLR_EN_ROI(oc)				( ((oc)->enabler &= ~OSD_EN_ROI))

#define OSD_PARM_DWELL			0
#define OSD_PARM_FADEOUT		1
#define OSD_PARM_FLASH			2
#define OSD_PARM_ONTIME			3
#define OSD_PARM_OFFTIME		4
#define OSD_PARM_ALIGN			5
#define OSD_PARM_SCALE			6
#define OSD_PARM_TARGET			7		// 0 overlay on video, 1 overlay on captured image

#pragma pack (push,2)
// this structure is also used as OSDText
typedef struct tagOSDConf {
	HI_U32	enabler;					// for OSDText, this is addr of payload (text body)
	short		x;								// this is X-coord of upper-left corner of text
														// if < 0; -1 ~ -9 means ul, uc, ur, ml, mc, mr, ll, lc, lr respectively
	short 	y;								// this is Y-coord of upper-left corner of text
	short		nFontId;					// 0 宋体, 1 楷体
	short	  nFontSize;				// 32, 40, 48, 56, 64
	HI_U32	RGBForgrund;			// for text type, this is text color
	HI_U32  RGBBkground;			// for BMP overlay, this is color which to apply background alpha
	HI_U16  alpha[2];					// forground/nackground alpha			
	HI_U8		param[8];					// 0: dwell time, 1: fade-out, 2: flash or not, 3: on_time (100s of msec), 4: off_time (100s of msec), 5 - plate alignment mode
} OSDConf, *POSDConf;

typedef OSDConf OSDText;

// payload of OSD config.
// 以下结构和OSDConf一起发送，当作帧的payload。可以分别定义4个叠加区域的属性
// 如果没有发送，就用默认值。全部是0
// 这个结构是定义完整的可以设置项目，当前只实现位置可定义 (x,y) 值
#define OSDID_TIMESTAMP	0		// 时间标签
#define OSDID_LOGO			1		// runwell logo
#define OSDID_LABEL			2		// 摄像机名称
#define OSDID_PLATE			3		// 车牌号码
#define NUM_OSDID				4
typedef struct tagOSDPayload {
	HI_U32	enabler;					// 有enable的bit，才使用这里的设置，否则用默认设置
	short		x[NUM_OSDID];							// this is X-coord of upper-left corner of text
	short 	y[NUM_OSDID];							// this is Y-coord of upper-left corner of text
	short		nFontId[NUM_OSDID];				// 0 宋体, 1 楷体
	short	  nFontSize[NUM_OSDID];			// 32, 40, 48, 56, 64
	HI_U32	RGBForgrund[NUM_OSDID];		// for text type, this is text color
	HI_U32  RGBBkground[NUM_OSDID];			// for BMP overlay, this is color which to apply background alpha
	HI_U16  alpha[NUM_OSDID][2];						// forground/background alpha			
} OSDPayload, *POSDPayload;

#pragma pack (pop)

typedef union tagDataAttr {
	HI_S32	val[8];
	HI_S16	s16Val[16];
	HI_U16	u16Val[16];
	HI_S8		s8Val[32];
	HI_U8		u8Val[32];
} DataAttr;

typedef struct tagTextAttr {
	char		text[32];
} TextAttr;

typedef struct tagVerAttr {
	char 	strVersion[16];	// 主程序版本号
	int		algver;					// 算法版本号
	int 	param[3];				// 0: 主程序版本号二进制值，1:摄像机型号+Sensor型号，2:条件编译使能的capability
} VerAttr;

typedef struct tagEvtAttr {
	int 	param;
	char  text[28];
} EvtAttr;

typedef struct tagDataHeader
{
	HI_S32	DataType;		// data type (what kind of data)
	HI_S32	DataId;			// which data of this kind. So far, only DataType==DTYP_DATA may use this Id
	union {
	TimeAttr	timeAttr;					// DataType is DTYP_TIME
	ImageAttr	imgAttr;					// DataType is DTYP_IMAGE
	CtrlAttr	ctrlAttr;						// DataType is DTYP_CTRL
	PlateInfo	plateInfo;				// DataType is DTYP_DATA, DataId is DID_PLATE
	TimeInfo	timeInfo;				// DataType is DTYP_DATA, DataId is DID_TIMING
	ParamConf paramConf;			// DataType is DTYP_CONF
	ExtParamConf extParamConf;	// DaraType is DTYP_EXCONF
	DataAttr	dataAttr;					// DataType is DTYP_DATA, available DataId is defined in above
	TextAttr	textAttr;						// DataType is DTYP_TEXT
	VerAttr		verAttr;					// DataType is DTYP_DATA, DataId is DID_VERSION
	H264Conf	h264Conf;				// DataType is DTYP_H264CONF
	OSDConf 	osdConf;				// DataType is DTYP_OSDCONF
	OSDText	osdText;					// DataType is DTYP_OSDTEXT
	EvtAttr		evtAttr;					// DataType is DTYP_EVENT, DataId is event Id
	};
	HI_S32	size;				// data size followed (payload of header)
} DataHeader;

#define CTRL_HEADER_INIT( hdr, c, p )	\
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_CTRL; \
		hdr.ctrlAttr.code = c; \
		hdr.ctrlAttr.param = p; \
	} while (0)

#define CTRL_HEADER_INITEX( hdr, c, p, opt, sz )	\
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_CTRL; \
		hdr.ctrlAttr.code = c; \
		hdr.ctrlAttr.param = p; \
		memcpy( hdr.ctrlAttr.option, opt, 24 ); \
		hdr.size = sz; \
	} while (0)

#define IMAGE_HEADER_INIT( hdr, id, w, h, fmt, s )	\
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_IMAGE; \
		hdr.DataId = id; \
		hdr.imgAttr.width = w; \
		hdr.imgAttr.height = h; \
		hdr.imgAttr.format = fmt; \
		hdr.size = s; \
	} while(0)

#define IMAGE_HEADER_INITEX( hdr, id, w, h, fmt, idx, nm, s )	\
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_IMAGE; \
		hdr.DataId = id; \
		hdr.imgAttr.width = w; \
		hdr.imgAttr.height = h; \
		hdr.imgAttr.format = fmt; \
		hdr.imgAttr.index = idx; \
		if ( nm != NULL ) \
			strncpy(hdr.imgAttr.basename, nm, 24); \
		hdr.imgAttr.basename[23] = '\0'; \
		hdr.size = s; \
	} while(0)
		
#define DATA_HEADER_INIT(hdr, id, pattr) \
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_DATA; \
		hdr.DataId = id; \
		if ( id == DID_PLATE ) \
			memcpy( &hdr.plateInfo, pattr, sizeof(PlateInfo) ); \
		else if ( id == DID_TIMING ) \
			memcpy( &hdr.timeInfo, pattr, sizeof(TimeInfo) ); \
		else if ( id == DID_END ) \
			memcpy( hdr.dataAttr.val, pattr, sizeof(DataAttr) ); \
		else if (id==DID_ISP || id==DID_SNKEY || id==DID_NETCONF) \
			memcpy( hdr.dataAttr.u8Val, pattr, sizeof(DataAttr) ); \
	} while(0)
			
#define DATA_HEADER_INITEX(hdr, id, sz) \
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_DATA; \
		hdr.DataId = id; \
		hdr.size = sz; \
	} while(0)
			
#define VERSION_HEADER_INIT( hdr, algv, s, p1, p2, p3 ) \
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_DATA; \
		hdr.DataId = DID_VERSION; \
		strncpy( hdr.verAttr.strVersion, s, 16 ); \
		hdr.verAttr.strVersion[15] = '\0'; \
		hdr.verAttr.algver = algv; \
		hdr.verAttr.param[0] = p1; \
		hdr.verAttr.param[1] = p2; \
		hdr.verAttr.param[2] = p3; \
	} while (0)
			
#define TEXT_HEADER_INIT( hdr, id, s ) \
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_TEXT; \
		hdr.DataId = id; \
		strncpy( hdr.textAttr.text, s, 32 ); \
		hdr.textAttr.text[31] = '\0'; \
	} while (0)

#define TEXT_HEADER_INITEX( hdr, cap, sz ) \
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_TEXT; \
		hdr.DataId = TID_LONGTEXT; \
		if ( cap != NULL ) \
		{ \
			strncpy( hdr.textAttr.text, cap, 32 ); \
			hdr.textAttr.text[31] = '\0'; \
		} \
		hdr.size = sz; \
	} while (0)

#define UPDATE_HEADER_INIT( hdr, s, n ) \
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_UPDSW; \
		strncpy( hdr.textAttr.text, s, 31 ); \
		hdr.textAttr.text[31] = '\0'; \
		hdr.size = n; \
	} while (0)

#define TIME_HEADER_INIT( hdr, y, m, d, hh, mm, ss, ms ) \
	do { \
		memset( &hdr, 0, sizeof(hdr) ); \
		hdr.DataType = DTYP_TIME; \
		hdr.timeAttr.year  = y; \
		hdr.timeAttr.month = m; \
		hdr.timeAttr.day = d; \
		hdr.timeAttr.hour = hh; \
		hdr.timeAttr.minute = mm; \
		hdr.timeAttr.second = ss; \
		hdr.timeAttr.msec = ms; \
	} while (0)
			
#define QUERY_HEADER_INIT( hdr, id ) \
do { \
	memset( &(hdr), 0, sizeof(hdr) ); \
	(hdr).DataType = DTYP_QUERY; \
	(hdr).DataId = id; \
} while (0)
			
#define REPLY_HEADER_INIT( hdr, id ) \
do { \
	memset( &(hdr), 0, sizeof(hdr) ); \
	(hdr).DataType = DTYP_REPLY; \
	(hdr).DataId = id; \
} while (0)

#define ACK_HEADER_INIT( hdr ) \
do { \
	memset( &hdr, 0, sizeof(hdr) ); \
	hdr.DataType = DTYP_ACK; \
} while (0)

#define HBEAT_HEADER_INIT( hdr ) \
do { \
	memset( &hdr, 0, sizeof(hdr) ); \
	hdr.DataType = DTYP_HBEAT; \
} while (0)


#define SEARCH_HEADER_INIT(hdr) \
do { \
	memset( &hdr, 0, sizeof(hdr) ); \
	hdr.DataType = DTYP_UDP; \
	hdr.DataId = DID_SEARCH; \
} while (0)

#define EVENT_HEADER_INIT(hdr, id, p, t) \
do { \
	memset( &hdr, 0, sizeof(hdr) ); \
	hdr.DataType = DTYP_EVENT; \
	hdr.DataId = id; \
	hdr.evtAttr.param = p; \
	strncpy(hdr.evtAttr.text,t,24); \
	hdr.evtAttr.text[23] = '\0'; \
} while (0)
/*
#define H264_HEADER_INIT(hdr, profile, mode, p1, p2, p3) \
do { \
	memset( &hdr, 0, sizeof(hdr) ); \
	hdr.DataType = DTYP_H264CONF; \
	hdr.h264Conf.enProfile = profile; \
	hdr.h264Conf.enRcMode = mode; \
	hdr.h264Conf.u32BitRate = p1; \
	hdr.h264Conf.u16QP[0] = p2; \
	hdr.h264Conf.u16QP[1] = p3; \
} while(0)
*/
#define H264_HEADER_INIT(hdr, conf) \
do { \
	memset( &hdr, 0, sizeof(hdr) ); \
	hdr.DataType = DTYP_H264CONF; \
	memcpy(&hdr.h264Conf, &conf, sizeof(H264Conf)); \
} while(0)

#define OSD_HEADER_INIT( hdr, conf ) \
do { \
	memset( &hdr, 0, sizeof(hdr) ); \
	hdr.DataType = DTYP_OSDCONF; \
	memcpy( &hdr.osdConf, conf, sizeof(OSDConf)); \
} while (0)

#define OSDTEXT_HEADER_INIT( hdr, conf ) \
do { \
	memset( &hdr, 0, sizeof(hdr) ); \
	hdr.DataType = DTYP_OSDTEXT; \
	if ( conf != NULL ) \
		memcpy( &hdr.osdText, conf, sizeof(OSDText)); \
} while (0)

#endif
