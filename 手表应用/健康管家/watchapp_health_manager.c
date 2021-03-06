/*
 * =====================================================================================
 * 
 *     Modulename:
 *       Filename:  watchapp_health_manager.c
 *
 *    Description:  健康管家
 *    Corporation:
 * 
 *         Author:  gliu (), gliu@damaijiankang.com
 *        Created:  2015年04月04日 14时33分42秒
 *
 * =====================================================================================
 *
 * =====================================================================================
 * 
 *   MODIFICATION HISTORY :
 *    
 *		     DATE :
 *		     DESC :
 * =====================================================================================
 */	
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "maibu_sdk.h"
#include "maibu_res.h"
#define  LOG_INFO(...)

/* 健康管家界面位置 */
#define INIT_SCENE_MOVE_BAR_ORIGIN_X              13
#define INIT_SCENE_MOVE_BAR_ORIGIN_Y              58
#define INIT_SCENE_EXERCISE_BAR_ORIGIN_X          13
#define INIT_SCENE_EXERCISE_BAR_ORIGIN_Y          79
#define INIT_SCENE_STAND_BAR_ORIGIN_X             13
#define INIT_SCENE_STAND_BAR_ORIGIN_Y             100

/* 子界面位置 */
#define SUB_SCENE_PROGRESS_BAR_ORIGIN_X           13
#define SUB_SCENE_PROGRESS_BAR_ORIGIN_Y           100
#define SUB_SCENE_LEFT_TEXT_ORIGIN_X              13
#define SUB_SCENE_LEFT_TEXT_ORIGIN_Y              87
#define SUB_SCENE_LEFT_TEXT_SIZE_H                12
#define SUB_SCENE_LEFT_TEXT_SIZE_W                40
#define SUB_SCENE_RIGHT_TEXT_ORIGIN_X             75
#define SUB_SCENE_RIGHT_TEXT_ORIGIN_Y             87
#define SUB_SCENE_RIGHT_TEXT_SIZE_H               12
#define SUB_SCENE_RIGHT_TEXT_SIZE_W               40

/* 进度条尺寸 */
#define PROGRESS_BAR_SIZE_H                       2
#define PROGRESS_BAR_SIZE_W                       100

/*背景图片图层位置*/
#define HEALTH_BG_ORIGIN_X		                  0
#define HEALTH_BG_ORIGIN_Y		                  0
#define HEALTH_BG_SIZE_H		                  128
#define HEALTH_BG_SIZE_W		                  128

/*页面指示小图片位置*/
#define HEALTH_PAGE_ORIGIN_X		              46
#define HEALTH_PAGE_ORIGIN_Y		              118
#define HEALTH_PAGE_SIZE_H		                  6	
#define HEALTH_PAGE_SIZE_W		                  36	


/*存储KEY*/
#define HEALTH_KEY	                              1	

#define PI                                        3.1415926



/*默认目标卡路里*/
#define HEALTH_CALORIE_GOAL		320

/*默认目标运动分钟*/
#define HEALTH_SPORT_GOAL	30

/*默认目标站立次数*/
#define HEALTH_STAND_GOAL		12

/*默认目标运动每分钟产生多少步有效*/
#define HEALTH_SPORT_MINUTES_STEPS	50

/*默认目标每分钟站立产生多少步有效*/
#define HEALTH_STAND_MINUTES_STEPS	25

/*一个小时内多久没有站立提醒*/
#define HEALTH_STAND_WARNING_MINUTES	50



/*初始化前界面状态*/
// #define HEALTH_INIT_STATE_BEFORE_0	0
// #define HEALTH_INIT_STATE_BEFORE_1	1
// #define HEALTH_INIT_STATE_BEFORE_2	2
// #define HEALTH_INIT_STATE_BEFORE_3	3
// #define HEALTH_INIT_STATE_BEFORE_4	4

/*初始化后界面状态*/
#define HEALTH_INIT_STATE_AFTER_0	0
#define HEALTH_INIT_STATE_AFTER_1	1
#define HEALTH_INIT_STATE_AFTER_2	2
#define HEALTH_INIT_STATE_AFTER_3	3


/*站立起身起始时间点*/
#define HEALTH_STAND_TIME_START		8
#define HEALTH_STAND_TIME_END		20



	


/*健康管家数据结构*/
typedef struct tag_HealthData
{
	/*是否已经初始化*/
	int8_t init_flag;

	/*状态界面序列*/
	int8_t state_index;
	
	/*前一个小时*/
	int8_t pre_hour;

	/*一个小时没运动提醒标志*/	
//	int8_t hint_flag:4;

	/*当天完成运动标志*/
	int8_t finish_flag:4;
	
	/*当天运动分钟*/	
	int16_t sport_time;
	
	/*当前站立次数*/
	int16_t stand_time;

	/*前一个小时为止活动度大小*/
//	uint16_t pre_activity;

	/*当前活动度*/
	int16_t activity;

	/*当天消耗calorie*/
	int16_t calorie;
	
	/*当天步数*/
	int32_t step;
}HealthData, *P_HealthData;


/*存储当天健康数据结构*/
static HealthData g_health_data;


/*向导图层ID*/
static int8_t g_intro_layer_id = -1;

/*窗口ID*/
static int32_t g_health_window_id = -1;

/*全局定时器*/
static int8_t g_health_timer_id = -1;


static void health_select_down(void *context);
static void health_select_up(void *context);
static void health_select_back(void *context);

char* tmp_itoa(int value, char* string, int radix)  
{  
    char tmp[33];  
    char* tp = tmp;  
    int i;  
    unsigned v;  
    int sign;  
    char* sp;  
    if (radix > 36 || radix <= 1)  
    {  
        return 0;  
    }  
    sign = (radix == 10 && value < 0);  
    if (sign)  
        v = -value;  
    else  
        v = (unsigned)value;  
    while (v || tp == tmp)  
    {  
        i = v % radix;  
        v = v / radix;  
        if (i < 10)  
            *tp++ = i+'0';  
        else  
            *tp++ = i + 'a' - 10;  
    }  
    if (string == 0)  
        string = (char*)malloc((tp-tmp)+sign+1);  
    sp = string;  
    if (sign)  
        *sp++ = '-';  
    while (tp > tmp)  
        *sp++ = *--tp;  
    *sp = 0;  
    return string;  
}  

static void create_sub_scene(P_Window p_window, int left_value, int right_value)
{
	Geometry *p_geometry_array[1];
	
	//绘制站立进度条
	GPoint prog_bar_points[4] = { {SUB_SCENE_PROGRESS_BAR_ORIGIN_X, SUB_SCENE_PROGRESS_BAR_ORIGIN_Y}, 
								  {SUB_SCENE_PROGRESS_BAR_ORIGIN_X, SUB_SCENE_PROGRESS_BAR_ORIGIN_Y}, 
								  {SUB_SCENE_PROGRESS_BAR_ORIGIN_X, SUB_SCENE_PROGRESS_BAR_ORIGIN_Y+PROGRESS_BAR_SIZE_H}, 
								  {SUB_SCENE_PROGRESS_BAR_ORIGIN_X, SUB_SCENE_PROGRESS_BAR_ORIGIN_Y+PROGRESS_BAR_SIZE_H} };

	int prog_bar_pix = left_value * PROGRESS_BAR_SIZE_W / right_value;
	prog_bar_pix = prog_bar_pix > 100 ? 100 : prog_bar_pix;
	prog_bar_points[1].x += prog_bar_pix;
	prog_bar_points[2].x += prog_bar_pix;
	
	Polygon prog_bar   = {4, prog_bar_points};
	Geometry geometry = {GeometryTypePolygon, FillArea, GColorWhite,(void*)&prog_bar}; 
	
	p_geometry_array[0] = &geometry;
	LayerGeometry prog_bar_struct = {1, p_geometry_array};
	
	P_Layer	 layer = app_layer_create_geometry(&prog_bar_struct);
	app_window_add_layer(p_window, layer);
	
	
	//绘制显示文本
	char val_str[20] = {0};
	tmp_itoa(left_value, val_str, 10);
	GRect left_text_frame = { {SUB_SCENE_LEFT_TEXT_ORIGIN_X, SUB_SCENE_LEFT_TEXT_ORIGIN_Y}, 
							  {SUB_SCENE_LEFT_TEXT_SIZE_H, SUB_SCENE_LEFT_TEXT_SIZE_W} };
							  
	LayerText left_text_struct = {val_str, left_text_frame, GAlignLeft, U_GBK_SIMSUNBD_12};
	P_Layer   left_text_layer  = app_layer_create_text(&left_text_struct);
	app_layer_set_bg_color(left_text_layer, GColorBlack);
	app_window_add_layer(p_window, left_text_layer);
	
	memset(val_str, 0, 20);
	tmp_itoa(right_value, val_str, 10);
	GRect right_text_frame = { {SUB_SCENE_RIGHT_TEXT_ORIGIN_X, SUB_SCENE_RIGHT_TEXT_ORIGIN_Y}, 
							   {SUB_SCENE_RIGHT_TEXT_SIZE_H, SUB_SCENE_RIGHT_TEXT_SIZE_W} };
							  
	LayerText right_text_struct = {val_str, right_text_frame, GAlignRight, U_GBK_SIMSUNBD_12};
	P_Layer   right_text_layer  = app_layer_create_text(&right_text_struct);
	app_layer_set_bg_color(right_text_layer, GColorBlack);
	app_window_add_layer(p_window, right_text_layer);
}

static void create_init_scene_prog_bar(P_Window p_window)
{
	if(g_health_data.state_index == HEALTH_INIT_STATE_AFTER_0)
	{
		Geometry *p_geometry_array[3];
		Geometry  move_bar_geometry;
		Geometry  exercise_bar_geometry;
		Geometry  stand_bar_geometry;
		
		//绘制活动进度条
		GPoint move_bar_points[4] = { {INIT_SCENE_MOVE_BAR_ORIGIN_X, INIT_SCENE_MOVE_BAR_ORIGIN_Y}, 
									  {INIT_SCENE_MOVE_BAR_ORIGIN_X, INIT_SCENE_MOVE_BAR_ORIGIN_Y}, 
									  {INIT_SCENE_MOVE_BAR_ORIGIN_X, INIT_SCENE_MOVE_BAR_ORIGIN_Y+PROGRESS_BAR_SIZE_H}, 
									  {INIT_SCENE_MOVE_BAR_ORIGIN_X, INIT_SCENE_MOVE_BAR_ORIGIN_Y+PROGRESS_BAR_SIZE_H} };

		uint32_t prog_bar_pix = g_health_data.calorie * PROGRESS_BAR_SIZE_W / HEALTH_CALORIE_GOAL;
		prog_bar_pix = prog_bar_pix > 100 ? 100 : prog_bar_pix;
		move_bar_points[1].x += prog_bar_pix;
		move_bar_points[2].x += prog_bar_pix;
		
		Polygon move_bar = {4, move_bar_points};
		move_bar_geometry.type      = GeometryTypePolygon;
		move_bar_geometry.fill_type = FillArea;
		move_bar_geometry.color     = GColorWhite;
		move_bar_geometry.element   = &move_bar;
		
		//绘制锻炼进度条
		GPoint exercise_bar_points[4] = { {INIT_SCENE_EXERCISE_BAR_ORIGIN_X, INIT_SCENE_EXERCISE_BAR_ORIGIN_Y}, 
									      {INIT_SCENE_EXERCISE_BAR_ORIGIN_X, INIT_SCENE_EXERCISE_BAR_ORIGIN_Y}, 
									      {INIT_SCENE_EXERCISE_BAR_ORIGIN_X, INIT_SCENE_EXERCISE_BAR_ORIGIN_Y+PROGRESS_BAR_SIZE_H}, 
									      {INIT_SCENE_EXERCISE_BAR_ORIGIN_X, INIT_SCENE_EXERCISE_BAR_ORIGIN_Y+PROGRESS_BAR_SIZE_H} };

		prog_bar_pix = g_health_data.sport_time * PROGRESS_BAR_SIZE_W / HEALTH_SPORT_GOAL;
		prog_bar_pix = prog_bar_pix > 100 ? 100 : prog_bar_pix;
		exercise_bar_points[1].x += prog_bar_pix;
		exercise_bar_points[2].x += prog_bar_pix;
		
		Polygon exercise_bar = {4, exercise_bar_points};
		exercise_bar_geometry.type      = GeometryTypePolygon;
		exercise_bar_geometry.fill_type = FillArea;
		exercise_bar_geometry.color     = GColorWhite;
		exercise_bar_geometry.element   = &exercise_bar;
		
		//绘制站立进度条
		GPoint stand_bar_points[4] = { {INIT_SCENE_STAND_BAR_ORIGIN_X, INIT_SCENE_STAND_BAR_ORIGIN_Y}, 
									   {INIT_SCENE_STAND_BAR_ORIGIN_X, INIT_SCENE_STAND_BAR_ORIGIN_Y}, 
									   {INIT_SCENE_STAND_BAR_ORIGIN_X, INIT_SCENE_STAND_BAR_ORIGIN_Y+PROGRESS_BAR_SIZE_H}, 
									   {INIT_SCENE_STAND_BAR_ORIGIN_X, INIT_SCENE_STAND_BAR_ORIGIN_Y+PROGRESS_BAR_SIZE_H} };

		prog_bar_pix = g_health_data.stand_time * PROGRESS_BAR_SIZE_W / HEALTH_STAND_GOAL;
		prog_bar_pix = prog_bar_pix > 100 ? 100 : prog_bar_pix;
		stand_bar_points[1].x += prog_bar_pix;
		stand_bar_points[2].x += prog_bar_pix;
		
		Polygon stand_bar = {4, stand_bar_points};
		stand_bar_geometry.type      = GeometryTypePolygon;
		stand_bar_geometry.fill_type = FillArea;
		stand_bar_geometry.color     = GColorWhite;
		stand_bar_geometry.element   = &stand_bar;
		
		p_geometry_array[0] = &move_bar_geometry;
		p_geometry_array[1] = &exercise_bar_geometry;
		p_geometry_array[2] = &stand_bar_geometry;
		
		LayerGeometry prog_bar_struct = {3, p_geometry_array};
		
		/*创建几何图层*/
		P_Layer	 layer = app_layer_create_geometry(&prog_bar_struct);
		app_window_add_layer(p_window, layer);
	}
}

static void update_display(void)
{
	P_Window p_window = app_window_stack_get_window_by_id(g_health_window_id);	
	if (NULL == p_window)
	{
		return;
	}
		
	P_Window p_new_window = app_window_create();
	if (NULL == p_new_window)
	{
		return;
	}
		
	//配置参数
    uint16_t array[] = {RES_BITMAP_HEALTH_INTRO_1, RES_BITMAP_HEALTH_INTRO_2, RES_BITMAP_HEALTH_INTRO_3, RES_BITMAP_HEALTH_INTRO_4};
    uint16_t right_text_value[] = {0, HEALTH_CALORIE_GOAL, HEALTH_SPORT_GOAL, HEALTH_STAND_GOAL};
	
	//添加背景图层
	GBitmap bmp_bg;
	GRect frame_bg = {{HEALTH_BG_ORIGIN_X, HEALTH_BG_ORIGIN_Y}, {HEALTH_BG_SIZE_H, HEALTH_BG_SIZE_W}};						
	
	res_get_user_bitmap(array[g_health_data.state_index], &bmp_bg);
	LayerBitmap layer_bmp_bg_struct = {bmp_bg, frame_bg, GAlignCenter};
	P_Layer layer_bmp_bg = app_layer_create_bitmap(&layer_bmp_bg_struct);
	app_layer_set_bg_color(layer_bmp_bg, GColorWhite);
	app_window_add_layer(p_new_window, layer_bmp_bg);

	if(g_health_data.state_index == HEALTH_INIT_STATE_AFTER_0)
	{
		create_init_scene_prog_bar(p_new_window);
	}
	else if(g_health_data.state_index == HEALTH_INIT_STATE_AFTER_1)
	{
		create_sub_scene(p_new_window, g_health_data.calorie, HEALTH_CALORIE_GOAL);
	}
	else if(g_health_data.state_index == HEALTH_INIT_STATE_AFTER_2)
	{
		create_sub_scene(p_new_window, g_health_data.sport_time, HEALTH_SPORT_GOAL);
	}
	else if(g_health_data.state_index == HEALTH_INIT_STATE_AFTER_3)
	{
		create_sub_scene(p_new_window, g_health_data.stand_time, HEALTH_STAND_GOAL);
	}

	/*添加窗口按键事件*/
	app_window_click_subscribe(p_new_window, ButtonIdDown, health_select_down);
	app_window_click_subscribe(p_new_window, ButtonIdUp, health_select_up);
	app_window_click_subscribe(p_new_window, ButtonIdBack, health_select_back);

	g_health_window_id = app_window_stack_replace_window(p_window, p_new_window);	
}

/*定义向上按键事件*/
static void health_select_up(void *context)
{
	P_Window p_window = (P_Window)context;		
	if (p_window == NULL)
	{
		return;	
	}	

	if (g_health_data.state_index == HEALTH_INIT_STATE_AFTER_0)
	{
		g_health_data.state_index = HEALTH_INIT_STATE_AFTER_3;	
	}
	else
	{
		g_health_data.state_index--;	
	}

	update_display();
	
	return;
}


/*定义向下按键事件*/
static void health_select_down(void *context)
{
	P_Window p_window = (P_Window)context;		
	if (p_window == NULL)
	{
		return;	
	}	

	if (g_health_data.state_index == HEALTH_INIT_STATE_AFTER_3)
	{
		g_health_data.state_index = HEALTH_INIT_STATE_AFTER_0;	
	}
	else
	{
		g_health_data.state_index++;	
	}
	
	update_display();
	
	return ;
}



/*定义后退按键事件*/
static void health_select_back(void *context)
{
	P_Window p_window = (P_Window)context;
	if (NULL != p_window)
	{
		app_window_stack_pop(p_window);

		/*保存记录*/
		app_persist_write_data_extend(HEALTH_KEY, (unsigned char *)&g_health_data, sizeof(HealthData));

		g_intro_layer_id = -1;
		g_health_window_id = -1;

	}
}



/*
 *--------------------------------------------------------------------------------------
 *     function:  health_init_window
 *    parameter: 
 *       return:
 *  description:  初始化窗口
 * 	  other:
 *--------------------------------------------------------------------------------------
 */
static P_Window health_init_window()
{
	HealthData data;
	memset(&data, 0, sizeof(HealthData));

	/*创建一个可读可写的保存构体的文件key*/
	app_persist_create(HEALTH_KEY, sizeof(HealthData));

	/*读取结构信息*/
	app_persist_read_data(HEALTH_KEY, 0, (unsigned char *)&data, sizeof(HealthData));

	/*如果已经初始化*/
	/*查看文件中存储数据与内存中数据哪个更大，则替换*/
	if (data.init_flag == 1)
	{
		if ((g_health_data.step > data.step) || (g_health_data.calorie > data.calorie) || (g_health_data.activity > data.activity))
		{
			/*保存*/
			app_persist_write_data_extend( HEALTH_KEY, (unsigned char *)&g_health_data, sizeof(HealthData));
		}
		else
		{
			memcpy(&g_health_data, &data, sizeof(HealthData));
		}	
	}
	else
	{
		/*如果没有初始化，要预先读取运动数据避免安装应用之前如果有运动数据，之后第一次定时回调时，会额外增加站立，运动数据*/
		/*获取当天数据*/
		SportData data;

        memset(&data, 0, sizeof(SportData));

		maibu_get_sport_data(&data, 0);
		memset(&g_health_data, 0, sizeof(HealthData));
		g_health_data.activity = data.activity;
		g_health_data.calorie = data.calorie;
		g_health_data.step = data.step;

        g_health_data.init_flag = 1;
	}

	P_Window p_window = app_window_create();
	if (NULL == p_window)
	{
		return NULL;
	}


	/*显示第一个状态界面*/
	GRect frame_guide = {{HEALTH_BG_ORIGIN_X, HEALTH_BG_ORIGIN_Y}, {HEALTH_BG_SIZE_H, HEALTH_BG_SIZE_W}};						
	GBitmap bmp_guide;
	res_get_user_bitmap(RES_BITMAP_HEALTH_INTRO_1, &bmp_guide);
	LayerBitmap layer_bmp_guide_struct = {bmp_guide, frame_guide, GAlignCenter};
	P_Layer layer_bmp_guide = app_layer_create_bitmap(&layer_bmp_guide_struct);
	if(layer_bmp_guide != NULL)
	{
		g_intro_layer_id = app_window_add_layer(p_window, layer_bmp_guide);
	}	

	g_health_data.state_index = HEALTH_INIT_STATE_AFTER_0;
	
	create_init_scene_prog_bar(p_window);
	
	/*添加窗口按键事件*/
	app_window_click_subscribe(p_window, ButtonIdDown, health_select_down);
	app_window_click_subscribe(p_window, ButtonIdUp, health_select_up);
	app_window_click_subscribe(p_window, ButtonIdBack, health_select_back);

	return p_window;
}


static void health_timer_callback(date_time_t tick_time, uint32_t millis, void *context)
{

	if (g_health_data.init_flag != 1)
	{
		return;
	}

	SportData data;

    memset(&data, 0, sizeof(data));

	int8_t flg = 0;

	/*如果小时为0，则为新的一天，清零, 最后一分钟数据不处理，丢弃*/
	if ((tick_time->hour == 0) && ( (tick_time->min == 0) || (tick_time->min == 1) )) //防止在0点卡顿超过10s
	{
		memset(&g_health_data, 0, sizeof(HealthData));
		g_health_data.init_flag = 1;
	
		/*保存记录*/
		app_persist_write_data_extend( HEALTH_KEY, (unsigned char *)&g_health_data, sizeof(HealthData));
	}
	
	/*获取当天数据*/
	maibu_get_sport_data(&data, 0);

#if 0	
	/*如果当前时间分钟小于50，则清除警告标志*/
	if (tick_time->min < HEALTH_STAND_WARNING_MINUTES)
	{
		g_health_data.hint_flag = 0;
	}

	
	/*记录该小时活动度*/
	if(tick_time->min == 0)
	{
		g_health_data.pre_activity = data.activity; 
	}
#endif

	/*卡路里增加*/
	if (data.calorie > g_health_data.calorie)
	{
		flg = 1;
		g_health_data.calorie = data.calorie;
	}		

	/*活动度增加*/
	if (data.activity > g_health_data.activity)
	{
		g_health_data.activity = data.activity;
	}


	/*运动分钟增加*/
	uint8_t step = data.step - g_health_data.step;
	if (step > HEALTH_SPORT_MINUTES_STEPS)
	{
		flg = 1;
		g_health_data.sport_time++;
	}

	/*步数增加*/	
	if (step > 0)
	{
		g_health_data.step = data.step;	
	}
	
	/*站立次数增加*/
	if ((step > HEALTH_STAND_MINUTES_STEPS) && (tick_time->hour != g_health_data.pre_hour)) 
//		&& ((tick_time->hour >= HEALTH_STAND_TIME_START) && (tick_time->hour < HEALTH_STAND_TIME_END)))
	{
		flg = 1;
		if (g_health_data.stand_time == HEALTH_STAND_GOAL)
		{
			g_health_data.stand_time = HEALTH_STAND_GOAL;		
		}
		else
		{
			g_health_data.stand_time++;
		}
		g_health_data.pre_hour = tick_time->hour;
	}		
#if 0
	/*如果分钟大于HEALTH_STAND_WARNING_MINUTES并且该小时没有站立且有活动度，则提醒*/	
	if ((tick_time->min >= HEALTH_STAND_WARNING_MINUTES) && (1 != g_health_data.hint_flag) && (tick_time->hour != g_health_data.pre_hour) 
		&& ((data.activity - g_health_data.pre_activity) != 0)
		&& ((tick_time->hour >= HEALTH_STAND_TIME_START) && (tick_time->hour < HEALTH_STAND_TIME_END)))
	{
		NotifyParam	param;
		memset(&param, 0, sizeof(NotifyParam));
		res_get_user_bitmap(RES_BITMAP_HEALTH_STAND_ALARM,  &param.bmp);	
		strcpy(param.main_title, "久坐伤身");
		strcpy(param.sub_title, "起身运动一分钟吧");
		param.pulse_type = VibesPulseTypeDouble;
        param.pulse_time = 2;
		maibu_service_sys_notify(&param);
	
		/*如果已经提醒，则该小时不再提醒*/
		g_health_data.hint_flag = 1;
	}
#endif

	/*如果在第一个状态界面，并且已经完成运动量，则不刷新该界面了, 省电*/	
	if ((g_health_data.state_index == HEALTH_INIT_STATE_AFTER_0) && (g_health_data.calorie > HEALTH_CALORIE_GOAL) 
		&& (g_health_data.sport_time > HEALTH_SPORT_GOAL) && (g_health_data.stand_time >  HEALTH_STAND_GOAL))
	{
		return;
	}


	/*更新图层*/
	if (flg)
	{

		/*保存记录*/
		app_persist_write_data_extend( HEALTH_KEY, (unsigned char *)&g_health_data, sizeof(HealthData));

		/* 更新显示 */
		update_display();
	}

	/*如果完成任务，系统弹出提示*/
	if ((g_health_data.calorie >= HEALTH_CALORIE_GOAL) && (g_health_data.sport_time >= HEALTH_SPORT_GOAL) && ( g_health_data.stand_time >= HEALTH_STAND_GOAL)
		&& (g_health_data.finish_flag == 0))
	{
		NotifyParam	param;
		memset(&param, 0, sizeof(NotifyParam));
		res_get_user_bitmap(RES_BITMAP_HEALTH_CUP,  &param.bmp);	
		strcpy(param.main_title, "圆满达标");
		strcpy(param.sub_title, "已完成今天三个目标");	
		param.pulse_type = VibesPulseTypeDouble;	
		param.pulse_time = 2;
		maibu_service_sys_notify(&param);

		/*修改当天完成运动标志*/
		g_health_data.finish_flag = 1;
	}

}

#if 0


 //应用结束后, 关机调用
 static void app_exit()
 {

     if (g_health_data.init_flag == 1)
     {
         /*保存*/
         app_persist_write_data_extend( HEALTH_KEY, (unsigned char *)&g_health_data, sizeof(HealthData));
     }

 }

#endif
int main(void)
{

    LOG_INFO("Start health manager");
	simulator_init();

	/*APP编写*/
	/*创建日期时间设置窗口*/
	P_Window p_window = health_init_window();
	if (NULL == p_window) 
	{
		return 0;
	}
	
	/*放入窗口栈显示*/
	g_health_window_id = app_window_stack_push(p_window);
	if(-1 != g_health_window_id)
	{
		/*添加定时器*/
        g_health_timer_id = app_service_timer_subscribe(1000*50, health_timer_callback, (void *)p_window);	//Danger!!!
	}

	simulator_wait();

}


